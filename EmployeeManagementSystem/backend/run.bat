@echo off
setlocal
echo Starting Employee Management System Backend...
if not exist server.exe (
    echo server.exe not found! Running build.bat first...
    call build.bat
    if errorlevel 1 exit /b 1
)
server.exe
