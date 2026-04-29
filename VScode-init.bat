::# Постоянный лаунчер изолированного окружения
@echo off
setlocal

REM очистить всё кроме нужного
for /f "tokens=1 delims==" %%A in ('set') do (
    if /I not "%%A"=="OS" if /I not "%%A"=="SystemRoot" if /I not "%%A"=="ComSpec" (set "%%A="))
:: 1. Сохраняем только критически важные системные переменные
::for /f "tokens=1 delims==" %%A in ('set') do (
::    echo %%A | findstr /I "SystemRoot ComSpec OS" >nul
::    if errorlevel 1 set "%%A="
::)
::TEMP TMP USERPROFILE APPDATA LOCALAPPDATA PATHEXT c:\vscodeport;
set "PATH=c:\windows\system32;c:\windows;C:\Program Files\Git\cmd;c:\msys64\ucrt64\bin;C:\msys64\home\maxko\dev\dev_node\node_bin;C:\Users\maxko\AppData\Local\Programs\Microsoft VS Code\bin"
::set "CODE_EXE=c:\vscodeport\Code.exe"
set "CODE_EXE=C:\Users\maxko\AppData\Local\Programs\Microsoft VS Code\Code.exe"
::SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo %PATH%
:: Переходим в корень расположения скрипта.
cd /d "%~dp0"
:: Определение для сокращения пути
set "BASE=%~dp0"
echo "BASE=%BASE%"

::# Подготовка Windows-путей (критично для Code.exe)
::WIN_DIR=$(cygpath -w "$SCRIPT_DIR")
::WIN_DATA=$(cygpath -w "$SCRIPT_DIR/.vscode/user-data")
::WIN_EXT=$(cygpath -w "$SCRIPT_DIR/.vscode/extensions")
set "WIN_DIR=%BASE%"
set "WIN_DATA=%BASE%.vscode\user-data"
set "WIN_EXT=%BASE%.vscode\extensions"
::echo "WIN_DATA=%WIN_DATA%"
::echo "WIN_EXT=%WIN_EXT%"

set "USERPROFILE=%BASE%.vscode\profile"
set "APPDATA=%USERPROFILE%\AppData\Roaming"
set "LOCALAPPDATA=%USERPROFILE%\AppData\Local"
set "TEMP=%USERPROFILE%\tmp"
set "TMP=%TEMP%"
set "USERPROFILE=%BASE%"
set "SystemDrive=C:"

set "USERDOMAIN=MAX_NOTE"
set "USERDOMAIN_ROAMINGPROFILE=MAX_NOTE"
set "USERNAME=maxko"
set "USERPROFILE=C:\Users\maxko"
set "windir=C:\WINDOWS"
set "ZES_ENABLE_SYSMAN=1"
::echo "USERPROFILE=%USERPROFILE%"
::echo "APPDATA=%APPDATA%"
::echo "LOCALAPPDATA=%LOCALAPPDATA%"
::echo "TEMP=%TEMP%"
::echo "TMP=%TEMP%"
:: Создаем структуру профиля, иначе VS Code упадет в дефолт
if not exist "%USERPROFILE%" mkdir "%USERPROFILE%"
if not exist "%TEMP%" mkdir "%TEMP%"
if not exist "%APPDATA%" mkdir "%APPDATA%"
if not exist "%LOCALAPPDATA%" mkdir "%LOCALAPPDATA%"

::# Создаем папки, если их нет (mkdir -p не вредит существующим)
::mkdir -p "$SCRIPT_DIR/.vscode/user-data"
::mkdir -p "$SCRIPT_DIR/.vscode/extensions"

::export VSCODE_PORTABLE="$WIN_DATA"
::export MSYSTEM=UCRT64
::export CHERE_INVOKING=1

echo "--- VS Code Launcher ---"
set
::# Запуск: логи в файл (на случай если захочешь почитать), процесс в фон
::"$CODE_EXE" --user-data-dir "$WIN_DATA" --extensions-dir "$WIN_EXT" --new-window "C:\\msys64\\home\\maxko\\dev\\AI_DJS" > .vscode/last-run-log.txt 2>&1 &

::start /b "" "%CODE_EXE%" --user-data-dir "%WIN_DATA%" --extensions-dir "%WIN_EXT%" --new-window "%BASE%" > .vscode\last-run-log.txt 2>&1
if "%BASE:~-1%"=="\" set "BASE=%BASE:~0,-1%"
::echo "BASE=%BASE%"
@echo on
start /b "" "%CODE_EXE%" --user-data-dir "%WIN_DATA%" --extensions-dir "%WIN_EXT%" --new-window "%BASE%" > .vscode\last-run-log.txt 2>&1
::"%CODE_EXE%" --user-data-dir "%WIN_DATA%" --extensions-dir "%WIN_EXT%" --new-window "%BASE%" > .vscode\last-run-log.txt 2>&1
endlocal

::disown
::echo "VS Code запущен. Консоль свободна для команд."
