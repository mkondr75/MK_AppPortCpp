@echo off


:: 2. Очистка, но ОСТАВЛЯЕМ критические системные константы
:: Используем полный путь к findstr, чтобы не зависеть от PATH
set "FIND_EXE=%SystemRoot%\System32\findstr.exe"
for /f "tokens=1 delims==" %%A in ('set') do (
    echo %%A | "%FIND_EXE%" /I "SystemDrive SystemRoot ComSpec PATHEXT OS USERNAME" >nul
    if errorlevel 1 set "%%A="
)
:: 1. Сначала определяем базу, пока окружение чистое
set "BASE=%~dp0"
if "%BASE:~-1%"=="\" set "BASE=%BASE:~0,-1%"

:: 3. Настройка путей (БЕЗ кавычек внутри значений)
set "PATH=C:\msys64\home\maxko\dev\dev_node\node_bin;c:\vscodeport;c:\msys64\ucrt64\bin;%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\System32\Wbem"
set "CODE_EXE=c:\vscodeport\Code.exe"
set "WIN_DATA=%BASE%\.vscode\user-data"
set "WIN_EXT=%BASE%\.vscode\extensions"

:: 4. Изоляция профиля (внутри проекта)
set "USERPROFILE=%BASE%\.vscode\profile"
set "APPDATA=%USERPROFILE%\AppData\Roaming"
set "LOCALAPPDATA=%USERPROFILE%\AppData\Local"
set "TEMP=%USERPROFILE%\tmp"
set "TMP=%USERPROFILE%\tmp"

:: 5. Физическое создание дерева папок (Критично для терминала!)
if not exist "%TEMP%" mkdir "%TEMP%"
if not exist "%APPDATA%" mkdir "%APPDATA%"
if not exist "%LOCALAPPDATA%" mkdir "%LOCALAPPDATA%"

echo --- VS Code Launcher ---
echo Project: %BASE%

@echo on
:: 6. Запуск (используем кавычки только при вызове)
"%CODE_EXE%" --user-data-dir "%WIN_DATA%" --extensions-dir "%WIN_EXT%" --new-window "%BASE%" > .vscode\last-run-log.txt 2>&1