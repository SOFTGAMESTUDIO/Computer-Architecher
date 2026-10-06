/* ================================================================
 *  anomaly.c  —  Rule-based Anomaly Detection & Risk Scoring
 *  C-IntelliGuard  |  Computer Architecture Project
 * ================================================================ */
#include "anomaly.h"
#include <stdio.h>
#include <string.h>

/* Last computed report cached here for external access */
static AnomalyReport s_lastReport;

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                   */
/* ------------------------------------------------------------------ */
static void countSpikes(double *arr, int n,
                        double critThresh, double warnThresh,
                        int *critOut, int *warnOut)
{
    *critOut = 0;
    *warnOut = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] >= critThresh)      (*critOut)++;
        else if (arr[i] >= warnThresh) (*warnOut)++;
    }
}

/* ------------------------------------------------------------------ */
/*  Risk score formula                                                 */
/* ------------------------------------------------------------------ */
static int calcRiskScore(const AnomalyReport *ar)
{
    int score = 0;

    int cpuCritPts = ar->cpuCriticalSpikes * 8;
    if (cpuCritPts > 40) cpuCritPts = 40;
    score += cpuCritPts;

    int cpuWarnPts = ar->cpuWarningSpikes * 3;
    if (cpuWarnPts > 15) cpuWarnPts = 15;
    score += cpuWarnPts;

    int ramCritPts = ar->ramCriticalSpikes * 8;
    if (ramCritPts > 32) ramCritPts = 32;
    score += ramCritPts;

    int ramWarnPts = ar->ramWarningSpikes * 3;
    if (ramWarnPts > 9) ramWarnPts = 9;
    score += ramWarnPts;

    int diskCritPts = ar->diskCriticalSpikes * 7;
    if (diskCritPts > 28) diskCritPts = 28;
    score += diskCritPts;

    int diskWarnPts = ar->diskWarningSpikes * 2;
    if (diskWarnPts > 6) diskWarnPts = 6;
    score += diskWarnPts;

    if (ar->highErrorRate)  score += 15;
    if (ar->criticalAlert)  score += 10;
    if (ar->securityThreat) score += 12;
    if (ar->timeoutAlert)   score += 7;

    if (score > 100) score = 100;
    return score;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */
void runAnomalyDetection(AnomalyReport *out)
{
    memset(out, 0, sizeof(AnomalyReport));

    if (!g_isLoaded) return;

    if (g_metrics.cpuCount > 0) {
        countSpikes(g_metrics.cpuValues, g_metrics.cpuCount,
                    THRESH_CPU_CRITICAL, THRESH_CPU_WARNING,
                    &out->cpuCriticalSpikes, &out->cpuWarningSpikes);
    }
    if (g_metrics.ramCount > 0) {
        countSpikes(g_metrics.ramValues, g_metrics.ramCount,
                    THRESH_RAM_CRITICAL, THRESH_RAM_WARNING,
                    &out->ramCriticalSpikes, &out->ramWarningSpikes);
    }
    if (g_metrics.diskCount > 0) {
        countSpikes(g_metrics.diskValues, g_metrics.diskCount,
                    THRESH_DISK_CRITICAL, THRESH_DISK_WARNING,
                    &out->diskCriticalSpikes, &out->diskWarningSpikes);
    }

    out->highErrorRate  = (g_patterns.errorCount    >= THRESH_ERROR_HIGH)    ? 1 : 0;
    out->criticalAlert  = (g_patterns.criticalCount >= THRESH_CRITICAL_ALERT) ? 1 : 0;
    out->securityThreat = (g_patterns.loginFailures >= THRESH_LOGIN_FAIL)    ? 1 : 0;
    out->timeoutAlert   = (g_patterns.timeoutCount  >= THRESH_TIMEOUT_ALERT) ? 1 : 0;

    out->riskScore = calcRiskScore(out);

    if      (out->riskScore >= 75) strcpy(out->status, "CRITICAL");
    else if (out->riskScore >= 50) strcpy(out->status, "WARNING");
    else if (out->riskScore >= 25) strcpy(out->status, "MODERATE");
    else                            strcpy(out->status, "OK");

    s_lastReport = *out;
}

int getRiskScore(void) { return s_lastReport.riskScore; }

/* ------------------------------------------------------------------ */
/*  Display                                                            */
/* ------------------------------------------------------------------ */
void displayAnomalyReport(const AnomalyReport *ar)
{
    if (!g_isLoaded) {
        printf("\n [!] No log file loaded. Please load a log file first (Option 1).\n");
        return;
    }

    printf("\n========================================================================\n");
    printf("                    ANOMALY DETECTION ENGINE REPORT                    \n");
    printf("========================================================================\n");

    printf("\n  RULE ENGINE  (threshold-based evaluation):\n");
    printf("  %-35s | %-10s | %-13s | %s\n", "Rule", "Threshold", "Actual", "Status");
    printf("  ------------------------------------+------------+---------------+--------\n");

    char buf[32];

#define RULE_ROW(label, thresh, actual, triggered) \
    printf("  %-35s | %-10s | %-13s | %s\n", \
           label, thresh, actual, triggered ? "[ALERT]" : "  OK  ")

    snprintf(buf, sizeof(buf), "%d spike(s)", ar->cpuCriticalSpikes);
    RULE_ROW("CPU > 90%  -> CRITICAL",          "> 90%",  buf, ar->cpuCriticalSpikes > 0);
    snprintf(buf, sizeof(buf), "%d spike(s)", ar->cpuWarningSpikes);
    RULE_ROW("CPU > 75%  -> WARNING",           "> 75%",  buf, ar->cpuWarningSpikes  > 0);
    snprintf(buf, sizeof(buf), "%d spike(s)", ar->ramCriticalSpikes);
    RULE_ROW("RAM > 90%  -> CRITICAL",          "> 90%",  buf, ar->ramCriticalSpikes > 0);
    snprintf(buf, sizeof(buf), "%d spike(s)", ar->ramWarningSpikes);
    RULE_ROW("RAM > 85%  -> WARNING",           "> 85%",  buf, ar->ramWarningSpikes  > 0);
    snprintf(buf, sizeof(buf), "%d spike(s)", ar->diskCriticalSpikes);
    RULE_ROW("DISK > 90% -> CRITICAL",          "> 90%",  buf, ar->diskCriticalSpikes > 0);
    snprintf(buf, sizeof(buf), "%d errors",   g_patterns.errorCount);
    RULE_ROW("ERROR > 20 -> HIGH RISK",         "> 20",   buf, ar->highErrorRate);
    snprintf(buf, sizeof(buf), "%d events",   g_patterns.criticalCount);
    RULE_ROW("CRITICAL > 3 -> ALERT",           "> 3",    buf, ar->criticalAlert);
    snprintf(buf, sizeof(buf), "%d failures", g_patterns.loginFailures);
    RULE_ROW("LOGIN FAIL > 5 -> SECURITY",      "> 5",    buf, ar->securityThreat);
    snprintf(buf, sizeof(buf), "%d events",   g_patterns.timeoutCount);
    RULE_ROW("TIMEOUT > 3 -> ALERT",            "> 3",    buf, ar->timeoutAlert);

#undef RULE_ROW

    /* Risk Score Bar */
    printf("\n------------------------------------------------------------------------\n");
    int score  = ar->riskScore;
    int barW   = 50;
    int filled = score * barW / 100;
    printf("  RISK SCORE : [");
    for (int i = 0; i < barW; i++) {
        if (i < filled) {
            putchar(score >= 75 ? '#' : score >= 50 ? '=' : score >= 25 ? '-' : '.');
        } else {
            putchar(' ');
        }
    }
    printf("] %d / 100\n", score);

    printf("\n  System Status : ");
    if      (score >= 75) printf("*** CRITICAL *** - Immediate attention required!\n");
    else if (score >= 50) printf("!!! WARNING !!!  - Elevated risk detected.\n");
    else if (score >= 25) printf("[~] MODERATE     - Monitor closely.\n");
    else                   printf("[OK] HEALTHY     - System operating normally.\n");

    /* Recommendations */
    printf("\n  RECOMMENDATIONS:\n");
    int recNum = 1;
    if (ar->cpuCriticalSpikes > 0 || ar->cpuWarningSpikes > 0)
        printf("  %d. Investigate CPU load spikes (%d critical, %d warning readings)\n",
               recNum++, ar->cpuCriticalSpikes, ar->cpuWarningSpikes);
    if (ar->ramCriticalSpikes > 0 || ar->ramWarningSpikes > 0)
        printf("  %d. Monitor RAM usage -- %d critical spike(s) detected\n",
               recNum++, ar->ramCriticalSpikes);
    if (ar->diskCriticalSpikes > 0)
        printf("  %d. Check disk I/O -- %d saturation event(s) recorded\n",
               recNum++, ar->diskCriticalSpikes);
    if (ar->highErrorRate)
        printf("  %d. Review error logs -- %d errors exceed safe threshold\n",
               recNum++, g_patterns.errorCount);
    if (ar->securityThreat)
        printf("  %d. Investigate failed logins (%d failures) -- possible brute force\n",
               recNum++, g_patterns.loginFailures);
    if (ar->timeoutAlert)
        printf("  %d. Check network/service timeouts (%d events)\n",
               recNum++, g_patterns.timeoutCount);
    if (recNum == 1)
        printf("  No critical recommendations at this time.\n");

    printf("========================================================================\n");
}
