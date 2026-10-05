@echo off
title Employee Management System
echo ==========================================================
echo       Starting Employee Management System (C++)
echo ==========================================================
cd /d "%~dp0EmployeeManagementSystem\backend"

if not exist server.exe (
    echo server.exe not found! Compiling backend first...
    call build.bat
    if errorlevel 1 (
        echo Compilation failed! Press any key to exit.
        pause
        exit /b 1
    )
)

echo Starting C++ Backend Server on http://localhost:8088 ...
start "" http://localhost:8088/login.html
server.exe
pause
