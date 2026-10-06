#include "kremoval.h"
#include "file_analyzer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/*  Core Algorithm                                                     */
/* ------------------------------------------------------------------ */
KRemovalResult maxKRemoval(const char *numStr, int k) {
    KRemovalResult res;
    memset(&res, 0, sizeof(res));

    if (!numStr || strlen(numStr) == 0) {
        strcpy(res.result, "0");
        return res;
    }

    /* Filter: keep only digits */
    char clean[KREMOVAL_MAX_INPUT];
    int cLen = 0;
    for (int i = 0; numStr[i] && cLen < KREMOVAL_MAX_INPUT - 1; i++) {
        if (isdigit((unsigned char)numStr[i])) {
            clean[cLen++] = numStr[i];
        }
    }
    clean[cLen] = '\0';

    strncpy(res.input, clean, KREMOVAL_MAX_INPUT - 1);
    res.k = k;

    if (cLen == 0) {
        strcpy(res.result, "0");
        return res;
    }

    /* Stack-based greedy algorithm */
    char stack[KREMOVAL_MAX_INPUT];
    int  top = -1;
    int  rem = k;
    res.stepCount = 0;
    res.removedCount = 0;

    for (int i = 0; i < cLen; i++) {
        char cur = clean[i];

        /* While top of stack is SMALLER than current, pop (remove smaller) */
        while (top >= 0 && rem > 0 && stack[top] < cur) {
            char removed = stack[top--];
            rem--;
            res.removedCount++;

            if (res.stepCount < KREMOVAL_MAX_INPUT * 4) {
                int sc = res.stepCount++;
                snprintf(res.steps[sc], 128,
                         "  Step %2d: digit '%c' > stack-top '%c' -> REMOVE '%c' from stack  [k remaining: %d]",
                         sc + 1, cur, removed, removed, rem);
            }
        }

        /* Push current digit */
        stack[++top] = cur;
        if (res.stepCount < KREMOVAL_MAX_INPUT * 4) {
            int sc = res.stepCount++;
            snprintf(res.steps[sc], 128,
                     "  Step %2d: PUSH '%c' onto stack                             [k remaining: %d]",
                     sc + 1, cur, rem);
        }
    }

    /* If k > 0, remove from end (top of stack) */
    while (rem > 0 && top >= 0) {
        char removed = stack[top--];
        rem--;
        res.removedCount++;
        if (res.stepCount < KREMOVAL_MAX_INPUT * 4) {
            int sc = res.stepCount++;
            snprintf(res.steps[sc], 128,
                     "  Step %2d: k still %d -> POP end digit '%c' from stack      [k remaining: %d]",
                     sc + 1, rem + 1, removed, rem);
        }
    }

    /* Build result from stack (bottom to top) */
    int rLen = top + 1;
    if (rLen == 0) {
        strcpy(res.result, "0");
    } else {
        for (int i = 0; i <= top; i++) {
            res.result[i] = stack[i];
        }
        res.result[rLen] = '\0';

        /* Remove leading zeros */
        int start = 0;
        while (start < rLen - 1 && res.result[start] == '0') start++;
        if (start > 0) {
            memmove(res.result, res.result + start, rLen - start + 1);
        }
    }

    return res;
}

/* ------------------------------------------------------------------ */
/*  Build a number string from event severity scores                  */
/* ------------------------------------------------------------------ */
static void buildEventScoreString(char *out, int maxLen) {
    out[0] = '\0';
    int pos = 0;
    for (int i = 0; i < g_metrics.eventCount && pos < maxLen - 2; i++) {
        char c = '0' + (g_metrics.events[i].severity % 10);
        out[pos++] = c;
    }
    out[pos] = '\0';
}

