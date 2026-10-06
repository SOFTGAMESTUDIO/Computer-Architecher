#include "file_analyzer.h"
#include <time.h>

CharFrequency g_charFreq;
WordFrequencyTable g_wordTable;
PatternStats g_patterns;
MetricCollection g_metrics;
char g_loadedFilePath[260] = "";
int g_isLoaded = 0;

void initAnalyzer(void) {
    memset(&g_charFreq, 0, sizeof(CharFrequency));
    memset(&g_wordTable, 0, sizeof(WordFrequencyTable));
    memset(&g_patterns, 0, sizeof(PatternStats));

    g_charFreq.shortestLineLen = 999999;
    g_charFreq.longestLineLen = 0;
    strcpy(g_charFreq.longestLineSnippet, "");
    strcpy(g_charFreq.shortestLineSnippet, "");

    if (g_metrics.cpuValues) free(g_metrics.cpuValues);
    if (g_metrics.ramValues) free(g_metrics.ramValues);
    if (g_metrics.diskValues) free(g_metrics.diskValues);

    g_metrics.cpuValues = NULL;
    g_metrics.cpuCount = 0;
    g_metrics.cpuCap = 0;

    g_metrics.ramValues = NULL;
    g_metrics.ramCount = 0;
    g_metrics.ramCap = 0;

    g_metrics.diskValues = NULL;
    g_metrics.diskCount = 0;
    g_metrics.diskCap = 0;

    g_metrics.eventCount = 0;
    g_isLoaded = 0;
}

void freeAnalyzer(void) {
    if (g_metrics.cpuValues) { free(g_metrics.cpuValues); g_metrics.cpuValues = NULL; }
    if (g_metrics.ramValues) { free(g_metrics.ramValues); g_metrics.ramValues = NULL; }
    if (g_metrics.diskValues) { free(g_metrics.diskValues); g_metrics.diskValues = NULL; }
    g_metrics.cpuCount = g_metrics.ramCount = g_metrics.diskCount = 0;
    g_metrics.cpuCap = g_metrics.ramCap = g_metrics.diskCap = 0;
}

static void addMetric(double **arr, int *count, int *cap, double val) {
    if (*count >= *cap) {
        int newCap = (*cap == 0) ? 64 : (*cap * 2);
        double *newArr = (double *)realloc(*arr, newCap * sizeof(double));
        if (!newArr) return;
        *arr = newArr;
        *cap = newCap;
    }
    (*arr)[(*count)++] = val;
}

static void addWord(const char *rawWord) {
    if (!rawWord || strlen(rawWord) == 0) return;

    char cleaned[MAX_WORD_LEN];
    int len = 0;
    for (int i = 0; rawWord[i] && len < MAX_WORD_LEN - 1; i++) {
        if (isalnum((unsigned char)rawWord[i]) || rawWord[i] == '-' || rawWord[i] == '_') {
            cleaned[len++] = (char)tolower((unsigned char)rawWord[i]);
        }
    }
    cleaned[len] = '\0';
    if (len < 2) return;

    /* Check if already in table */
    for (int i = 0; i < g_wordTable.uniqueCount; i++) {
        if (strcmp(g_wordTable.entries[i].word, cleaned) == 0) {
            g_wordTable.entries[i].count++;
            return;
        }
    }

    if (g_wordTable.uniqueCount < MAX_UNIQUE_WORDS) {
        strncpy(g_wordTable.entries[g_wordTable.uniqueCount].word, cleaned, MAX_WORD_LEN - 1);
        g_wordTable.entries[g_wordTable.uniqueCount].word[MAX_WORD_LEN - 1] = '\0';
        g_wordTable.entries[g_wordTable.uniqueCount].count = 1;
        g_wordTable.uniqueCount++;
    }
}

