#include "report.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MAKE_DIR(d) mkdir(d, 0755)
#endif

/* ------------------------------------------------------------------ */
/*  Build recommendations based on analysis                           */
/* ------------------------------------------------------------------ */
static int buildRecommendations(const AnomalyReport *ar,
                                 const StatResult *cpuSR,
                                 const StatResult *ramSR,
                                 const StatResult *diskSR,
                                 char recs[][128], int maxRecs) {
    int n = 0;

    if (ar->cpuCriticalSpikes > 0 && n < maxRecs)
        snprintf(recs[n++], 128, "Investigate CPU spikes (%d critical readings > %.0f%%)",
                 ar->cpuCriticalSpikes, THRESH_CPU_CRITICAL);

    if (cpuSR->valid && cpuSR->mean > 70.0 && n < maxRecs)
        snprintf(recs[n++], 128, "Optimize workload distribution (avg CPU %.1f%% is high)",
                 cpuSR->mean);

    if (ar->ramCriticalSpikes > 0 && n < maxRecs)
        snprintf(recs[n++], 128, "Increase RAM capacity or reduce memory-intensive services");

    if (ramSR->valid && ramSR->mean > 80.0 && n < maxRecs)
        snprintf(recs[n++], 128, "Review memory leaks — avg RAM at %.1f%% is concerning",
                 ramSR->mean);

    if (ar->diskCriticalSpikes > 0 && n < maxRecs)
        snprintf(recs[n++], 128, "Check disk health — %d critical DISK saturation events",
                 ar->diskCriticalSpikes);

    if (diskSR->valid && diskSR->mean > 75.0 && n < maxRecs)
        snprintf(recs[n++], 128, "Monitor disk bandwidth — avg DISK %.1f%% is elevated",
                 diskSR->mean);

    if (g_patterns.errorCount >= THRESH_ERROR_HIGH && n < maxRecs)
        snprintf(recs[n++], 128, "Root-cause analysis needed for %d ERROR events",
                 g_patterns.errorCount);

    if (ar->securityThreat && n < maxRecs)
        snprintf(recs[n++], 128, "Enable IP rate-limiting — %d failed login attempts detected",
                 g_patterns.loginFailures);

    if (g_patterns.timeoutCount > 0 && n < maxRecs)
        snprintf(recs[n++], 128, "Review network timeouts — %d TIMEOUT events in log",
                 g_patterns.timeoutCount);

    if (g_patterns.criticalCount > 0 && n < maxRecs)
        snprintf(recs[n++], 128, "Respond to %d CRITICAL events — escalate to on-call team",
                 g_patterns.criticalCount);

    if (n == 0 && n < maxRecs)
        snprintf(recs[n++], 128, "System is healthy — maintain current monitoring cadence");

    return n;
}