/* ------------------------------------------------------------------ */
/*  Display Module                                                     */
/* ------------------------------------------------------------------ */
void displayKRemovalModule(void) {
    printf("\n========================================================================\n");
    printf("              MAXIMUM-K REMOVAL  (Greedy Algorithm + Stack)              \n");
    printf("========================================================================\n");
    printf("\n  CONCEPT:\n");
    printf("  Given a number string and integer K, remove exactly K digits\n");
    printf("  to produce the LARGEST possible resulting number.\n");
    printf("\n  ALGORITHM: Monotonic Stack (Greedy)\n");
    printf("  -> For each digit, while stack-top is SMALLER than current digit\n");
    printf("     and K > 0: pop stack-top (remove the smaller digit).\n");
    printf("  -> Push current digit. If K > 0 after scan, trim from the end.\n");

    int running = 1;
    while (running) {
        printf("\n  +---------------------------------------------+\n");
        printf("  |       MAX-K REMOVAL MENU                    |\n");
        printf("  +---------------------------------------------+\n");
        printf("  | 1. Run with custom number string & K        |\n");
        printf("  | 2. Auto-demo with preset examples           |\n");
        if (g_isLoaded)
            printf("  | 3. Run on log event severity scores         |\n");
        printf("  | 0. Back to Main Menu                        |\n");
        printf("  +---------------------------------------------+\n");
        printf("  Choice: ");

        int ch;
        if (scanf("%d", &ch) != 1) { while(getchar()!='\n'); continue; }
        while (getchar() != '\n');

        switch (ch) {
            case 1: {
                char numBuf[KREMOVAL_MAX_INPUT];
                int k;
                printf("\n  Enter number string (digits only, e.g. 918234) : ");
                if (!fgets(numBuf, sizeof(numBuf), stdin)) break;
                numBuf[strcspn(numBuf, "\r\n")] = '\0';

                printf("  Enter K (digits to remove)                    : ");
                if (scanf("%d", &k) != 1) { while(getchar()!='\n'); break; }
                while (getchar() != '\n');
                if (k < 0) k = 0;

                KRemovalResult r = maxKRemoval(numBuf, k);

                printf("\n  -------------------------------------------------------\n");
                printf("  Input Number  : %s\n", r.input);
                printf("  K (to remove) : %d\n", r.k);
                printf("  -------------------------------------------------------\n");
                printf("  STEP-BY-STEP TRACE:\n\n");
                for (int i = 0; i < r.stepCount; i++) {
                    printf("%s\n", r.steps[i]);
                }
                printf("\n  -------------------------------------------------------\n");
                printf("  Digits removed : %d\n", r.removedCount);
                printf("  RESULT         : %s\n", r.result);
                printf("  -------------------------------------------------------\n");
                break;
            }

            case 2: {
                /* Built-in demos */
                struct { const char *num; int k; const char *desc; } demos[] = {
                    {"9182737",   2, "Remove 2 digits from 9182737"},
                    {"10200",     1, "Remove 1 digit from 10200"},
                    {"112",       1, "Remove 1 digit from 112"},
                    {"987654321", 3, "Remove 3 digits from 987654321"},
                    {"1432219",   3, "Remove 3 digits from 1432219"},
                };
                int nDemos = (int)(sizeof(demos) / sizeof(demos[0]));

                printf("\n  === AUTO DEMO EXAMPLES ===\n\n");
                for (int d = 0; d < nDemos; d++) {
                    KRemovalResult r = maxKRemoval(demos[d].num, demos[d].k);
                    printf("  [Demo %d] %s\n", d + 1, demos[d].desc);
                    printf("  Input  : %s\n", r.input);
                    printf("  K      : %d\n", r.k);
                    printf("  Result : %s   (removed %d digit(s))\n\n", r.result, r.removedCount);
                }

                /* Show first demo step-by-step */
                printf("  --- Detailed trace for Demo 1 ---\n");
                KRemovalResult r0 = maxKRemoval(demos[0].num, demos[0].k);
                for (int i = 0; i < r0.stepCount; i++) printf("%s\n", r0.steps[i]);
                printf("  Result : %s\n\n", r0.result);
                break;
            }

            case 3: {
                if (!g_isLoaded) {
                    printf("\n  [!] No log file loaded.\n");
                    break;
                }
                char scoreStr[KREMOVAL_MAX_INPUT];
                buildEventScoreString(scoreStr, KREMOVAL_MAX_INPUT);
                if (strlen(scoreStr) == 0) {
                    printf("\n  [!] No events found in log.\n");
                    break;
                }

                int k = (int)(strlen(scoreStr) / 4);
                if (k < 1) k = 1;

                printf("\n  Log event severity scores : %s\n", scoreStr);
                printf("  Auto-selected K           : %d\n", k);

                KRemovalResult r = maxKRemoval(scoreStr, k);
                printf("\n  STEP-BY-STEP TRACE:\n\n");
                for (int i = 0; i < r.stepCount; i++) printf("%s\n", r.steps[i]);
                printf("\n  Result (optimized score sequence) : %s\n", r.result);
                printf("  This represents the highest-value event priority ordering.\n\n");
                break;
            }

            case 0:
                running = 0;
                break;

            default:
                printf("\n  [!] Invalid option.\n");
        }
    }
}
