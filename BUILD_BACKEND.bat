@echo off
title Build Employee Management System
echo ==========================================================
echo       Building C++ Backend with MSVC x64
echo ==========================================================
cd /d "%~dp0EmployeeManagementSystem\backend"
call build.bat
pause
