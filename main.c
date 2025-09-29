#include <openssl/crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <libnotify/notify.h>
#include "queue.h"

PrayerTimeQueue getPrayerTimes()
{
    PrayerTimeQueue queue;
    initQueue(&queue);
    
    int api_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(api_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(api_fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
    
    struct sockaddr_in server_address;
    struct addrinfo hints = {.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM};
    struct addrinfo *res;

    getaddrinfo("api.aladhan.com", "443", &hints, &res);
    connect(api_fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    OPENSSL_init_ssl(0, NULL);
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, api_fd);
    if (SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    
    char requestBuffer[200] = {0}; 
    
    snprintf(requestBuffer, sizeof(requestBuffer), 
                                                "GET /v1/timingsByCity/%d-%02d-%02d?city=Algiers&country=Algeria&method=2 HTTP/1.1\r\n"
                                                "Host: api.aladhan.com\r\n"
                                                "User-Agent: PrayerTimeApp\r\n"
                                                "Accept: */*\r\n\r\n", lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);

    SSL_write(ssl, requestBuffer, strlen(requestBuffer));

    char responseBuffer[2500] = {0};
    int total_bytes = 0;
    int bytes_read;

    while ((bytes_read = SSL_read(ssl, responseBuffer + total_bytes, 
                                 sizeof(responseBuffer) - total_bytes - 1)) > 0) {
        total_bytes += bytes_read;
        if (total_bytes >= (int)sizeof(responseBuffer) - 1) {
            break;
        }
    }

    responseBuffer[total_bytes] = '\0';    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(api_fd);
    
    char *body = strstr(responseBuffer, "\r\n\r\n") + 4;

    if (strstr(responseBuffer, "Transfer-Encoding: chunked"))
    {
        char *p = body;
        char *out = body;
        char *line_end;
        
        while (1)
        {
            line_end = strstr(p, "\r\n");
            if (!line_end) break;
    
            unsigned int chunkSize = 0;
            if (sscanf(p, "%x", &chunkSize) != 1) break;
    
            if (chunkSize == 0) break;
    
            p = line_end + 2;
    
            memmove(out, p, chunkSize);
            out += chunkSize;
            p += chunkSize + 2;
        }
    
        *out = '\0';
    }
        
    cJSON *root = cJSON_Parse(body);

    cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");

    cJSON *timings = cJSON_GetObjectItemCaseSensitive(data, "timings");
    
    PrayerInfo info; 
    cJSON *item = cJSON_GetObjectItemCaseSensitive(timings, "Fajr");
    info.prayerTime.tm_hour = atoi(item->valuestring);
    info.prayerTime.tm_min = atoi(item->valuestring + 3);
    strcpy(info.name, "Fajr");
    enqueue(&queue, info); 

    item = cJSON_GetObjectItemCaseSensitive(timings, "Dhuhr");
    info.prayerTime.tm_hour = atoi(item->valuestring);
    info.prayerTime.tm_min = atoi(item->valuestring + 3);
    strcpy(info.name, "Dhuhr");
    enqueue(&queue, info);    

    item = cJSON_GetObjectItemCaseSensitive(timings, "Asr");
    info.prayerTime.tm_hour = atoi(item->valuestring);
    info.prayerTime.tm_min = atoi(item->valuestring + 3);
    strcpy(info.name, "Asr");
    enqueue(&queue, info);
    
    item = cJSON_GetObjectItemCaseSensitive(timings, "Maghrib");
    info.prayerTime.tm_hour = atoi(item->valuestring);
    info.prayerTime.tm_min = atoi(item->valuestring + 3);
    strcpy(info.name, "Maghrib");
    enqueue(&queue, info);

    item = cJSON_GetObjectItemCaseSensitive(timings, "Isha");
    info.prayerTime.tm_hour = atoi(item->valuestring);
    info.prayerTime.tm_min = atoi(item->valuestring + 3);
    strcpy(info.name, "Isha");
    enqueue(&queue, info);

    cJSON_Delete(root);

    return queue;
}

int myCompareTime(struct tm time1, struct tm time2)
{
    if (time1.tm_hour > time2.tm_hour) 
    {
        return 1;
    }

    if (time1.tm_hour < time2.tm_hour) 
    {
        return -1;
    }

    if (time1.tm_min > time2.tm_min) 
    {
        return 1;
    }

    if (time1.tm_min < time2.tm_min) 
    {
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    struct tm midnight;
    midnight.tm_hour = 0;
    midnight.tm_min = 0;
    
    PrayerTimeQueue queue = getPrayerTimes();
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);

    while (!isQueueEmpty(queue)) 
    {
        if (myCompareTime((queue.firstElement->label).prayerTime, *lt) == -1)
        {
            dequeue(&queue);
        }
        else 
        {
            break;
        }
    }

    notify_init("PrayerTimeApp");
    
    NotifyNotification* n;
    
    n = notify_notification_new(
        "Launch",
        "PrayerTimeApp Started",
        "/home/zakaria/Documents/PrayerTimeApp/prayer.png"
    );

    notify_notification_show(n, NULL);
    
    g_object_unref(G_OBJECT(n));
    
    char title[12];
    char body[100];
    
    while (1) 
    {
        while (!isQueueEmpty(queue)) 
        {
            now = time(NULL);
            lt = localtime(&now);
    
            if (myCompareTime(queue.firstElement->label.prayerTime, *lt) == 0) 
            {
                strcpy(title, queue.firstElement->label.name);
                snprintf(body, sizeof(body), "%s prayer time has come", title);
                n = notify_notification_new(
                    title,
                    body,
                    "/home/zakaria/Documents/PrayerTimeApp/prayer.png"
                );
            
                notify_notification_show(n, NULL);
                
                g_object_unref(G_OBJECT(n));
                
                dequeue(&queue);
                
                sleep(3000);
            }
            else if (myCompareTime(queue.firstElement->label.prayerTime, *lt) == -1) 
            {
                while (myCompareTime(queue.firstElement->label.prayerTime, *lt) == -1) 
                {
                    strcpy(title, queue.firstElement->label.name);
                    snprintf(body, sizeof(body), "%s prayer time has come", title);
                    n = notify_notification_new(
                        title,
                        body,
                        "/home/zakaria/Documents/PrayerTimeApp/prayer.png"
                    );
                
                    notify_notification_show(n, NULL);
                    
                    g_object_unref(G_OBJECT(n));
                    
                    dequeue(&queue);
                }
                
                if (!isQueueEmpty(queue) && myCompareTime(queue.firstElement->label.prayerTime, *lt) == 0) 
                {
                    strcpy(title, queue.firstElement->label.name);
                    snprintf(body, sizeof(body), "%s prayer time came at %d:%d", title, queue.firstElement->label.prayerTime.tm_hour, queue.firstElement->label.prayerTime.tm_min);
                    n = notify_notification_new(
                        title,
                        body,
                        "/home/zakaria/Documents/PrayerTimeApp/prayer.png"
                    );
                
                    notify_notification_show(n, NULL);
                    
                    g_object_unref(G_OBJECT(n));
                    
                    dequeue(&queue);
                    
                    sleep(3000);
                }
            }
            
            sleep(30);
        }
        
        sleep(3600);
        
        now = time(NULL);
        lt = localtime(&now);
        
        if (myCompareTime(*lt, midnight) == 1) 
        {
            queue = getPrayerTimes();
        }
    }
    
    return EXIT_SUCCESS;
}