/* ------------------------------------------------------------------ */
/*  Write report body to a FILE* (stdout or file)                     */
/* ------------------------------------------------------------------ */
static void writeReport(FILE *fp, const AnomalyReport *ar,
                         const StatResult *cpuSR,
                         const StatResult *ramSR,
                         const StatResult *diskSR) {
    /* Timestamp */
    time_t now = time(NULL);
    char timeBuf[64];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    /* Extract filename from path */
    const char *fname = g_loadedFilePath;
    const char *slash = strrchr(fname, '\\');
    if (!slash) slash = strrchr(fname, '/');
    if (slash) fname = slash + 1;

    fprintf(fp, "\n");
    fprintf(fp, "========================================================================\n");
    fprintf(fp, "                    C-INTELLIGUARD SYSTEM REPORT                        \n");
    fprintf(fp, "========================================================================\n");
    fprintf(fp, "  Generated : %s\n", timeBuf);
    fprintf(fp, "  File      : %s\n", g_loadedFilePath);
    fprintf(fp, "========================================================================\n");

    /* --- FILE INFORMATION --- */
    fprintf(fp, "\n  FILE INFORMATION\n");
    fprintf(fp, "  -----------------------------------------------------------------------\n");
    fprintf(fp, "  File Name        : %-40s\n", fname);
    fprintf(fp, "  Total Lines      : %ld\n", g_charFreq.totalLines);
    fprintf(fp, "  Total Characters : %ld\n", g_charFreq.totalChars);
    fprintf(fp, "  Total Words      : %ld\n", g_charFreq.totalWords);
    fprintf(fp, "  Longest Line     : %d characters\n", g_charFreq.longestLineLen);
    fprintf(fp, "  Shortest Line    : %d characters\n", g_charFreq.shortestLineLen);

    /* --- LOG PATTERN SUMMARY --- */
    fprintf(fp, "\n  LOG ANALYSIS\n");
    fprintf(fp, "  -----------------------------------------------------------------------\n");
    fprintf(fp, "  %-18s : %d\n", "INFO",           g_patterns.infoCount);
    fprintf(fp, "  %-18s : %d\n", "SUCCESS",        g_patterns.successCount);
    fprintf(fp, "  %-18s : %d\n", "WARNING",        g_patterns.warningCount);
    fprintf(fp, "  %-18s : %d\n", "ERROR",          g_patterns.errorCount);
    fprintf(fp, "  %-18s : %d\n", "CRITICAL",       g_patterns.criticalCount);
    fprintf(fp, "  %-18s : %d\n", "FAILED events",  g_patterns.failedCount);
    fprintf(fp, "  %-18s : %d\n", "TIMEOUT events", g_patterns.timeoutCount);
    fprintf(fp, "  %-18s : %d\n", "LOGIN events",   g_patterns.loginCount);
    fprintf(fp, "  %-18s : %d\n", "Login FAILURES", g_patterns.loginFailures);

    /* --- STATISTICS --- */
    fprintf(fp, "\n  STATISTICS\n");
    fprintf(fp, "  -----------------------------------------------------------------------\n");
    if (cpuSR->valid) {
        fprintf(fp, "  CPU  | Mean: %6.2f%%  Median: %6.2f%%  StdDev: %6.2f  Min: %5.2f  Max: %5.2f\n",
                cpuSR->mean, cpuSR->median, cpuSR->stddev, cpuSR->minimum, cpuSR->maximum);
    } else {
        fprintf(fp, "  CPU  | No data\n");
    }
    if (ramSR->valid) {
        fprintf(fp, "  RAM  | Mean: %6.2f%%  Median: %6.2f%%  StdDev: %6.2f  Min: %5.2f  Max: %5.2f\n",
                ramSR->mean, ramSR->median, ramSR->stddev, ramSR->minimum, ramSR->maximum);
    } else {
        fprintf(fp, "  RAM  | No data\n");
    }
    if (diskSR->valid) {
        fprintf(fp, "  DISK | Mean: %6.2f%%  Median: %6.2f%%  StdDev: %6.2f  Min: %5.2f  Max: %5.2f\n",
                diskSR->mean, diskSR->median, diskSR->stddev, diskSR->minimum, diskSR->maximum);
    } else {
        fprintf(fp, "  DISK | No data\n");
    }

    /* --- ANOMALIES --- */
    fprintf(fp, "\n  ANOMALY SUMMARY\n");
    fprintf(fp, "  -----------------------------------------------------------------------\n");
    fprintf(fp, "  %-28s : %d\n", "CPU Critical Spikes (>90%)",  ar->cpuCriticalSpikes);
    fprintf(fp, "  %-28s : %d\n", "CPU Warning Spikes  (>75%)",  ar->cpuWarningSpikes);
    fprintf(fp, "  %-28s : %d\n", "RAM Critical Spikes (>90%)",  ar->ramCriticalSpikes);
    fprintf(fp, "  %-28s : %d\n", "RAM Warning Spikes  (>85%)",  ar->ramWarningSpikes);
    fprintf(fp, "  %-28s : %d\n", "DISK Critical Spikes(>90%)",  ar->diskCriticalSpikes);
    fprintf(fp, "  %-28s : %d\n", "Login Failures",               g_patterns.loginFailures);

    /* --- RISK ANALYSIS --- */
    fprintf(fp, "\n  RISK ANALYSIS\n");
    fprintf(fp, "  -----------------------------------------------------------------------\n");
    fprintf(fp, "  Risk Score    : %d / 100\n", ar->riskScore);
    fprintf(fp, "  System Status : %s\n", ar->status);

    int barFill = ar->riskScore / 4;
    fprintf(fp, "  Risk Meter    : [");
    for (int i = 0; i < 25; i++) fprintf(fp, i < barFill ? "#" : "-");
    fprintf(fp, "]\n");

    /* --- RECOMMENDATIONS --- */
    char recs[10][128];
    int nRecs = buildRecommendations(ar, cpuSR, ramSR, diskSR, recs, 10);

    fprintf(fp, "\n  RECOMMENDATIONS\n");
    fprintf(fp, "  -----------------------------------------------------------------------\n");
    for (int i = 0; i < nRecs; i++) {
        fprintf(fp, "  %2d. %s\n", i + 1, recs[i]);
    }

    /* --- TOP LOG EVENTS --- */
    fprintf(fp, "\n  TOP CRITICAL/ERROR EVENTS\n");
    fprintf(fp, "  -----------------------------------------------------------------------\n");
    int shown = 0;
    for (int i = 0; i < g_metrics.eventCount && shown < 10; i++) {
        LogEvent *ev = &g_metrics.events[i];
        if (ev->severity >= 4) {  /* ERROR or CRITICAL */
            fprintf(fp, "  [L%-4d] [%-8s] %s\n", ev->lineNum, ev->level, ev->message);
            shown++;
        }
    }
    if (shown == 0) fprintf(fp, "  No high-severity events found.\n");

    fprintf(fp, "\n========================================================================\n");
    fprintf(fp, "                          END OF REPORT                                 \n");
    fprintf(fp, "========================================================================\n\n");
}

