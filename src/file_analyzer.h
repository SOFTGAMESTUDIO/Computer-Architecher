#ifndef FILE_ANALYZER_H
#define FILE_ANALYZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 64
#define MAX_UNIQUE_WORDS 1000
#define MAX_SNIPPET_LEN 256
#define MAX_EVENTS 500

typedef struct {
    int alphaUpper[26];
    int alphaLower[26];
    int digits[10];
    int special[128];
    long spaces;
    long tabs;
    long newlines;
    long totalChars;
    long totalLines;
    long totalWords;
    int longestLineLen;
    char longestLineSnippet[MAX_SNIPPET_LEN];
    int shortestLineLen;
    char shortestLineSnippet[MAX_SNIPPET_LEN];
} CharFrequency;

typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordFreqEntry;

typedef struct {
    WordFreqEntry entries[MAX_UNIQUE_WORDS];
    int uniqueCount;
} WordFrequencyTable;

typedef struct {
    int errorCount;
    int warningCount;
    int infoCount;
    int criticalCount;
    int failedCount;
    int successCount;
    int loginCount;
    int timeoutCount;
    int memoryCount;
    int cpuCount;
    int diskCount;
    int loginFailures;
} PatternStats;

typedef struct {
    char level[16];
    char timestamp[32];
    char message[MAX_SNIPPET_LEN];
    int severity;      /* 1: INFO, 2: SUCCESS, 3: WARNING, 4: ERROR, 5: CRITICAL */
    int lineNum;
} LogEvent;

typedef struct {
    double *cpuValues;
    int cpuCount;
    int cpuCap;

    double *ramValues;
    int ramCount;
    int ramCap;

    double *diskValues;
    int diskCount;
    int diskCap;

    LogEvent events[MAX_EVENTS];
    int eventCount;
} MetricCollection;

/* Global state containers */
extern CharFrequency g_charFreq;
extern WordFrequencyTable g_wordTable;
extern PatternStats g_patterns;
extern MetricCollection g_metrics;
extern char g_loadedFilePath[260];
extern int g_isLoaded;

/* Function prototypes */
void initAnalyzer(void);
void freeAnalyzer(void);
int processLogFile(const char *filepath);
void displayCharFrequency(void);
void displayPatternAnalysis(void);
void displayWordFrequency(int topN);
void generateSampleLogFile(const char *filepath, int numLines);

#endif /* FILE_ANALYZER_H */