static void extractMetricsFromLine(const char *line) {
    /* Look for CPU = XX or CPU: XX */
    const char *p = line;
    while (*p) {
        if ((p[0] == 'C' || p[0] == 'c') &&
            (p[1] == 'P' || p[1] == 'p') &&
            (p[2] == 'U' || p[2] == 'u')) {
            const char *q = p + 3;
            while (*q == ' ' || *q == '=' || *q == ':') q++;
            double val = 0.0;
            if (sscanf(q, "%lf", &val) == 1) {
                if (val >= 0.0 && val <= 100.0) {
                    addMetric(&g_metrics.cpuValues, &g_metrics.cpuCount, &g_metrics.cpuCap, val);
                }
            }
        }

        if (((p[0] == 'R' || p[0] == 'r') && (p[1] == 'A' || p[1] == 'a') && (p[2] == 'M' || p[2] == 'm')) ||
            ((p[0] == 'M' || p[0] == 'm') && (p[1] == 'E' || p[1] == 'e') && (p[2] == 'M' || p[2] == 'm'))) {
            const char *q = p + 3;
            while (*q && *q != '=' && *q != ':' && !isdigit((unsigned char)*q)) q++;
            if (*q == '=' || *q == ':') q++;
            while (*q == ' ') q++;
            double val = 0.0;
            if (sscanf(q, "%lf", &val) == 1) {
                if (val >= 0.0 && val <= 100.0) {
                    addMetric(&g_metrics.ramValues, &g_metrics.ramCount, &g_metrics.ramCap, val);
                }
            }
        }

        if ((p[0] == 'D' || p[0] == 'd') &&
            (p[1] == 'I' || p[1] == 'i') &&
            (p[2] == 'S' || p[2] == 's') &&
            (p[3] == 'K' || p[3] == 'k')) {
            const char *q = p + 4;
            while (*q == ' ' || *q == '=' || *q == ':') q++;
            double val = 0.0;
            if (sscanf(q, "%lf", &val) == 1) {
                if (val >= 0.0 && val <= 100.0) {
                    addMetric(&g_metrics.diskValues, &g_metrics.diskCount, &g_metrics.diskCap, val);
                }
            }
        }
        p++;
    }
}

static void extractEvent(const char *line, int lineNum) {
    if (g_metrics.eventCount >= MAX_EVENTS) return;

    int severity = 0;
    char level[16] = "";

    if (strstr(line, "CRITICAL")) {
        severity = 5;
        strcpy(level, "CRITICAL");
    } else if (strstr(line, "ERROR")) {
        severity = 4;
        strcpy(level, "ERROR");
    } else if (strstr(line, "WARNING")) {
        severity = 3;
        strcpy(level, "WARNING");
    } else if (strstr(line, "SUCCESS")) {
        severity = 2;
        strcpy(level, "SUCCESS");
    } else if (strstr(line, "INFO")) {
        severity = 1;
        strcpy(level, "INFO");
    }

    if (severity >= 3) { /* Store high/notable severity events in event log */
        LogEvent *ev = &g_metrics.events[g_metrics.eventCount];
        strncpy(ev->level, level, sizeof(ev->level) - 1);
        ev->level[sizeof(ev->level) - 1] = '\0';
        ev->severity = severity;
        ev->lineNum = lineNum;

        /* Extract timestamp if bracketed [YYYY-MM-DD ...] */
        const char *openB = strchr(line, '[');
        const char *closeB = (openB) ? strchr(openB, ']') : NULL;
        if (openB && closeB && (closeB - openB < 30)) {
            int tlen = (int)(closeB - openB - 1);
            strncpy(ev->timestamp, openB + 1, tlen);
            ev->timestamp[tlen] = '\0';
        } else {
            snprintf(ev->timestamp, sizeof(ev->timestamp), "Line #%d", lineNum);
        }

        /* Message snippet */
        const char *msgStart = line;
        if (closeB) {
            msgStart = closeB + 1;
            /* Skip second bracket if [LEVEL] */
            const char *secOpen = strchr(msgStart, '[');
            const char *secClose = (secOpen) ? strchr(secOpen, ']') : NULL;
            if (secOpen && secClose && secClose - msgStart < 25) {
                msgStart = secClose + 1;
            }
        }
        while (*msgStart == ' ' || *msgStart == '\t') msgStart++;

        strncpy(ev->message, msgStart, sizeof(ev->message) - 1);
        ev->message[sizeof(ev->message) - 1] = '\0';

        /* Strip trailing newline */
        int mlen = (int)strlen(ev->message);
        while (mlen > 0 && (ev->message[mlen - 1] == '\r' || ev->message[mlen - 1] == '\n')) {
            ev->message[mlen - 1] = '\0';
            mlen--;
        }

        g_metrics.eventCount++;
    }
}

