#include "queue_ops.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/*  Core Queue Operations (circular array)                            */
/* ------------------------------------------------------------------ */
void queueInit(EventQueue *q) {
    q->front = 0;
    q->rear  = -1;
    q->count = 0;
    memset(q->items, 0, sizeof(q->items));
}

int queueIsFull(const EventQueue *q)  { return q->count >= QUEUE_MAX; }
int queueIsEmpty(const EventQueue *q) { return q->count == 0; }
int queueSize(const EventQueue *q)    { return q->count; }

int enqueue(EventQueue *q, const char *label, int severity, int lineNum) {
    if (queueIsFull(q)) return 0;
    q->rear = (q->rear + 1) % QUEUE_MAX;
    strncpy(q->items[q->rear].label, label, QUEUE_LABEL_LEN - 1);
    q->items[q->rear].label[QUEUE_LABEL_LEN - 1] = '\0';
    q->items[q->rear].severity = severity;
    q->items[q->rear].lineNum  = lineNum;
    q->count++;
    return 1;
}

int dequeue(EventQueue *q, QueueItem *out) {
    if (queueIsEmpty(q)) return 0;
    if (out) *out = q->items[q->front];
    q->front = (q->front + 1) % QUEUE_MAX;
    q->count--;
    return 1;
}

int queueFront(const EventQueue *q, QueueItem *out) {
    if (queueIsEmpty(q)) return 0;
    if (out) *out = q->items[q->front];
    return 1;
}

