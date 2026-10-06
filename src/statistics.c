#include "statistics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */
static int cmpDouble(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da < db) return -1;
    if (da > db) return  1;
    return 0;
}

static double calcMode(double *sorted, int n) {
    if (n == 0) return 0.0;

    double mode = sorted[0];
    int maxRun = 1, curRun = 1;

    for (int i = 1; i < n; i++) {
        if (sorted[i] == sorted[i - 1]) {
            curRun++;
            if (curRun > maxRun) {
                maxRun = curRun;
                mode   = sorted[i];
            }
        } else {
            curRun = 1;
        }
    }
    return mode;
}

/* ------------------------------------------------------------------ */
/*  Core computation                                                   */
/* ------------------------------------------------------------------ */
StatResult computeStats(double *arr, int n) {
    StatResult sr;
    memset(&sr, 0, sizeof(StatResult));

    if (!arr || n <= 0) {
        sr.valid = 0;
        return sr;
    }

    /* Make a sorted copy */
    double *tmp = (double *)malloc(n * sizeof(double));
    if (!tmp) { sr.valid = 0; return sr; }
    memcpy(tmp, arr, n * sizeof(double));
    qsort(tmp, n, sizeof(double), cmpDouble);

    /* Min / Max */
    sr.minimum = tmp[0];
    sr.maximum = tmp[n - 1];

    /* Mean */
    double sum = 0.0;
    for (int i = 0; i < n; i++) sum += tmp[i];
    sr.mean = sum / (double)n;

    /* Median */
    if (n % 2 == 0) {
        sr.median = (tmp[n / 2 - 1] + tmp[n / 2]) / 2.0;
    } else {
        sr.median = tmp[n / 2];
    }

    /* Mode */
    sr.mode = calcMode(tmp, n);

    /* Variance & Std Dev (population) */
    double vsum = 0.0;
    for (int i = 0; i < n; i++) {
        double diff = tmp[i] - sr.mean;
        vsum += diff * diff;
    }
    sr.variance = vsum / (double)n;
    sr.stddev   = sqrt(sr.variance);

    /* Quartiles */
    int mid = n / 2;
    /* Q1: median of lower half */
    if (n == 1) {
        sr.q1 = tmp[0];
        sr.q3 = tmp[0];
    } else {
        int lowerN = mid;
        int upperStart = (n % 2 == 0) ? mid : mid + 1;
        int upperN = n - upperStart;

        if (lowerN % 2 == 0)
            sr.q1 = (tmp[lowerN / 2 - 1] + tmp[lowerN / 2]) / 2.0;
        else
            sr.q1 = tmp[lowerN / 2];

        if (upperN % 2 == 0)
            sr.q3 = (tmp[upperStart + upperN / 2 - 1] + tmp[upperStart + upperN / 2]) / 2.0;
        else
            sr.q3 = tmp[upperStart + upperN / 2];
    }
    sr.iqr = sr.q3 - sr.q1;

    /* Outlier count (1.5 * IQR rule) */
    double lowerFence = sr.q1 - 1.5 * sr.iqr;
    double upperFence = sr.q3 + 1.5 * sr.iqr;
    sr.outlierCount = 0;
    for (int i = 0; i < n; i++) {
        if (tmp[i] < lowerFence || tmp[i] > upperFence) {
            sr.outlierCount++;
        }
    }

    free(tmp);
    sr.valid = 1;
    return sr;
}

/* ------------------------------------------------------------------ */
/*  Display                                                            */
/* ------------------------------------------------------------------ */
static void printBar(double val, double maxVal, int width) {
    if (maxVal <= 0) { printf("["); printf("]"); return; }
    int filled = (int)((val / maxVal) * width);
    if (filled > width) filled = width;
    printf("[");
    for (int i = 0; i < width; i++) printf(i < filled ? "#" : "-");
    printf("]");
}