int processLogFile(const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return 0;
    }

    initAnalyzer();
    strncpy(g_loadedFilePath, filepath, sizeof(g_loadedFilePath) - 1);

    char line[2048];
    int lineNum = 0;

    while (fgets(line, sizeof(line), fp)) {
        lineNum++;
        g_charFreq.totalLines++;

        int len = (int)strlen(line);
        /* Trim trailing newline for line length measurement */
        int contentLen = len;
        while (contentLen > 0 && (line[contentLen - 1] == '\n' || line[contentLen - 1] == '\r')) {
            contentLen--;
        }

        if (contentLen > g_charFreq.longestLineLen) {
            g_charFreq.longestLineLen = contentLen;
            strncpy(g_charFreq.longestLineSnippet, line, sizeof(g_charFreq.longestLineSnippet) - 1);
            g_charFreq.longestLineSnippet[sizeof(g_charFreq.longestLineSnippet) - 1] = '\0';
            /* Clean newline from snippet */
            int sl = (int)strlen(g_charFreq.longestLineSnippet);
            while (sl > 0 && (g_charFreq.longestLineSnippet[sl - 1] == '\r' || g_charFreq.longestLineSnippet[sl - 1] == '\n')) {
                g_charFreq.longestLineSnippet[sl - 1] = '\0';
                sl--;
            }
        }

        if (contentLen > 0 && contentLen < g_charFreq.shortestLineLen) {
            g_charFreq.shortestLineLen = contentLen;
            strncpy(g_charFreq.shortestLineSnippet, line, sizeof(g_charFreq.shortestLineSnippet) - 1);
            g_charFreq.shortestLineSnippet[sizeof(g_charFreq.shortestLineSnippet) - 1] = '\0';
            int sl = (int)strlen(g_charFreq.shortestLineSnippet);
            while (sl > 0 && (g_charFreq.shortestLineSnippet[sl - 1] == '\r' || g_charFreq.shortestLineSnippet[sl - 1] == '\n')) {
                g_charFreq.shortestLineSnippet[sl - 1] = '\0';
                sl--;
            }
        }

        /* Character and frequency analysis */
        for (int i = 0; i < len; i++) {
            unsigned char c = (unsigned char)line[i];
            g_charFreq.totalChars++;

            if (c >= 'A' && c <= 'Z') {
                g_charFreq.alphaUpper[c - 'A']++;
            } else if (c >= 'a' && c <= 'z') {
                g_charFreq.alphaLower[c - 'a']++;
            } else if (c >= '0' && c <= '9') {
                g_charFreq.digits[c - '0']++;
            } else if (c == ' ') {
                g_charFreq.spaces++;
            } else if (c == '\t') {
                g_charFreq.tabs++;
            } else if (c == '\n' || c == '\r') {
                if (c == '\n') g_charFreq.newlines++;
            } else if (c < 128) {
                g_charFreq.special[c]++;
            }
        }

        /* Word extraction */
        char wordBuf[MAX_WORD_LEN];
        int wPos = 0;
        for (int i = 0; i <= len; i++) {
            unsigned char c = (unsigned char)line[i];
            if (isalnum(c) || c == '-' || c == '_') {
                if (wPos < MAX_WORD_LEN - 1) {
                    wordBuf[wPos++] = (char)c;
                }
            } else {
                if (wPos > 0) {
                    wordBuf[wPos] = '\0';
                    g_charFreq.totalWords++;
                    addWord(wordBuf);
                    wPos = 0;
                }
            }
        }

        /* Pattern Detection */
        if (strstr(line, "ERROR"))    g_patterns.errorCount++;
        if (strstr(line, "WARNING"))  g_patterns.warningCount++;
        if (strstr(line, "INFO"))     g_patterns.infoCount++;
        if (strstr(line, "CRITICAL")) g_patterns.criticalCount++;
        if (strstr(line, "FAILED"))   g_patterns.failedCount++;
        if (strstr(line, "SUCCESS"))  g_patterns.successCount++;
        if (strstr(line, "LOGIN"))    g_patterns.loginCount++;
        if (strstr(line, "TIMEOUT"))  g_patterns.timeoutCount++;
        if (strstr(line, "MEMORY"))   g_patterns.memoryCount++;
        if (strstr(line, "CPU"))      g_patterns.cpuCount++;
        if (strstr(line, "DISK"))     g_patterns.diskCount++;

        if (strstr(line, "LOGIN") && (strstr(line, "FAILED") || strstr(line, "FAIL"))) {
            g_patterns.loginFailures++;
        }

        /* Extract numerical metrics (CPU/RAM/DISK values) */
        extractMetricsFromLine(line);

        /* Store event for stack & queue */
        extractEvent(line, lineNum);
    }

    fclose(fp);

    if (g_charFreq.shortestLineLen == 999999) {
        g_charFreq.shortestLineLen = 0;
    }

    g_isLoaded = 1;
    return 1;
}

