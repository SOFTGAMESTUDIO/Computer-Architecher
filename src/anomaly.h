#ifndef ANOMALY_H
#define ANOMALY_H

#include "file_analyzer.h"
#include "statistics.h"

/* Anomaly thresholds */
#define THRESH_CPU_CRITICAL    90.0
#define THRESH_CPU_WARNING     75.0
#define THRESH_RAM_CRITICAL    90.0
#define THRESH_RAM_WARNING     85.0
#define THRESH_DISK_CRITICAL   90.0
#define THRESH_DISK_WARNING    80.0
#define THRESH_ERROR_HIGH      20
#define THRESH_ERROR_MEDIUM    10
#define THRESH_CRITICAL_ALERT  3
#define THRESH_LOGIN_FAIL      5
#define THRESH_TIMEOUT_ALERT   3

typedef struct {
    /* Rule-violation counts */
    int cpuCriticalSpikes;      /* readings > THRESH_CPU_CRITICAL */
    int cpuWarningSpikes;       /* readings > THRESH_CPU_WARNING  */
    int ramCriticalSpikes;
    int ramWarningSpikes;
    int diskCriticalSpikes;
    int diskWarningSpikes;

    /* Derived from patterns */
    int highErrorRate;          /* 0/1 flag */
    int criticalAlert;          /* 0/1 flag */
    int securityThreat;         /* 0/1 flag */
    int timeoutAlert;           /* 0/1 flag */

    /* Computed risk score 0-100 */
    int riskScore;

    /* Status string */
    char status[32];    /* "OK", "MODERATE", "WARNING", "CRITICAL" */
} AnomalyReport;

void runAnomalyDetection(AnomalyReport *out);
void displayAnomalyReport(const AnomalyReport *ar);
int  getRiskScore(void);    /* Returns last computed risk score */

#endif /* ANOMALY_H */
