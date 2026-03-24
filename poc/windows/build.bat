@echo off
REM build.bat — Compile the Invisible Window Windows PoC
REM Author: Mohammad Raouf Abedini
REM
REM Option 1: Visual Studio (cl.exe)
REM   Open "Developer Command Prompt for VS" and run this script
REM
REM Option 2: MinGW/GCC
REM   gcc invisible_window.c -o invisible_window.exe -lgdi32 -luser32 -mwindows

echo ===================================================
echo   Building Invisible Window — Windows PoC
echo ===================================================
echo.

where cl >nul 2>nul
if %ERRORLEVEL%==0 (
    echo [BUILD] Using MSVC (cl.exe)
    cl invisible_window.c /Fe:invisible_window.exe /link user32.lib gdi32.lib
) else (
    where gcc >nul 2>nul
    if %ERRORLEVEL%==0 (
        echo [BUILD] Using GCC (MinGW)
        gcc invisible_window.c -o invisible_window.exe -lgdi32 -luser32 -mwindows
    ) else (
        echo [ERROR] Neither cl.exe nor gcc found.
        echo [ERROR] Install Visual Studio Build Tools or MinGW.
        exit /b 1
    )
)

if exist invisible_window.exe (
    echo.
    echo [BUILD] Success! Binary: invisible_window.exe
    echo.
    echo Run with:
    echo   invisible_window.exe
    echo.
    echo Requirements: Windows 10 Version 2004 or later
) else (
    echo.
    echo [ERROR] Build failed.
)
