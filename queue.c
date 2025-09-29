#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"

void initQueue(PrayerTimeQueue *queue)
{
    queue->firstElement = NULL;
    queue->lastElement = NULL;
    queue->numberOfElements = 0;
}

void enqueue(PrayerTimeQueue *queue, PrayerInfo prayerInfo)
{
    QueueNode *node;
    node = malloc(sizeof(QueueNode));
    node->label.prayerTime = prayerInfo.prayerTime;
    strcpy(node->label.name, prayerInfo.name);
    node->next = NULL;
    
    if (queue->lastElement == NULL) 
    {
        queue->firstElement = node;
        queue->lastElement = node;
        return;
    }

    queue->lastElement->next = node;
    queue->lastElement = node;
    (queue->numberOfElements)++;
}

PrayerInfo dequeue(PrayerTimeQueue *queue)
{
    PrayerInfo prayerTime = {0};
    
    if (queue->firstElement == NULL) 
    {
        perror("You can't dequeue an empty queue\n");
        return prayerTime;
    }

    QueueNode * node;
    prayerTime.prayerTime = queue->firstElement->label.prayerTime;
    strcpy(prayerTime.name, queue->firstElement->label.name);

    node = queue->firstElement;
    queue->firstElement = queue->firstElement->next;

    free(node);
    
    (queue->numberOfElements)--;
    return prayerTime;
}

int isQueueEmpty(PrayerTimeQueue queue)
{
    return queue.firstElement == NULL;
}
