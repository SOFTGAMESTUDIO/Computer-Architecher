/*
 * ============================================================
 *   C-IntelliGuard — Intelligent Log Analysis & Optimization
 * ============================================================
 *  Modules:
 *    file_analyzer  — File I/O, char/word/pattern frequency
 *    statistics     — Mean, median, mode, variance, std dev
 *    anomaly        — Rule-based anomaly detection & risk score
 *    stack_ops      — LIFO stack with interactive operations
 *    queue_ops      — FIFO queue with interactive operations
 *    kremoval       — Max-K removal (greedy + stack)
 *    report         — Integrated full system report
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_analyzer.h"
#include "statistics.h"
#include "anomaly.h"
#include "stack_ops.h"
#include "queue_ops.h"
#include "kremoval.h"
#include "report.h"

/* ------------------------------------------------------------------ */
/*  Banner & Menu                                                      */
/* ------------------------------------------------------------------ */
static void printBanner(void) {
    printf("\n");
    printf("  +===========================================================+\n");
    printf("  |                                                           |\n");
    printf("  |          C - I N T E L L I G U A R D                     |\n");
    printf("  |   Intelligent Log Analysis & Optimization System          |\n");
    printf("  |                                                           |\n");
    printf("  +===========================================================+\n");
    printf("  |  Modules: FileIO | Statistics | Anomaly | Stack | Queue   |\n");
    printf("  |           MaxK-Removal | Report Generator                 |\n");
    printf("  +===========================================================+\n\n");
}

static void printMenu(void) {
    const char *loadStatus = g_isLoaded ? g_loadedFilePath : "No file loaded";
    /* Shorten path for display */
    const char *slash = strrchr(loadStatus, '\\');
    if (!slash) slash = strrchr(loadStatus, '/');
    const char *shortName = slash ? slash + 1 : loadStatus;

    printf("\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |              C-INTELLIGUARD  MAIN MENU               |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |  Loaded : %-42s|\n", shortName);
    printf("  +------------------------------------------------------+\n");
    printf("  |  1.  Load Log File                                   |\n");
    printf("  |  2.  Generate Sample Log File                        |\n");
    printf("  |  3.  Character & String Frequency Analysis           |\n");
    printf("  |  4.  Log Pattern Analysis                            |\n");
    printf("  |  5.  Word Frequency (Top 20)                         |\n");
    printf("  |  6.  Statistical Analysis (CPU / RAM / DISK)         |\n");
    printf("  |  7.  Anomaly Detection & Risk Score                  |\n");
    printf("  |  8.  Stack Operations                                |\n");
    printf("  |  9.  Queue Operations                                |\n");
    printf("  |  10. Maximum-K Removal (Greedy Algorithm)            |\n");
    printf("  |  11. Generate & Display Full Report                  |\n");
    printf("  |  12. Save Full Report to File                        |\n");
    printf("  |  0.  Exit                                            |\n");
    printf("  +------------------------------------------------------+\n");
    printf("  Enter choice: ");
}

/* ------------------------------------------------------------------ */
/*  Option handlers                                                    */
/* ------------------------------------------------------------------ */
static void handleLoadFile(void) {
    char path[260];
    printf("\n  Enter path to log file (or press ENTER for default 'logs/system.log'): ");
    if (!fgets(path, sizeof(path), stdin)) return;
    path[strcspn(path, "\r\n")] = '\0';

    if (strlen(path) == 0) {
        strcpy(path, "logs/system.log");
    }

    printf("\n  Loading: %s ...\n", path);
    if (processLogFile(path)) {
        printf("  [OK] File loaded successfully!\n");
        printf("       Lines      : %ld\n", g_charFreq.totalLines);
        printf("       Characters : %ld\n", g_charFreq.totalChars);
        printf("       Words      : %ld\n", g_charFreq.totalWords);
        printf("       Events     : %d high-severity events captured\n", g_metrics.eventCount);
    } else {
        printf("  [!] Failed to open file: %s\n", path);
        printf("      Make sure the path is correct and the file exists.\n");
    }
}

static void handleGenerateSample(void) {
    char path[260];
    int  numLines;

    printf("\n  Enter output path (ENTER = 'logs/system.log'): ");
    if (!fgets(path, sizeof(path), stdin)) return;
    path[strcspn(path, "\r\n")] = '\0';
    if (strlen(path) == 0) strcpy(path, "logs/system.log");

    printf("  Number of log entries to generate (ENTER = 500): ");
    char buf[32];
    if (!fgets(buf, sizeof(buf), stdin)) return;
    buf[strcspn(buf, "\r\n")] = '\0';
    numLines = (strlen(buf) > 0) ? atoi(buf) : 500;
    if (numLines < 10)   numLines = 10;
    if (numLines > 50000) numLines = 50000;

    generateSampleLogFile(path, numLines);

    char reload;
    printf("  Load this file now? (y/n): ");
    if (scanf(" %c", &reload) == 1 && (reload == 'y' || reload == 'Y')) {
        while (getchar() != '\n');
        if (processLogFile(path)) {
            printf("  [OK] File loaded: %ld lines, %ld chars.\n",
                   g_charFreq.totalLines, g_charFreq.totalChars);
        }
    } else {
        while (getchar() != '\n');
    }
}

static void handleSaveReport(void) {
    char path[260];
    printf("\n  Enter report filename (saved in 'reports/' folder, ENTER = 'intelliguard_report.txt'): ");
    if (!fgets(path, sizeof(path), stdin)) return;
    path[strcspn(path, "\r\n")] = '\0';
    if (strlen(path) == 0) strcpy(path, "intelliguard_report.txt");
    saveReportToFile(path);
}

/* ------------------------------------------------------------------ */
/*  Main                                                               */
/* ------------------------------------------------------------------ */
int main(void) {
    printBanner();

    /* Try auto-loading default log on startup */
    if (processLogFile("logs/system.log")) {
        printf("  [AUTO] Loaded default log file: logs/system.log\n");
        printf("         Lines: %ld | Characters: %ld | Events: %d\n\n",
               g_charFreq.totalLines, g_charFreq.totalChars, g_metrics.eventCount);
    } else {
        printf("  [INFO] No default log found at 'logs/system.log'.\n");
        printf("         Use option 1 to load or option 2 to generate a sample log.\n\n");
    }

    int running = 1;
    while (running) {
        printMenu();

        int choice;
        char inputBuf[32];
        if (!fgets(inputBuf, sizeof(inputBuf), stdin)) break;
        inputBuf[strcspn(inputBuf, "\r\n")] = '\0';

        if (strlen(inputBuf) == 0) continue;
        choice = atoi(inputBuf);

        switch (choice) {
            case 0:
                printf("\n  Goodbye! C-IntelliGuard shutting down.\n\n");
                running = 0;
                break;

            case 1:
                handleLoadFile();
                break;

            case 2:
                handleGenerateSample();
                break;

            case 3:
                displayCharFrequency();
                break;

            case 4:
                displayPatternAnalysis();
                break;

            case 5:
                displayWordFrequency(20);
                break;

            case 6:
                displayStatisticalAnalysis();
                break;

            case 7: {
                AnomalyReport ar;
                runAnomalyDetection(&ar);
                displayAnomalyReport(&ar);
                break;
            }

            case 8:
                displayStackModule();
                break;

            case 9:
                displayQueueModule();
                break;

            case 10:
                displayKRemovalModule();
                break;

            case 11:
                generateFullReport();
                break;

            case 12:
                handleSaveReport();
                break;

            default:
                printf("\n  [!] Invalid choice. Please enter 0-12.\n");
        }
    }

    freeAnalyzer();
    return 0;
}
