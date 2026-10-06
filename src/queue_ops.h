#ifndef QUEUE_OPS_H
#define QUEUE_OPS_H

#include "file_analyzer.h"

#define QUEUE_MAX 256
#define QUEUE_LABEL_LEN 64

typedef struct {
    char label[QUEUE_LABEL_LEN];
    int  severity;
    int  lineNum;
} QueueItem;

typedef struct {
    QueueItem items[QUEUE_MAX];
    int front;   /* index of front element */
    int rear;    /* index of last element  */
    int count;
} EventQueue;

void queueInit(EventQueue *q);
int  queueIsFull(const EventQueue *q);
int  queueIsEmpty(const EventQueue *q);
int  queueSize(const EventQueue *q);
int  enqueue(EventQueue *q, const char *label, int severity, int lineNum);
int  dequeue(EventQueue *q, QueueItem *out);
int  queueFront(const EventQueue *q, QueueItem *out);
int  queueRear(const EventQueue *q, QueueItem *out);

void displayQueueModule(void);

#endif /* QUEUE_OPS_H */
