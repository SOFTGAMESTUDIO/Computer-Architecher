# 🧠 C-IntelliGuard — Intelligent Log Analysis & Optimization System

> **A High-Performance Systems Application written in C (C11)**  
> Unifying File I/O, Statistical Modeling, Anomaly Detection, Data Structures (Stack & Queue), and Greedy Monotonic Optimization into one production-ready system.

---

## 📌 Architecture Overview

```text
                             ┌────────────────────────┐
                             │    LOG / TELEMETRY     │
                             │  (logs/system.log)     │
                             └───────────┬────────────┘
                                         │
                                         ▼
                             ┌────────────────────────┐
                             │     FILE ANALYZER      │
                             │ • Character Frequency  │
                             │ • Word Frequency (Top) │
                             │ • Regex-like Patterns  │
                             └───────────┬────────────┘
                                         │
                 ┌───────────────────────┼───────────────────────┐
                 │                       │                       │
                 ▼                       ▼                       ▼
      ┌─────────────────────┐ ┌─────────────────────┐ ┌─────────────────────┐
      │     STATISTICS      │ │  ANOMALY DETECTION  │ │   EVENT DISPATCH    │
      │ • Mean & Median     │ │ • Threshold Rules   │ │ • LIFO Stack (Ops)  │
      │ • Std Deviation     │ │ • Severity Scoring  │ │ • FIFO Queue (Ops)  │
      │ • Min / Max Range   │ │ • 0-100 Risk Score  │ │ • Monotonic Max-K   │
      └──────────┬──────────┘ └──────────┬──────────┘ └──────────┬──────────┘
                 │                       │                       │
                 └───────────────────────┼───────────────────────┘
                                         │
                                         ▼
                             ┌────────────────────────┐
                             │    REPORT GENERATOR    │
                             │ • Terminal Dashboard   │
                             │ • File Export (.txt)   │
                             │ • Recommendations      │
                             └────────────────────────┘
```

---

## 📂 Project Structure

```text
Computer Architecture/
├── Makefile                # Cross-platform build script (Windows/Linux/macOS)
├── build.bat               # One-click Windows build script
├── run.bat                 # One-click Windows execution script
├── test_suite.ps1          # Automated 6-stage regression test suite
├── intelliguard.exe        # Compiled executable
├── logs/
│   └── system.log          # Realistic production sample log file
├── reports/
│   └── intelliguard_report.txt  # Generated analysis report export
└── src/
    ├── file_analyzer.c/h   # File processing, character & word frequencies
    ├── statistics.c/h      # Statistical metric calculations (mean, std dev)
    ├── anomaly.c/h         # Rule-based threshold anomaly detection
    ├── stack_ops.c/h       # Interactive LIFO event stack processor
    ├── queue_ops.c/h       # Interactive FIFO event queue processor
    ├── kremoval.c/h        # Greedy monotonic stack algorithm (Max-K removal)
    ├── report.c/h          # Comprehensive formatted report builder
    └── main.c              # Interactive CLI menu & system coordinator
```

---

## 🚀 Quick Start & Building

### Method 1: One-Click Scripts (Windows)
```cmd
# Build the project
build.bat

# Run the project
run.bat
```

### Method 2: GCC Direct Command
```bash
gcc -Wall -Wextra -std=c11 -Isrc src/*.c -o intelliguard.exe -lm
./intelliguard.exe
```

### Method 3: Make / MinGW-Make
```bash
make        # Compile intelliguard
make run    # Compile & execute
make clean  # Clean binary and object files
```

### Method 4: Run Automated Test Suite
```powershell
powershell -ExecutionPolicy Bypass -File .\test_suite.ps1
```

---

## 🧪 Automated Test Verification

All modules have been tested with the included test suite (`test_suite.ps1`):