void displayCharFrequency(void) {
    if (!g_isLoaded) {
        printf("\n [!] No log file loaded. Please load a log file first (Option 1).\n");
        return;
    }

    printf("\n========================================================================\n");
    printf("                  CHARACTER & STRING FREQUENCY ANALYSIS                 \n");
    printf("========================================================================\n");
    printf(" Total Characters : %ld\n", g_charFreq.totalChars);
    printf(" Total Lines      : %ld\n", g_charFreq.totalLines);
    printf(" Total Words      : %ld\n", g_charFreq.totalWords);
    printf(" Longest Line     : %d chars -> \"%s\"\n", g_charFreq.longestLineLen, g_charFreq.longestLineSnippet);
    printf(" Shortest Line    : %d chars -> \"%s\"\n", g_charFreq.shortestLineLen, g_charFreq.shortestLineSnippet);
    printf(" Whitespace       : Spaces: %ld | Tabs: %ld | Newlines: %ld\n", g_charFreq.spaces, g_charFreq.tabs, g_charFreq.newlines);
    printf("------------------------------------------------------------------------\n");

    /* Alphabet Table */
    printf("\n [ALPHABET FREQUENCY (A-Z)]:\n");
    printf(" Char | Upper | Lower | Total | Char | Upper | Lower | Total\n");
    printf("------+-------+-------+-------+------+-------+-------+------\n");
    for (int i = 0; i < 13; i++) {
        int u1 = g_charFreq.alphaUpper[i];
        int l1 = g_charFreq.alphaLower[i];
        int t1 = u1 + l1;

        int j = i + 13;
        int u2 = g_charFreq.alphaUpper[j];
        int l2 = g_charFreq.alphaLower[j];
        int t2 = u2 + l2;

        printf("  %c   | %5d | %5d | %5d |  %c   | %5d | %5d | %5d\n",
               'A' + i, u1, l1, t1,
               'A' + j, u2, l2, t2);
    }

    /* Digit Table */
    printf("\n [DIGIT FREQUENCY (0-9)]:\n");
    printf(" Digit | Count |  %% of Total Chars | Visual Bar\n");
    printf("-------+-------+-------------------+------------------------------\n");
    for (int i = 0; i < 10; i++) {
        int count = g_charFreq.digits[i];
        double pct = (g_charFreq.totalChars > 0) ? ((double)count / g_charFreq.totalChars * 100.0) : 0.0;
        int barLen = (int)(pct * 2.5);
        if (barLen > 25) barLen = 25;
        char bar[30] = "";
        for (int b = 0; b < barLen; b++) bar[b] = '#';
        bar[barLen] = '\0';
        printf("   %d   | %5d | %16.2f%% | %-25s\n", i, count, pct, bar);
    }

    /* Common Special Characters */
    printf("\n [SPECIAL CHARACTERS FREQUENCY]:\n");
    const char specialList[] = "[]:=-/%.,()\"'{}#*@!;\\|><+`~";
    int printed = 0;
    for (int i = 0; specialList[i]; i++) {
        unsigned char c = (unsigned char)specialList[i];
        if (g_charFreq.special[c] > 0) {
            printf("  '%c' -> %-5d   ", c, g_charFreq.special[c]);
            printed++;
            if (printed % 4 == 0) printf("\n");
        }
    }
    if (printed % 4 != 0) printf("\n");
    printf("========================================================================\n");
}

