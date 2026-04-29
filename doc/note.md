Подключаю смаке
C:\msys64\home\maxko\dev\AppPortCpp\CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Явно указываем пути к компиляторам, если они не в PATH (опционально, но надежно)
# set(CMAKE_C_COMPILER "C:/msys64/ucrt64/bin/gcc.exe")
# set(CMAKE_CXX_COMPILER "C:/msys64/ucrt64/bin/g++.exe")

project(AppGui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Исходники
set(SOURCES
    src/main.cpp
    src/wk_flags.cpp
    src/logsystem.cpp
    src/node_rt.cpp
)

# 2. Создание EXE (WIN32 убирает консоль, аналог -mwindows)
add_executable(AppGui WIN32 ${SOURCES})

# 3. Настройки компиляции
target_compile_definitions(AppGui PRIVATE UNICODE _UNICODE)
target_compile_options(AppGui PRIVATE -O3 -Wall)

# 4. Пути к заголовкам
target_include_directories(AppGui PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "C:/msys64/ucrt64/include"
)

# 5. Линковка библиотек
# Указываем полный путь к DLL, как в твоем батнике
target_link_libraries(AppGui PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/WebView2Loader.dll"
    ole32
    uuid
    user32
    gdi32
    ws2_32
)

# 6. Копирование WebView2Loader.dll в папку с .exe после сборки
add_custom_command(TARGET AppGui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/WebView2Loader.dll"
    "$<TARGET_FILE_DIR:AppGui>/"
    COMMENT "Copying WebView2Loader.dll to binary directory"
)

# 7. Вывод результата в папку bin в корне (как в батнике)
set_target_properties(AppGui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
Подготовка.
C:\msys64\home\maxko\dev\AppPortCpp>cmake -G Ninja -B build
Сборка.
C:\msys64\home\maxko\dev\AppPortCpp>cmake --build build
подключение всего этого к вскоде - создано:
1.C:\msys64\home\maxko\dev\AppPortCpp\CMakePresets.json
2.C:\msys64\home\maxko\dev\AppPortCpp\.vscode\launch.json
3.В Закладке смаке указано:
  Проджект статус:
    Фолдер(корень проэкта), Конфиг(выбор релиз\дебуг),Билд(дефолт\аппгуи)...,Дебуг(аппгуи),Лаунч(аппгуи)
  Проджект оутлине:
    напротив >аппгуи - конфиг(?), билд, клин.
4.В нижней(статусной)строке кнопки: 
  билд, дебуг,лаунч.
смена релиз\дебуг клин, билд затем ран. Без клин версия ехе будет от предыдущего конфига.


debug:

Thread 1 hit Breakpoint 1, main (argc=1, argv=0x74c620, envp=0x758430) at D:/W/B/src/mingw-w64/mingw-w64-crt/crt/crtexewin.c:19
Loaded 'C:\WINDOWS\SYSTEM32\ntdll.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\kernel32.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\KernelBase.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\comdlg32.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\msvcp_win.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\ucrtbase.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\combase.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\rpcrt4.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\SHCore.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\user32.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\win32u.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\gdi32.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\gdi32full.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\shlwapi.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\windows.storage.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\advapi32.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\msvcrt.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\sechost.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\shell32.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\WinTypes.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\WinSxS\amd64_microsoft.windows.common-controls_6595b64144ccf1df_5.82.26100.8115_none_87ecef177a2e8a19\comctl32.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\ole32.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\ws2_32.dll'. Symbols loaded.
Loaded 'C:\msys64\home\maxko\dev\AppPortCpp\bin\libgcc_s_seh-1.dll'. Symbols loaded.
Loaded 'C:\msys64\home\maxko\dev\AppPortCpp\bin\libstdc++-6.dll'. Symbols loaded.
Loaded 'C:\msys64\home\maxko\dev\AppPortCpp\bin\libwinpthread-1.dll'. Symbols loaded.
Loaded 'C:\msys64\home\maxko\dev\AppPortCpp\bin\WebView2Loader.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\imm32.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\SYSTEM32\cryptbase.dll'. Symbols loaded.
Loaded 'C:\WINDOWS\System32\bcryptprimitives.dll'. Symbols loaded.
PS C:\msys64\home\maxko\dev\AppPortCpp> .\memcalc_tril.ps1

Launcher ProcessCount TotalRAM_MB
-------- ------------ -----------
AppGui              1       15,98
Судя по списку загруженных библиотек, у вас классический «винегрет» из современного окружения (UCRT64/Win11) и legacy-вызовов, которые тянутся за старыми функциями. Поскольку вы используете **UCRT64**, ваша основная цель — минимизировать использование старых зависимостей MSVC.

Вот разбор по блокам, зависимостям и «лишним» элементам.

---

## 1. Блок: Ядро системы (The Core)
Это фундамент, без них процесс не запустится.
*   **ntdll.dll**: Самый низкий уровень (NT API).
*   **kernel32.dll / KernelBase.dll**: Основной Win32 API.
*   **advapi32.dll / sechost.dll**: Работа с реестром и безопасностью.
*   **ucrtbase.dll**: Ваша основная библиотека C-Runtime (раз вы в UCRT64).

## 2. Блок: Графика и UI (The Face)
Здесь больше всего дублирования из-за эволюции Windows.
*   **user32.dll / win32u.dll**: Окно, мышь, клавиатура. `win32u` — это современная «прослойка» для системных вызовов в Win11.
*   **gdi32.dll / gdi32full.dll**: Рисование. В Win11 `gdi32` — это просто пустая оболочка (враппер), которая перенаправляет вызовы в `gdi32full.dll`.
*   **comctl32.dll (v5.82)**: Старые элементы управления (кнопки, прогресс-бары).
*   **comdlg32.dll**: Стандартные диалоги (открыть файл, выбор цвета).
*   **imm32.dll**: Ввод иероглифов/спецсимволов (IME).

## 3. Блок: C++ и GCC Runtime
Это специфично для вашего компилятора MinGW-w64.
*   **libstdc++-6.dll**: Стандартная библиотека C++.
*   **libgcc_s_seh-1.dll**: Обработка исключений.
*   **libwinpthread-1.dll**: Потоки (POSIX threads для Windows).

---

## Кто здесь «лишний» и конкуренты

### 1. Главный конфликт: `msvcrt.dll` против `ucrtbase.dll`
*   **Проблема:** Вы собираете под **UCRT64**, но в списке есть `msvcrt.dll`.
*   **Почему она там:** Скорее всего, она подтянулась как зависимость от какой-то старой библиотеки или системного компонента.
*   **Вердикт:** Если вы явно линкуетесь с `-lmsvcrt`, уберите это. В UCRT64 вы должны использовать только `ucrtbase.dll`. `msvcrt.dll` — это артефакт времен Windows 95/XP.

### 2. Устаревший менеджер путей: `shlwapi.dll`
*   **Конкурент:** `SHCore.dll` и `windows.storage.dll`.
*   **Вердикт:** `shlwapi.dll` (Shell Light-Weight API) — это технологии эпохи Internet Explorer 4. Большинство функций работы со строками и путями оттуда сегодня лучше заменять на современные аналоги из `SHCore` или стандартную библиотеку C++17 `<filesystem>`.

### 3. Дублирование GDI: `gdi32.dll` vs `gdi32full.dll`
*   **Вердикт:** Вы не можете избавиться от них вручную (система сама их грузит), но знайте, что `gdi32.dll` в Windows 11 больше ничего не делает сам.

### 4. Веб-компоненты: `WebView2Loader.dll`
*   **Вердикт:** Если ваше приложение не отображает веб-страницы через движок Edge (Chromium), эта библиотека вам не нужна. Она довольно тяжелая по цепочке зависимостей.

---

## Цепочки зависимостей (Кто кого тянет)



1.  **WebView2Loader.dll** → тянет за собой половину списка (криптографию `bcrypt`, сеть `ws2_32`, хранилища `windows.storage`).
2.  **comdlg32.dll** → тянет **shlwapi.dll** и **shell32.dll**. Если вы используете свои кастомные диалоги выбора файлов, этот блок можно «отрезать».
3.  **libstdc++-6.dll** → тянет **libgcc_s_seh-1.dll** и **libwinpthread-1.dll**.

---

## Рекомендации по оптимизации

1.  **Проверьте WebView2:** Если вы просто рисуете формочки на Win32 API и вам не нужен браузер внутри — удаляйте `WebView2Loader.dll`. Это сэкономит кучу памяти.
2.  **Статическая линковка Runtime:** Если вы хотите избавиться от `libstdc++-6.dll` и `libwinpthread-1.dll` в папке с программой, используйте флаги при сборке:
    ```bash
    -static -static-libgcc -static-libstdc++
    ```
    Это увеличит размер `.exe`, но уменьшит количество загружаемых DLL и ускорит запуск.
3.  **Очистка от Shell-хлама:** Проверьте, используете ли вы функции, начинающиеся на `Path...` (например, `PathFileExists`). Если да — они тянут `shlwapi.dll`. Замените их на стандартные средства C++ или `GetFileAttributes`.
4.  **Удаление `imm32.dll`:** Если вы не планируете поддерживать китайский/японский ввод, можно попробовать не линковаться с ней, но часто её подтягивает сама `user32.dll`.

**Итог:** Самый эффективный способ «похудеть» — это отказаться от `WebView2` (если он не критичен) и перейти на статическую линковку библиотек GCC. 16 МБ для Win32 приложения — это немного, но для простого калькулятора или утилиты многовато; львиную долю здесь съедает инициализация C++ Runtime и Shell-компонентов.

Есть ли в коде специфические функции, ради которых вы подключали `WebView2` или `comdlg32`?
```