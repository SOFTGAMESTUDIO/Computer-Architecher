#ifndef STATISTICS_H
#define STATISTICS_H

#include <math.h>
#include "file_analyzer.h"

typedef struct {
    double mean;
    double median;
    double mode;
    double variance;
    double stddev;
    double minimum;
    double maximum;
    double q1;       /* First quartile  */
    double q3;       /* Third quartile  */
    double iqr;      /* Interquartile range */
    int    outlierCount;
    int    valid;    /* 1 if stats computed, 0 if no data */
} StatResult;

/* Compute statistics for an array of doubles */
StatResult computeStats(double *arr, int n);

/* Display stat results with a label (e.g. "CPU", "RAM", "DISK") */
void displayStats(const StatResult *sr, const char *label, const char *unit);

/* Main function called from menu */
void displayStatisticalAnalysis(void);

#endif /* STATISTICS_H */