static int compareWords(const void *a, const void *b) {
    const WordFreqEntry *wa = (const WordFreqEntry *)a;
    const WordFreqEntry *wb = (const WordFreqEntry *)b;
    return wb->count - wa->count;
}

void displayWordFrequency(int topN) {
    if (!g_isLoaded) {
        printf("\n [!] No log file loaded.\n");
        return;
    }

    if (g_wordTable.uniqueCount == 0) {
        printf("\n [!] No words recorded.\n");
        return;
    }

    /* Copy and sort */
    WordFreqEntry sorted[MAX_UNIQUE_WORDS];
    int count = g_wordTable.uniqueCount;
    memcpy(sorted, g_wordTable.entries, count * sizeof(WordFreqEntry));
    qsort(sorted, count, sizeof(WordFreqEntry), compareWords);

    if (topN > count) topN = count;

    printf("\n========================================================================\n");
    printf("                      TOP %d MOST FREQUENT WORDS                         \n", topN);
    printf("========================================================================\n");
    printf(" Rank | Word                 | Frequency | Percentage\n");
    printf("------+----------------------+-----------+-----------\n");
    for (int i = 0; i < topN; i++) {
        double pct = (g_charFreq.totalWords > 0) ? ((double)sorted[i].count / g_charFreq.totalWords * 100.0) : 0.0;
        printf("  #%-2d | %-20s | %9d | %8.2f%%\n", i + 1, sorted[i].word, sorted[i].count, pct);
    }
    printf("========================================================================\n");
}

void displayPatternAnalysis(void) {
    if (!g_isLoaded) {
        printf("\n [!] No log file loaded. Please load a log file first (Option 1).\n");
        return;
    }

    printf("\n========================================================================\n");
    printf("                          LOG PATTERN ANALYSIS                          \n");
    printf("========================================================================\n");
    printf(" Pattern Detected     | Occurrences | Impact / Significance\n");
    printf("----------------------+-------------+-----------------------------------\n");
    printf(" ERROR                | %11d | Operational errors and failures\n", g_patterns.errorCount);
    printf(" WARNING              | %11d | Threshold deviations and non-fatal alerts\n", g_patterns.warningCount);
    printf(" INFO                 | %11d | Routine lifecycle and state telemetry\n", g_patterns.infoCount);
    printf(" CRITICAL             | %11d | High severity system hazards\n", g_patterns.criticalCount);
    printf(" FAILED               | %11d | Failed transactions, DNS, or tasks\n", g_patterns.failedCount);
    printf(" SUCCESS              | %11d | Healthy transactions and handshakes\n", g_patterns.successCount);
    printf(" LOGIN                | %11d | Authentication events\n", g_patterns.loginCount);
    printf(" LOGIN FAILURES       | %11d | Security threat alerts (Failed auth)\n", g_patterns.loginFailures);
    printf(" TIMEOUT              | %11d | Network and connection latencies\n", g_patterns.timeoutCount);
    printf(" MEMORY               | %11d | RAM buffers and allocation events\n", g_patterns.memoryCount);
    printf(" CPU                  | %11d | Processor telemetry events\n", g_patterns.cpuCount);
    printf(" DISK                 | %11d | Storage I/O read/write activity\n", g_patterns.diskCount);
    printf("----------------------+-------------+-----------------------------------\n");
    printf(" Total Key Patterns   | %11d\n",
           g_patterns.errorCount + g_patterns.warningCount + g_patterns.infoCount +
           g_patterns.criticalCount + g_patterns.failedCount + g_patterns.successCount +
           g_patterns.loginCount + g_patterns.timeoutCount + g_patterns.memoryCount +
           g_patterns.cpuCount + g_patterns.diskCount);
    printf("========================================================================\n");
}

