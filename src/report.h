#ifndef REPORT_H
#define REPORT_H

#include "file_analyzer.h"
#include "statistics.h"
#include "anomaly.h"

/* Generate and display full report to console */
void generateFullReport(void);

/* Save report to a file */
void saveReportToFile(const char *outputPath);

#endif /* REPORT_H */
