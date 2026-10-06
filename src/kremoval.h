#ifndef KREMOVAL_H
#define KREMOVAL_H

/* Maximum-K Digit Removal (Greedy Algorithm + Stack)
 *
 * Problem: Given a number string and k, remove k digits so the result
 *          is the LARGEST possible number (max-K variant).
 *
 * Strategy: Use a monotonic stack. For each digit, while the stack top
 *           is SMALLER than the current digit and k > 0, pop (remove)
 *           the smaller digit and decrement k.
 */

#define KREMOVAL_MAX_INPUT 256

typedef struct {
    char input[KREMOVAL_MAX_INPUT];
    int  k;
    char result[KREMOVAL_MAX_INPUT];
    int  removedCount;
    char steps[KREMOVAL_MAX_INPUT * 4][128]; /* step-by-step trace */
    int  stepCount;
} KRemovalResult;

/* Perform maximum-K removal */
KRemovalResult maxKRemoval(const char *numStr, int k);

/* Display the module interactively */
void displayKRemovalModule(void);

#endif /* KREMOVAL_H */