void generateSampleLogFile(const char *filepath, int numLines) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("\n [!] Failed to create file at: %s\n", filepath);
        return;
    }

    const char *users[] = {"admin", "guest", "db_worker", "operator", "analytics_svc", "root"};
    const char *services[] = {"auth-gateway", "order-engine", "cache-cluster", "db-pool", "event-bus", "payment-api"};

    srand((unsigned int)time(NULL));

    int baseYear = 2026, baseMonth = 10, baseDay = 6;
    int hour = 8, min = 0, sec = 0;

    for (int i = 0; i < numLines; i++) {
        sec += (rand() % 5) + 1;
        if (sec >= 60) { sec %= 60; min++; }
        if (min >= 60) { min %= 60; hour++; }
        if (hour >= 24) hour %= 24;

        char ts[32];
        snprintf(ts, sizeof(ts), "%04d-%02d-%02d %02d:%02d:%02d", baseYear, baseMonth, baseDay, hour, min, sec);

        int r = rand() % 100;
        if (r < 40) {
            /* Normal telemetry INFO */
            double cpu = 20.0 + (rand() % 450) / 10.0;
            double ram = 40.0 + (rand() % 350) / 10.0;
            double disk = 50.0 + (rand() % 250) / 10.0;
            fprintf(fp, "[%s] [INFO] CPU = %.1f%% | RAM = %.1f%% | DISK = %.1f%%\n", ts, cpu, ram, disk);
        } else if (r < 55) {
            /* SUCCESS event */
            const char *u = users[rand() % 6];
            fprintf(fp, "[%s] [SUCCESS] User %s LOGIN successful. Session token granted.\n", ts, u);
        } else if (r < 75) {
            /* WARNING */
            int wType = rand() % 4;
            if (wType == 0) {
                double ram = 85.0 + (rand() % 70) / 10.0;
                fprintf(fp, "[%s] [WARNING] RAM = %.1f%% exceeds warning threshold of 85.0%%\n", ts, ram);
            } else if (wType == 1) {
                fprintf(fp, "[%s] [WARNING] Worker thread pool utilization reached 84%%\n", ts);
            } else if (wType == 2) {
                fprintf(fp, "[%s] [WARNING] User guest LOGIN FAILED - invalid credentials [Attempt %d/5]\n", ts, (rand() % 5) + 1);
            } else {
                fprintf(fp, "[%s] [WARNING] Latency detected on service %s: response time %d ms\n", ts, services[rand() % 6], 500 + rand() % 900);
            }
        } else if (r < 90) {
            /* ERROR */
            int eType = rand() % 4;
            if (eType == 0) {
                fprintf(fp, "[%s] [ERROR] TIMEOUT while waiting for transaction response id=tx_%d\n", ts, 10000 + rand() % 90000);
            } else if (eType == 1) {
                fprintf(fp, "[%s] [ERROR] FAILED to resolve DNS for host internal-%s.cluster\n", ts, services[rand() % 6]);
            } else if (eType == 2) {
                fprintf(fp, "[%s] [ERROR] User guest LOGIN FAILED - brute force threshold triggered from IP 198.51.100.%d\n", ts, rand() % 255);
            } else {
                fprintf(fp, "[%s] [ERROR] FAILED background sync job: out of socket descriptors\n", ts);
            }
        } else {
            /* CRITICAL */
            int cType = rand() % 4;
            if (cType == 0) {
                double cpu = 92.0 + (rand() % 80) / 10.0;
                fprintf(fp, "[%s] [CRITICAL] CPU = %.1f%% spike detected! Core thermal threshold reached\n", ts, cpu);
            } else if (cType == 1) {
                double ram = 91.0 + (rand() % 80) / 10.0;
                fprintf(fp, "[%s] [CRITICAL] MEMORY pressure high: RAM = %.1f%% - kernel OOM-killer alert\n", ts, ram);
            } else if (cType == 2) {
                double disk = 93.0 + (rand() % 65) / 10.0;
                fprintf(fp, "[%s] [CRITICAL] DISK I/O queue saturated: DISK = %.1f%% bandwidth utilized\n", ts, disk);
            } else {
                fprintf(fp, "[%s] [CRITICAL] Cluster partition detected: node unreachable during quorum vote\n", ts);
            }
        }
    }

    fclose(fp);
    printf("\n [+] Generated realistic sample log with %d entries at: %s\n", numLines, filepath);
}