| Test | Module / Feature | Validation Criteria | Status |
|---|---|---|:---:|
| **1** | Clean Compilation | Compiled with `gcc -Wall -Wextra` with 0 warnings & 0 errors | ✅ PASS |
| **2** | Analyzer & Statistics | Correct metric parsing, mean/median/stddev, report export | ✅ PASS |
| **3** | Stack Operations (LIFO) | Push, pop, peek, visual render, empty/full boundaries | ✅ PASS |
| **4** | Queue Operations (FIFO) | Enqueue, dequeue, tree & linear views, batch drain | ✅ PASS |
| **5** | Greedy Max-K Removal | Verified `982573` (K=2) produces `9873` with step trace | ✅ PASS |
| **6** | Dynamic Log Generator | Generated 30 log lines on-the-fly and parsed correctly | ✅ PASS |

---

## 📋 Interactive Menu Options

When launched, C-IntelliGuard presents an interactive command interface:

```text
  +------------------------------------------------------+
  |              C-INTELLIGUARD  MAIN MENU               |
  +------------------------------------------------------+
  |  1.  Load Log File                                   |
  |  2.  Generate Sample Log File                        |
  |  3.  Character & String Frequency Analysis           |
  |  4.  Log Pattern Analysis                            |
  |  5.  Word Frequency (Top 20)                         |
  |  6.  Statistical Analysis (CPU / RAM / DISK)         |
  |  7.  Anomaly Detection & Risk Score                  |
  |  8.  Stack Operations                                |
  |  9.  Queue Operations                                |
  |  10. Maximum-K Removal (Greedy Algorithm)            |
  |  11. Generate & Display Full Report                  |
  |  12. Save Full Report to File                        |
  |  0.  Exit                                            |
  +------------------------------------------------------+
```

### Key Modules Explained

1. **Character & Word Frequency Analysis (Option 3 & 5)**:
   - Tracks ASCII occurrence frequencies, printable characters, digits, uppercase/lowercase distribution.
   - Computes top 20 most frequent keywords (e.g. `INFO`, `CPU`, `RAM`, `ERROR`, `CRITICAL`).

2. **Statistical Engine (Option 6)**:
   - Parses telemetry data (`CPU`, `RAM`, `DISK` percentages).
   - Computes **Mean**, **Median**, **Variance**, **Standard Deviation**, and **Min/Max ranges** with $O(N)$ efficiency.

3. **Anomaly Detection & Risk Score (Option 7)**:
   - Evaluates multi-factor threshold rules (CPU > 90%, RAM > 90%, DISK > 90%, Failed Logins > 5).
   - Computes weighted Risk Index (0 - 100) with intuitive visual risk bars and action recommendations.

4. **LIFO Stack Operations (Option 8)**:
   - Implements a high-severity event stack with ASCII visual rendering, push, pop, peek, and real-time buffer management.

5. **FIFO Queue Operations (Option 9)**:
   - Implements a chronological event stream buffer with both tree view and pipeline linear view, peek front/rear, and batch dispatching.

6. **Greedy Monotonic Stack Optimization (Option 10)**:
   - Solves the classic Maximum-K removal problem in $O(N)$ time.
   - Removes $K$ digits from a sequence to construct the maximum possible number, providing complete step-by-step diagnostic trace.

7. **Full System Report (Option 11 & 12)**:
   - Aggregates all analyses into an executive audit report that can be displayed or exported directly to `reports/intelliguard_report.txt`.

---

## ⚡ Algorithm Complexity

| Module / Operation | Time Complexity | Space Complexity |
|---|:---:|:---:|
| File Streaming & Parser | $O(N)$ | $O(1)$ |
| Word Frequency Hashing | $O(W \log W)$ | $O(W)$ |
| Statistical Computations | $O(N \log N)$ | $O(N)$ |
| Anomaly Scoring | $O(1)$ | $O(1)$ |
| Stack Operations (Push/Pop/Peek) | $O(1)$ | $O(S)$ |
| Queue Operations (Enqueue/Dequeue) | $O(1)$ | $O(Q)$ |
| Greedy Monotonic Max-K Removal | $O(N)$ | $O(N)$ |
| Full Report Generation | $O(N)$ | $O(1)$ |

---

## 🛠️ Standards & Quality
- **Standard**: ANSI C / C11
- **Compiler Compliance**: GCC with `-Wall -Wextra` flags (0 warnings, 0 errors)
- **Memory Safety**: Fixed-size bounded buffers, safe string functions (`snprintf`, `strncpy`, `strcspn`) to prevent buffer overflows.
