#include "stack_ops.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/*  Core Stack Operations                                              */
/* ------------------------------------------------------------------ */
void stackInit(EventStack *s) {
    s->top = -1;
    memset(s->items, 0, sizeof(s->items));
}

int stackIsFull(const EventStack *s)  { return s->top >= STACK_MAX - 1; }
int stackIsEmpty(const EventStack *s) { return s->top < 0; }
int stackSize(const EventStack *s)    { return s->top + 1; }

int stackPush(EventStack *s, const char *label, int severity, int lineNum) {
    if (stackIsFull(s)) return 0;
    s->top++;
    strncpy(s->items[s->top].label, label, STACK_LABEL_LEN - 1);
    s->items[s->top].label[STACK_LABEL_LEN - 1] = '\0';
    s->items[s->top].severity = severity;
    s->items[s->top].lineNum  = lineNum;
    return 1;
}

int stackPop(EventStack *s, StackItem *out) {
    if (stackIsEmpty(s)) return 0;
    if (out) *out = s->items[s->top];
    s->top--;
    return 1;
}

int stackPeek(const EventStack *s, StackItem *out) {
    if (stackIsEmpty(s)) return 0;
    if (out) *out = s->items[s->top];
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Display helpers                                                    */
/* ------------------------------------------------------------------ */
static const char *severityLabel(int sev) {
    switch (sev) {
        case 5: return "CRITICAL";
        case 4: return "ERROR";
        case 3: return "WARNING";
        case 2: return "SUCCESS";
        case 1: return "INFO";
        default: return "UNKNOWN";
    }
}

static const char *severityBar(int sev) {
    switch (sev) {
        case 5: return "!!!!!";
        case 4: return "####";
        case 3: return "~~~";
        case 2: return "---";
        case 1: return "...";
        default: return "   ";
    }
}

static void printStackVisual(const EventStack *s) {
    if (stackIsEmpty(s)) {
        printf("\n    [ STACK IS EMPTY ]\n\n");
        return;
    }

    printf("\n    TOP\n");
    printf("     |\n");
    int sz = stackSize(s);
    for (int i = s->top; i >= 0; i--) {
        const StackItem *it = &s->items[i];
        int isLast = (i == 0);
        if (isLast) {
            printf("     +-- [%-8s] %s  (Line: %d)\n",
                   severityLabel(it->severity), it->label, it->lineNum);
        } else {
            printf("     |-- [%-8s] %s  (Line: %d)\n",
                   severityLabel(it->severity), it->label, it->lineNum);
        }
    }
    printf("\n    Size: %d / %d\n", sz, STACK_MAX);
}

/* ------------------------------------------------------------------ */
/*  Interactive Stack Module                                           */
/* ------------------------------------------------------------------ */
void displayStackModule(void) {
    if (!g_isLoaded) {
        printf("\n [!] No log file loaded. Please load a log file first (Option 1).\n");
        return;
    }

    EventStack stack;
    stackInit(&stack);

    /* Auto-load high-severity events from parsed log */
    int autoLoaded = 0;
    for (int i = 0; i < g_metrics.eventCount && !stackIsFull(&stack); i++) {
        LogEvent *ev = &g_metrics.events[i];
        if (ev->severity >= 3) {  /* WARNING and above */
            stackPush(&stack, ev->message[0] ? ev->message : ev->level,
                      ev->severity, ev->lineNum);
            autoLoaded++;
        }
    }

    printf("\n========================================================================\n");
    printf("                      EVENT STACK PROCESSOR                              \n");
    printf("========================================================================\n");
    printf("  Auto-loaded %d high-severity events (WARNING/ERROR/CRITICAL) from log.\n", autoLoaded);

    int running = 1;
    while (running) {
        printf("\n  +-----------------------------------------+\n");
        printf("  |         STACK OPERATIONS MENU           |\n");
        printf("  +-----------------------------------------+\n");
        printf("  | 1. View Current Stack                   |\n");
        printf("  | 2. Push Custom Event                    |\n");
        printf("  | 3. Pop Top Event                        |\n");
        printf("  | 4. Peek Top Event                       |\n");
        printf("  | 5. Clear Stack                          |\n");
        printf("  | 6. Reload Events From Log               |\n");
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
                printf("\n  --- STACK VISUAL ---\n");
                printStackVisual(&stack);
                break;

            case 2: {
                if (stackIsFull(&stack)) {
                    printf("\n  [!] Stack is full (%d items)!\n", STACK_MAX);
                    break;
                }
                char buf[STACK_LABEL_LEN];
                int sev;
                printf("  Enter event label  : ");
                if (!fgets(buf, sizeof(buf), stdin)) break;
                buf[strcspn(buf, "\r\n")] = '\0';

                printf("  Enter severity (1=INFO 2=SUCCESS 3=WARNING 4=ERROR 5=CRITICAL): ");
                if (scanf("%d", &sev) != 1) { while(getchar()!='\n'); break; }
                while (getchar() != '\n');
                if (sev < 1 || sev > 5) sev = 1;

                if (stackPush(&stack, buf, sev, 0)) {
                    printf("  [+] PUSH  \"%s\" [%s] -> Stack size: %d\n",
                           buf, severityLabel(sev), stackSize(&stack));
                }
                break;
            }

            case 3: {
                StackItem popped;
                if (stackPop(&stack, &popped)) {
                    printf("\n  [-] POP  -> \"%s\" [%s] (Line: %d) removed from stack.\n",
                           popped.label, severityLabel(popped.severity), popped.lineNum);
                    printf("  Stack size now: %d\n", stackSize(&stack));
                } else {
                    printf("\n  [!] Stack is empty — nothing to pop.\n");
                }
                break;
            }

            case 4: {
                StackItem top;
                if (stackPeek(&stack, &top)) {
                    printf("\n  [PEEK] Top item: \"%s\" [%s]  %s  (Line: %d)\n",
                           top.label, severityLabel(top.severity),
                           severityBar(top.severity), top.lineNum);
                } else {
                    printf("\n  [!] Stack is empty.\n");
                }
                break;
            }

            case 5:
                stackInit(&stack);
                autoLoaded = 0;
                printf("\n  [OK] Stack cleared.\n");
                break;

            case 6:
                stackInit(&stack);
                autoLoaded = 0;
                for (int i = 0; i < g_metrics.eventCount && !stackIsFull(&stack); i++) {
                    LogEvent *ev = &g_metrics.events[i];
                    if (ev->severity >= 3) {
                        stackPush(&stack, ev->message[0] ? ev->message : ev->level,
                                  ev->severity, ev->lineNum);
                        autoLoaded++;
                    }
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
