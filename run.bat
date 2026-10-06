@echo off
if not exist intelliguard.exe (
    echo [INFO] intelliguard.exe not found. Building first...
    call build.bat
    if errorlevel 1 exit /b 1
)
intelliguard.exe
