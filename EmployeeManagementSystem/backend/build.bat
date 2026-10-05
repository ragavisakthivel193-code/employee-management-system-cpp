@echo off
setlocal
echo ==========================================================
echo   Building Employee Management System Backend (C++)
echo ==========================================================

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo [ERROR] Failed to locate MSVC x64 build environment.
    exit /b 1
)

set MYSQL_INC="C:\Program Files\MySQL\MySQL Server 8.0\include"
set MYSQL_LIB="C:\Program Files\MySQL\MySQL Server 8.0\lib"

echo [1/3] Compiling C++ Backend with MSVC...
cl /nologo /EHsc /std:c++17 /O2 /Iinclude /I%MYSQL_INC% ^
    src\main.cpp src\Employee.cpp src\Database.cpp src\Auth.cpp src\EmployeeService.cpp ^
    /link /LIBPATH:%MYSQL_LIB% libmysql.lib ws2_32.lib crypt32.lib ^
    /out:server.exe

if errorlevel 1 (
    echo [ERROR] Build failed! Check compiler errors above.
    exit /b 1
)

echo [2/3] Copying libmysql.dll runtime dependency...
copy /Y "C:\Program Files\MySQL\MySQL Server 8.0\lib\libmysql.dll" .\libmysql.dll >nul

echo [3/3] Build succeeded: server.exe is ready!
echo ==========================================================
echo Run with: run.bat or .\server.exe
echo ==========================================================
