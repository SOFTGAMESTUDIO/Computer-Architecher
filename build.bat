@echo off
echo ============================================================
echo   Building C-IntelliGuard (GCC / MinGW)
echo ============================================================
gcc -Wall -Wextra -std=c11 -Isrc src\*.c -o intelliguard.exe -lm
if %ERRORLEVEL% equ 0 (
    echo [BUILD SUCCESS] Binary ready: intelliguard.exe
) else (
    echo [BUILD FAILED] Compilation failed with exit code %ERRORLEVEL%
)