/* Ensure 'reports' directory exists */
static void ensureReportsDirectory(void) {
    MAKE_DIR("reports");
}

/* Ensure report file is always located in reports/ directory */
static void resolveReportPath(const char *input, char *output, size_t maxLen) {
    ensureReportsDirectory();
    if (!input || strlen(input) == 0) {
        snprintf(output, maxLen, "reports/intelliguard_report.txt");
        return;
    }
    /* If already prefixed with reports/ or reports\ */
    if (strncmp(input, "reports/", 8) == 0 || strncmp(input, "reports\\", 8) == 0) {
        snprintf(output, maxLen, "%s", input);
        return;
    }
    /* If it contains no directory separator, automatically save in reports/ */
    if (!strchr(input, '/') && !strchr(input, '\\')) {
        snprintf(output, maxLen, "reports/%s", input);
        return;
    }
    snprintf(output, maxLen, "%s", input);
}

/* ------------------------------------------------------------------ */
/*  Public functions                                                   */
/* ------------------------------------------------------------------ */
void generateFullReport(void) {
    if (!g_isLoaded) {
        printf("\n [!] No log file loaded. Please load a log file first (Option 1).\n");
        return;
    }

    /* Run stats and anomaly detection */
    StatResult cpuSR  = computeStats(g_metrics.cpuValues,  g_metrics.cpuCount);
    StatResult ramSR  = computeStats(g_metrics.ramValues,  g_metrics.ramCount);
    StatResult diskSR = computeStats(g_metrics.diskValues, g_metrics.diskCount);

    AnomalyReport ar;
    runAnomalyDetection(&ar);

    writeReport(stdout, &ar, &cpuSR, &ramSR, &diskSR);

    /* Automatically persist every generated report into reports/ directory */
    ensureReportsDirectory();
    const char *autoPath = "reports/intelliguard_report.txt";
    FILE *fp = fopen(autoPath, "w");
    if (fp) {
        writeReport(fp, &ar, &cpuSR, &ramSR, &diskSR);
        fclose(fp);
        printf(" [+] Report automatically saved to: %s\n\n", autoPath);
    }
}

void saveReportToFile(const char *outputPath) {
    if (!g_isLoaded) {
        printf("\n [!] No log file loaded.\n");
        return;
    }

    char resolved[260];
    resolveReportPath(outputPath, resolved, sizeof(resolved));

    FILE *fp = fopen(resolved, "w");
    if (!fp) {
        printf("\n [!] Cannot create report file at: %s\n", resolved);
        return;
    }

    StatResult cpuSR  = computeStats(g_metrics.cpuValues,  g_metrics.cpuCount);
    StatResult ramSR  = computeStats(g_metrics.ramValues,  g_metrics.ramCount);
    StatResult diskSR = computeStats(g_metrics.diskValues, g_metrics.diskCount);

    AnomalyReport ar;
    runAnomalyDetection(&ar);

    writeReport(fp, &ar, &cpuSR, &ramSR, &diskSR);
    fclose(fp);

    printf("\n [+] Report successfully saved to: %s\n", resolved);
}
