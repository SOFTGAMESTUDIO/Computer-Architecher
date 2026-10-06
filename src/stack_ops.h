#ifndef STACK_OPS_H
#define STACK_OPS_H

#include "file_analyzer.h"

#define STACK_MAX 256
#define STACK_LABEL_LEN 64

typedef struct {
    char  label[STACK_LABEL_LEN];
    int   severity;     /* 1=INFO, 2=SUCCESS, 3=WARNING, 4=ERROR, 5=CRITICAL */
    int   lineNum;
} StackItem;

typedef struct {
    StackItem items[STACK_MAX];
    int       top;        /* index of top element; -1 = empty */
} EventStack;

/* Stack operations */
void  stackInit(EventStack *s);
int   stackIsFull(const EventStack *s);
int   stackIsEmpty(const EventStack *s);
int   stackPush(EventStack *s, const char *label, int severity, int lineNum);
int   stackPop(EventStack *s, StackItem *out);
int   stackPeek(const EventStack *s, StackItem *out);
int   stackSize(const EventStack *s);

/* Menu-driven interactive stack module */
void displayStackModule(void);

#endif /* STACK_OPS_H */
