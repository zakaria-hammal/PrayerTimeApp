#ifndef QUEUE
#define QUEUE

typedef struct PrayerInfoStruct PrayerInfo;
struct PrayerInfoStruct
{
    char name[12];
    struct tm prayerTime;
};

typedef struct QueueNode QueueNode;
struct QueueNode
{
    PrayerInfo label;
    QueueNode *next;
};

typedef struct PrayerTimeQueue PrayerTimeQueue;
struct PrayerTimeQueue
{
    QueueNode *firstElement;
    QueueNode *lastElement;
    int numberOfElements;
};

void initQueue(PrayerTimeQueue *queue);
void enqueue(PrayerTimeQueue *queue, PrayerInfo prayerInfo);
PrayerInfo dequeue(PrayerTimeQueue *queue);
int isQueueEmpty(PrayerTimeQueue queue);

#endif