void displayStats(const StatResult *sr, const char *label, const char *unit) {
    if (!sr->valid) {
        printf("\n  [%s] No data available for statistical analysis.\n", label);
        return;
    }

    printf("\n  +---------------------------------------------------------+\n");
    printf("  |  %-10s STATISTICS                                    |\n", label);
    printf("  +---------------------------------------------------------+\n");
    printf("  | Sample Count       : %-10s (data points found)    |\n", "N/A");
    printf("  | Minimum            : %7.2f %-3s                         |\n", sr->minimum, unit);
    printf("  | Maximum            : %7.2f %-3s                         |\n", sr->maximum, unit);
    printf("  | Mean (Average)     : %7.2f %-3s                         |\n", sr->mean, unit);
    printf("  | Median             : %7.2f %-3s                         |\n", sr->median, unit);
    printf("  | Mode               : %7.2f %-3s                         |\n", sr->mode, unit);
    printf("  | Variance           : %7.2f                              |\n", sr->variance);
    printf("  | Std Deviation      : %7.2f %-3s                         |\n", sr->stddev, unit);
    printf("  | Q1 (25th pct)      : %7.2f %-3s                         |\n", sr->q1, unit);
    printf("  | Q3 (75th pct)      : %7.2f %-3s                         |\n", sr->q3, unit);
    printf("  | IQR                : %7.2f %-3s                         |\n", sr->iqr, unit);
    printf("  | Outliers Detected  : %7d (via 1.5*IQR rule)          |\n", sr->outlierCount);
    printf("  +---------------------------------------------------------+\n");

    /* Visual gauge for mean relative to max */
    printf("  | Mean Load : ");
    printBar(sr->mean, 100.0, 30);
    printf(" %5.1f%s |\n", sr->mean, unit);
    printf("  | Max Load  : ");
    printBar(sr->maximum, 100.0, 30);
    printf(" %5.1f%s |\n", sr->maximum, unit);
    printf("  +---------------------------------------------------------+\n");

    /* Threshold Assessment */
    printf("  | Assessment : ");
    if (sr->mean >= 90.0)        printf("  *** CRITICAL - Severe overload      ***");
    else if (sr->mean >= 75.0)   printf("  !!! WARNING  - High resource usage  !!!");
    else if (sr->mean >= 50.0)   printf("  [~] MODERATE - Within normal range     ");
    else                         printf("  [OK] HEALTHY - Low resource usage       ");
    printf(" |\n");
    printf("  +---------------------------------------------------------+\n");
}

void displayStatisticalAnalysis(void) {
    if (!g_isLoaded) {
        printf("\n [!] No log file loaded. Please load a log file first (Option 1).\n");
        return;
    }

    printf("\n========================================================================\n");
    printf("                       STATISTICAL ANALYSIS ENGINE                       \n");
    printf("========================================================================\n");

    StatResult cpuSR, ramSR, diskSR;
    int cpuN  = g_metrics.cpuCount;
    int ramN  = g_metrics.ramCount;
    int diskN = g_metrics.diskCount;

    printf("\n  Data Points Extracted from Log:\n");
    printf("    CPU  readings : %d samples\n", cpuN);
    printf("    RAM  readings : %d samples\n", ramN);
    printf("    DISK readings : %d samples\n", diskN);

    /* CPU */
    cpuSR = computeStats(g_metrics.cpuValues, cpuN);
    /* Override sample count display */
    printf("\n --- CPU Analysis --- (from %d log readings)\n", cpuN);
    displayStats(&cpuSR, "CPU", "%");

    /* RAM */
    ramSR = computeStats(g_metrics.ramValues, ramN);
    printf("\n --- RAM Analysis --- (from %d log readings)\n", ramN);
    displayStats(&ramSR, "RAM", "%");

    /* DISK */
    diskSR = computeStats(g_metrics.diskValues, diskN);
    printf("\n --- DISK Analysis --- (from %d log readings)\n", diskN);
    displayStats(&diskSR, "DISK", "%");

    printf("\n========================================================================\n");

    /* Outlier spotlight */
    int anyOutlier = (cpuSR.valid && cpuSR.outlierCount > 0) ||
                     (ramSR.valid && ramSR.outlierCount > 0)  ||
                     (diskSR.valid && diskSR.outlierCount > 0);

    if (anyOutlier) {
        printf("  [OUTLIER SPOTLIGHT]\n");
        if (cpuSR.valid && cpuSR.outlierCount > 0)
            printf("  CPU  : %d outlier(s) detected outside IQR fence [%.2f - %.2f]%%\n",
                   cpuSR.outlierCount, cpuSR.q1 - 1.5 * cpuSR.iqr, cpuSR.q3 + 1.5 * cpuSR.iqr);
        if (ramSR.valid && ramSR.outlierCount > 0)
            printf("  RAM  : %d outlier(s) detected outside IQR fence [%.2f - %.2f]%%\n",
                   ramSR.outlierCount, ramSR.q1 - 1.5 * ramSR.iqr, ramSR.q3 + 1.5 * ramSR.iqr);
        if (diskSR.valid && diskSR.outlierCount > 0)
            printf("  DISK : %d outlier(s) detected outside IQR fence [%.2f - %.2f]%%\n",
                   diskSR.outlierCount, diskSR.q1 - 1.5 * diskSR.iqr, diskSR.q3 + 1.5 * diskSR.iqr);
        printf("========================================================================\n");
    }
}
