::g++ main.cpp -o Appgui.exe -DUNICODE -D_UNICODE -mwindows -Iinclude -Llib WebView2Loader.dll -lole32 -luuid -luser32 -lgdi32 -lws2_32

@echo off
setlocal EnableDelayedExpansion
::setlocal

REM очистить всё кроме нужного
for /f "tokens=1 delims==" %%A in ('set') do (
    if /I not "%%A"=="SystemRoot" if /I not "%%A"=="ComSpec" (
        set "%%A="
    )
)

set "PATH=C:\msys64\ucrt64\bin;c:\msys64\ucrt64\include;c:\msys64\ucrt64\lib;C:\WINDOWS;C:\Windows\System32"
echo %PATH%

set ROOT=%~dp0
set TEMP=%ROOT%temp
set TMP=%ROOT%temp

if not exist "%ROOT%temp" mkdir "%ROOT%temp"
REM === CONFIG ===
set APP=AppGui
set SRC=%ROOT%src
set INC=%ROOT%include
set LIB=%ROOT%lib
set OBJ=%ROOT%obj
set BIN=%ROOT%bin

REM === CREATE DIRS ===
if not exist "%OBJ%" mkdir "%OBJ%"
if not exist "%BIN%" mkdir "%BIN%"

REM === FLAGS ===
set CFLAGS=-std=c++20 -O3 -Wall -Winvalid-pch
set CFLAGS=%CFLAGS% -DUNICODE -D_UNICODE
::set CFLAGS=%CFLAGS% -I"%INC%"

REM GUI subsystem
set LDFLAGS=-mwindows

REM libs
set LIBS= lib\WebView2Loader.dll -lole32 -luuid -luser32 -lgdi32 -lws2_32

echo.
echo ===== BUILD =====
echo.
::C:\Windows\System32\where.exe libstdc++-6.dll
::g++ src\main.cpp src\wk_flags.cpp -o bin\Appgui.exe -DUNICODE -D_UNICODE -mwindows -Iinclude -Llib bin\WebView2Loader.dll -lole32 -luuid -luser32 -lgdi32 -lws2_32
REM compile
g++ -c src\main.cpp -o obj\main.o %CFLAGS%
g++ -c src\wk_flags.cpp -o obj\wk_flags.o %CFLAGS%
g++ -c src\logsystem.cpp -o obj\logsystem.o %CFLAGS%
g++ -c src\node_rt.cpp -o obj\node_rt.o %CFLAGS%
if not exist obj\*.o (
    echo no obj files
    exit /b 1
)
echo.
echo ===== LINKED =====
echo.

set OBJS=obj\main.o obj\wk_flags.o obj\logsystem.o obj\node_rt.o
set LFLAGS=-mwindows -Iinclude -Llib bin\WebView2Loader.dll -lole32 -luuid -luser32 -lgdi32 -lws2_32
REM link
g++ %OBJS% -o bin\AppGui.exe %LFLAGS%

if errorlevel 1 (
    echo.
    echo BUILD FAILED
    exit /b 1
)

echo.
echo BUILD OK
echo.

REM copy runtime dlls if needed
::copy /Y "%LIB%\*.dll" "%BIN%\" >nul 2>nul