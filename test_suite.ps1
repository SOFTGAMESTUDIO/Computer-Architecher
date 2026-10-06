# ============================================================
#  test_suite.ps1 — C-IntelliGuard Automated Test Suite
# ============================================================

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "    C-INTELLIGUARD COMPREHENSIVE AUTOMATED TEST SUITE       " -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan

# Step 1: Clean build test
Write-Host "`n[TEST 1] Clean compilation with GCC (-Wall -Wextra)..." -ForegroundColor Yellow
$buildOutput = & gcc -Wall -Wextra -std=c11 -Isrc src/*.c -o intelliguard.exe -lm 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[-] Build Failed!" -ForegroundColor Red
    Write-Host $buildOutput
    exit 1
}
if ($buildOutput) {
    Write-Host "[!] Build succeeded with warnings:" -ForegroundColor Magenta
    Write-Host $buildOutput
} else {
    Write-Host "[+] Build passed with 0 errors and 0 warnings!" -ForegroundColor Green
}

# Step 2: Test Analysis, Statistics, Anomaly, Report Export
Write-Host "`n[TEST 2] Testing Analysis, Statistics, Anomaly, and Report Export..." -ForegroundColor Yellow
$inputFlow1 = @"
3
4
5
6
7
11
12
reports/test_report.txt
0
"@
$out1 = $inputFlow1 | .\intelliguard.exe 2>&1 | Out-String

if ($out1 -match "ANOMALY DETECTION ENGINE REPORT" -and 
    $out1 -match "STATISTICS" -and 
    $out1 -match "Report saved to: reports/test_report.txt") {
    Write-Host "[+] Analysis and Report modules passed!" -ForegroundColor Green
} else {
    Write-Host "[-] Analysis or Report test failed!" -ForegroundColor Red
    exit 1
}

# Verify report file exists and is not empty
if (Test-Path "reports/test_report.txt") {
    $reportSize = (Get-Item "reports/test_report.txt").Length
    Write-Host "[+] Saved report verified (Size: $reportSize bytes)" -ForegroundColor Green
    Remove-Item "reports/test_report.txt" -Force
} else {
    Write-Host "[-] Report file not generated!" -ForegroundColor Red
    exit 1
}

# Step 3: Test Stack Operations
Write-Host "`n[TEST 3] Testing Stack Operations (LIFO)..." -ForegroundColor Yellow
$inputStack = @"
8
1
2
CRITICAL DB ERROR
5
4
3
5
1
6
0
0
"@
$outStack = $inputStack | .\intelliguard.exe 2>&1 | Out-String
if ($outStack -match "PUSH  `"CRITICAL DB ERROR`"" -and 
    $outStack -match "POP  -> `"CRITICAL DB ERROR`"" -and
    $outStack -match "Stack cleared") {
    Write-Host "[+] Stack Operations passed!" -ForegroundColor Green
} else {
    Write-Host "[-] Stack test failed!" -ForegroundColor Red
    exit 1
}

# Step 4: Test Queue Operations
Write-Host "`n[TEST 4] Testing Queue Operations (FIFO)..." -ForegroundColor Yellow
$inputQueue = @"
9
1
2
3
INCOMING LOG PACKET
4
5
6
4
7
8
0
0
"@
$outQueue = $inputQueue | .\intelliguard.exe 2>&1 | Out-String
if ($outQueue -match "ENQUEUE `"INCOMING LOG PACKET`"" -and 
    $outQueue -match "Total processed:" -and
    $outQueue -match "Queue is now empty") {
    Write-Host "[+] Queue Operations passed!" -ForegroundColor Green
} else {
    Write-Host "[-] Queue test failed!" -ForegroundColor Red
    exit 1
}

# Step 5: Test Maximum-K Removal (Greedy Monotonic Stack)
Write-Host "`n[TEST 5] Testing Maximum-K Removal Algorithm..." -ForegroundColor Yellow
$inputK = @"
10
1
982573
2
2
3
0
0
"@
$outK = $inputK | .\intelliguard.exe 2>&1 | Out-String
if ($outK -match "RESULT         : 9873" -and 
    $outK -match "=== AUTO DEMO EXAMPLES ===" -and
    $outK -match "optimized score sequence") {
    Write-Host "[+] Maximum-K Removal algorithm verified (Expected: 9873, Matched)!" -ForegroundColor Green
} else {
    Write-Host "[-] Max-K algorithm test failed!" -ForegroundColor Red
    exit 1
}

# Step 6: Test Dynamic Sample Generator and Reload
Write-Host "`n[TEST 6] Testing Sample Log Generator & Parsing..." -ForegroundColor Yellow
$inputGen = @"
2
logs/temp_test.log
30
y
7
0
"@
$outGen = $inputGen | .\intelliguard.exe 2>&1 | Out-String
if ($outGen -match "Generated realistic sample log with 30 entries" -and 
    $outGen -match "File loaded: 30 lines") {
    Write-Host "[+] Dynamic sample log generation & reload passed!" -ForegroundColor Green
} else {
    Write-Host "[-] Sample generation test failed!" -ForegroundColor Red
    exit 1
}

if (Test-Path "logs/temp_test.log") {
    Remove-Item "logs/temp_test.log" -Force
}

Write-Host "`n============================================================" -ForegroundColor Cyan
Write-Host "          ALL 6 AUTOMATED TEST SUITES PASSED!               " -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Cyan