int queueRear(const EventQueue *q, QueueItem *out) {
    if (queueIsEmpty(q)) return 0;
    if (out) *out = q->items[q->rear];
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Display helpers                                                    */
/* ------------------------------------------------------------------ */
static const char *qSevLabel(int sev) {
    switch (sev) {
        case 5: return "CRITICAL";
        case 4: return "ERROR";
        case 3: return "WARNING";
        case 2: return "SUCCESS";
        case 1: return "INFO";
        default: return "UNKNOWN";
    }
}

static void printQueueVisual(const EventQueue *q) {
    if (queueIsEmpty(q)) {
        printf("\n    [ QUEUE IS EMPTY ]\n\n");
        return;
    }

    printf("\n    FRONT\n");
    printf("      |\n");
    int sz = q->count;
    for (int i = 0; i < sz; i++) {
        int idx = (q->front + i) % QUEUE_MAX;
        const QueueItem *it = &q->items[idx];
        int isLast = (i == sz - 1);
        if (isLast)
            printf("      +--> [%-8s] %s  (Line: %d)  <-- REAR\n",
                   qSevLabel(it->severity), it->label, it->lineNum);
        else
            printf("      +--> [%-8s] %s  (Line: %d)\n",
                   qSevLabel(it->severity), it->label, it->lineNum);
    }
    printf("\n    Size: %d / %d\n", sz, QUEUE_MAX);
}

static void printQueueLinear(const EventQueue *q) {
    if (queueIsEmpty(q)) {
        printf("    [EMPTY QUEUE]\n");
        return;
    }
    printf("    FRONT");
    int sz = q->count;
    for (int i = 0; i < sz; i++) {
        int idx = (q->front + i) % QUEUE_MAX;
        printf(" --> %s", q->items[idx].label);
        if (strlen(q->items[idx].label) > 20)
            printf("...");
    }
    printf(" --> REAR\n");
}

/* ------------------------------------------------------------------ */
/*  Interactive Queue Module                                           */
/* ------------------------------------------------------------------ */
void displayQueueModule(void) {
    if (!g_isLoaded) {
        printf("\n [!] No log file loaded. Please load a log file first (Option 1).\n");
        return;
    }

    EventQueue queue;
    queueInit(&queue);

    /* Auto-load all parsed events into queue (FIFO order) */
    int autoLoaded = 0;
    for (int i = 0; i < g_metrics.eventCount && !queueIsFull(&queue); i++) {
        LogEvent *ev = &g_metrics.events[i];
        enqueue(&queue,
                ev->message[0] ? ev->message : ev->level,
                ev->severity, ev->lineNum);
        autoLoaded++;
    }

    printf("\n========================================================================\n");
    printf("                      EVENT QUEUE PROCESSOR (FIFO)                       \n");
    printf("========================================================================\n");
    printf("  Auto-loaded %d events from log (chronological order).\n", autoLoaded);

    int running = 1;
    while (running) {
        printf("\n  +-----------------------------------------+\n");
        printf("  |         QUEUE OPERATIONS MENU           |\n");
        printf("  +-----------------------------------------+\n");
        printf("  | 1. View Queue (Tree format)             |\n");
        printf("  | 2. View Queue (Linear format)           |\n");
        printf("  | 3. Enqueue Custom Event                 |\n");
        printf("  | 4. Dequeue Front Event                  |\n");
        printf("  | 5. Peek Front Event                     |\n");
        printf("  | 6. Peek Rear Event                      |\n");
        printf("  | 7. Process All (Dequeue until empty)    |\n");
        printf("  | 8. Clear & Reload From Log              |\n");
        printf("  | 0. Back to Main Menu                    |\n");
        printf("  +-----------------------------------------+\n");
        printf("  Choice: ");

        int ch;
        if (scanf("%d", &ch) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        switch (ch) {
            case 1:
                printf("\n  --- QUEUE VISUAL (Tree) ---");
                printQueueVisual(&queue);
                break;

            case 2:
                printf("\n  --- QUEUE VISUAL (Linear) ---\n");
                printQueueLinear(&queue);
                printf("\n");
                break;

            case 3: {
                if (queueIsFull(&queue)) {
                    printf("\n  [!] Queue is full (%d items)!\n", QUEUE_MAX);
                    break;
                }
                char buf[QUEUE_LABEL_LEN];
                int sev;
                printf("  Enter event label  : ");
                if (!fgets(buf, sizeof(buf), stdin)) break;
                buf[strcspn(buf, "\r\n")] = '\0';
                printf("  Enter severity (1=INFO 2=SUCCESS 3=WARNING 4=ERROR 5=CRITICAL): ");
                if (scanf("%d", &sev) != 1) { while(getchar()!='\n'); break; }
                while (getchar() != '\n');
                if (sev < 1 || sev > 5) sev = 1;
                if (enqueue(&queue, buf, sev, 0)) {
                    printf("  [+] ENQUEUE \"%s\" [%s] -> Queue size: %d\n",
                           buf, qSevLabel(sev), queueSize(&queue));
                }
                break;
            }

            case 4: {
                QueueItem out;
                if (dequeue(&queue, &out)) {
                    printf("\n  [-] DEQUEUE -> \"%s\" [%s] (Line: %d) processed.\n",
                           out.label, qSevLabel(out.severity), out.lineNum);
                    printf("  Queue size now: %d\n", queueSize(&queue));
                } else {
                    printf("\n  [!] Queue is empty.\n");
                }
                break;
            }

            case 5: {
                QueueItem front;
                if (queueFront(&queue, &front))
                    printf("\n  [FRONT] \"%s\" [%s] (Line: %d)\n",
                           front.label, qSevLabel(front.severity), front.lineNum);
                else
                    printf("\n  [!] Queue is empty.\n");
                break;
            }

            case 6: {
                QueueItem rear;
                if (queueRear(&queue, &rear))
                    printf("\n  [REAR] \"%s\" [%s] (Line: %d)\n",
                           rear.label, qSevLabel(rear.severity), rear.lineNum);
                else
                    printf("\n  [!] Queue is empty.\n");
                break;
            }

            case 7: {
                printf("\n  [PROCESS ALL] Dequeuing all events...\n\n");
                int processed = 0;
                QueueItem out;
                while (dequeue(&queue, &out)) {
                    printf("  [%3d] Processed: [%-8s] %s\n",
                           ++processed, qSevLabel(out.severity), out.label);
                }
                printf("\n  Total processed: %d events. Queue is now empty.\n", processed);
                break;
            }

            case 8:
                queueInit(&queue);
                autoLoaded = 0;
                for (int i = 0; i < g_metrics.eventCount && !queueIsFull(&queue); i++) {
                    LogEvent *ev = &g_metrics.events[i];
                    enqueue(&queue,
                            ev->message[0] ? ev->message : ev->level,
                            ev->severity, ev->lineNum);
                    autoLoaded++;
                }
                printf("\n  [+] Reloaded %d events from log.\n", autoLoaded);
                break;

            case 0:
                running = 0;
                break;

            default:
                printf("\n  [!] Invalid option.\n");
        }
    }
}
