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

Раз вы подтвердили, что **WebView2Loader.dll** необходим, это объясняет 80% вашего списка загруженных библиотек. WebView2 — это не просто «браузер в окне», это огромный программный стек, который тянет за собой зависимости из разных эпох Windows для обеспечения совместимости.

Вот детальный разбор вашего «винегрета» из `target_link_libraries`:

---

### 1. Группировка и анализ зависимостей

Я разделил библиотеки по их роли в системе.

| Блок | Библиотеки | Зачем это нужно? |
| :--- | :--- | :--- |
| **Основа WebView2** | `WebView2Loader.dll`, `ole32`, `uuid` | **WebView2** построен на технологии **COM (Component Object Model)**. `ole32` управляет объектами, а `uuid` содержит идентификаторы интерфейсов. |
| **Системный UI** | `user32`, `gdi32` | Создание самого окна приложения, в котором «живет» WebView2. |
| **Сеть** | `ws2_32` (Winsock) | Работа с сокетами. WebView2 сам управляет трафиком, но часто требует инициализации сетевого стека. |
| **Runtime (GCC)** | `libstdc++`, `libgcc_s`, `libwinpthread` | Специфичный для MinGW «обвес» для поддержки стандартов C++. |

---

### 2. Конфликтующий и дублирующий функционал

В вашем логе видно «столкновение» технологий разных лет. Windows 11 вынуждена грузить обе версии, чтобы всё работало.

#### **Эпоха Legacy (Старое) vs Эпоха Modern (Новое)**

1.  **`msvcrt.dll` vs `ucrtbase.dll` (ГЛАВНЫЙ КОНФЛИКТ)**
    *   **Суть:** `msvcrt.dll` — это старый рантайм (до 2015 года). `ucrtbase.dll` — современный Universal C Runtime.
    *   **Почему оба здесь:** Ваша сборка UCRT64 использует новый рантайм, но `WebView2Loader` или системные DLL (типа `shell32`) до сих пор завязаны на старый `msvcrt`.
    *   **Можно ли убрать:** Нет. Вы не можете заставить системные библиотеки Microsoft перестать использовать их собственные зависимости.

2.  **`gdi32.dll` vs `gdi32full.dll`**
    *   **Суть:** В Win11 `gdi32.dll` — это просто «заглушка» для совместимости. Реальный код переехал в `gdi32full.dll`.
    *   **Можно ли убрать:** Из `target_link_libraries` — **да**, если вы не вызываете функции типа `BitBlt` или `CreateFont` напрямую. WebView2 сам разберется с рисованием.

3.  **`shlwapi.dll` vs `SHCore.dll`**
    *   **Суть:** `shlwapi` — старые функции путей и строк. `SHCore` — современные API для работы с мониторами (DPI) и памятью.
    *   **Вердикт:** Они дублируют друг друга в части обработки строк, но WebView2 требует оба.

---

### 3. Цепочки зависимостей (Кто кого «привел»)

Если вы удалите верхнее звено, нижние исчезнут из памяти:

1.  **WebView2Loader.dll** (Главный «виновник»)
    *   → `ole32.dll` (COM-объекты)
    *   → `shcore.dll` (DPI Awareness и работа с памятью)
    *   → `comctl32.dll` (Визуальные стили кнопок)
    *   → `ws2_32.dll` (Сетевые протоколы)

2.  **user32.dll**
    *   → `gdi32.dll` (Низкоуровневое рисование)
    *   → `imm32.dll` (Поддержка ввода)

3.  **libstdc++-6.dll**
    *   → `libwinpthread-1.dll` (Потоки)
    *   → `libgcc_s_seh-1.dll` (Обработка исключений)

---

### 4. От кого можно избавиться?

В вашем `CMakeLists.txt` и процессе сборки можно провести «хирургию»:

1.  **`uuid`**: Эту библиотеку можно не линковать явно как DLL, она обычно вшивается статически.
2.  **`gdi32`**: Попробуйте убрать её из `target_link_libraries`. Если проект скомпилируется, значит, вы не используете её функции напрямую, а WebView2 подтянет её сам, когда нужно.
3.  **`ws2_32`**: Если вы не пишете свой сетевой код (например, на `boost::asio` или сырых сокетах), скорее всего, она вам в списке линковки не нужна — WebView2 сам её найдет.
4.  **Статическая линковка GCC**:
    Чтобы избавиться от `libstdc++-6.dll`, `libgcc_s_seh-1.dll` и `libwinpthread-1.dll` в папке с программой (и уменьшить количество «Loaded...» в логе), добавьте эти флаги:
    ```cmake
    target_link_options(AppGui PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive)
    ```
    Это "вмонтирует" их внутрь вашего `.exe`.

### Резюме по вашему списку:
Лишним (в плане явной линковки) выглядят **`gdi32`** и **`ws2_32`**. WebView2 слишком самодостаточен и сам обеспечит их наличие.

Ваше потребление в **15.98 MB RAM** для Win32 + WebView2 — это **отличный результат**. Для сравнения: пустое приложение на Electron (тот же Chromium) сразу съедает около 80-100 MB. Вы идете по очень экономному пути.

Нужно ли помочь с флагами для полной статической сборки, чтобы в папке остался только `.exe` и `WebView2Loader.dll`?

1. Магия разделяемой памяти (Shared Pages)
Вы правы, подозревая, что эти библиотеки уже есть в памяти. Системные DLL (ntdll.dll, kernel32.dll, user32.dll и т.д.) загружаются ядром Windows один раз в физическую память.

Как это работает: Когда ваше приложение запускается, Windows просто «проецирует» (mapping) те же самые страницы памяти, где уже лежат эти DLL, в адресное пространство вашего процесса.

Итог: Код (инструкции) этих библиотек общий для всех программ. Если 100 программ используют user32.dll, в оперативной памяти она лежит в единственном экземпляре.

За что вы платите: Вы платите только за секции данных (переменные, специфичные для вашего окна), которые помечаются как Copy-on-Write (копирование при записи). Это сущие копейки (килобайты).

2. Все ли DLL «ваши»?
В списке есть три типа библиотек:

Системный стандарт (Hardwired): ntdll, kernel32. Без них процесс не существует. Они всегда в RAM.

Инфраструктурные (Shared): comctl32, shell32, ole32. Они почти наверняка уже загружены Проводником (Explorer) или другими программами. Юзая их, вы не нагружаете систему.

Ваш «багаж»: libstdc++-6.dll, libwinpthread-1.dll, WebView2Loader.dll.

Вот они — уникальные для вашего процесса (или других программ на GCC).

Если в системе больше никто не запустил софт на MinGW, то эти DLL грузятся в память только ради вас.

3. Можно ли «доиспользовать» то, что уже есть?
Раз вы не хотите статику, ваш путь — Dynamic Loading (Runtime Linking).

Вместо того чтобы указывать библиотеки в target_link_libraries, вы можете вызывать их через LoadLibrary() и GetProcAddress().

Плюс: DLL не будет висеть в списке загруженных, пока вы её явно не позовете. Если условие в коде не сработало — DLL не заняла даже виртуальный адрес.

Минус: Это адски неудобно писать (нужно вручную объявлять сигнатуры всех функций).

Что в вашем списке действительно «лишнее»?
Если смотреть на ваш результат в 15.98 MB, то основную часть памяти съедают не DLL, а Heap (куча) и Stack (стек) вашего процесса, а также внутренние структуры WebView2.

Цепочки «паразитов»:

imm32.dll: Грузится почти всегда через user32. Если у вас нет полей ввода текста, она просто висит мертвым грузом. Но выкинуть её нельзя — она вшита в логику оконной подсистемы.

uxtheme.dll (часто лезет через comctl32): Рисует красивые кнопочки Win11. Хотите сэкономить 100 КБ? Придется рисовать кнопки самому через сырой GDI (что ужасно).

Вердикт: как достичь идеала без статики?
Если цель — минимальный «отпечаток» в системе:

Отложенная загрузка (Delay Load): В MSVC есть флаг /DELAYLOAD, в GCC/MinGW это имитируется через -Wl,--delayload. DLL загрузится только в момент первого вызова функции из неё. Если вы редко вызываете диалоги comdlg32, она не будет «греть» память при старте.

Замена libstdc++: Если вы перепишете код на чистый C (без классов, векторов и исключений), вы избавитесь от libstdc++-6.dll и libwinpthread-1.dll. Это даст самый заметный профит, так как эти библиотеки тянут за собой много инициализационного кода.

WebView2 — это «черная дыра»: Поймите, что WebView2Loader.dll — это только верхушка айсберга. Как только вы вызываете CreateCoreWebView2Environment, система порождает еще 3-5 процессов msedgewebview2.exe. Ваше основное потребление будет там, а не в списке DLL.

12) Что меня сейчас настораживает

Вот это:

api-ms-win-crt-filesystem-l1-1-0.dll

Это почти наверняка:

std::filesystem

А значит:

C++ runtime уже активно участвует,
проект не совсем «Си-лаунчер».

Не проблема, но это главный потенциальный источник разрастания зависимостей в будущем.

maxko@MAX_NOTE UCRT64 ~/dev/AppPortCpp/bin
$ objdump -p AppGui.exe | grep "DLL Name"
        DLL Name: comdlg32.dll
        DLL Name: libgcc_s_seh-1.dll
        DLL Name: GDI32.dll
        DLL Name: KERNEL32.dll
        DLL Name: api-ms-win-crt-convert-l1-1-0.dll
        DLL Name: api-ms-win-crt-environment-l1-1-0.dll
        DLL Name: api-ms-win-crt-filesystem-l1-1-0.dll
        DLL Name: api-ms-win-crt-heap-l1-1-0.dll
        DLL Name: api-ms-win-crt-locale-l1-1-0.dll
        DLL Name: api-ms-win-crt-math-l1-1-0.dll
        DLL Name: api-ms-win-crt-private-l1-1-0.dll
        DLL Name: api-ms-win-crt-runtime-l1-1-0.dll
        DLL Name: api-ms-win-crt-stdio-l1-1-0.dll
        DLL Name: api-ms-win-crt-string-l1-1-0.dll
        DLL Name: ole32.dll
        DLL Name: SHELL32.dll
        DLL Name: libstdc++-6.dll
        DLL Name: USER32.dll
        DLL Name: WS2_32.dll
        DLL Name: WebView2Loader.dll


5) Очень полезная дисциплина
Запрещённые заголовки
Сделай себе внутренний blacklist:
shlwapi.h
shellapi.h
commdlg.h
dwmapi.h
uxtheme.h
wininet.h
urlmon.h
Подключать только после осознанного решения.

maxko@MAX_NOTE UCRT64 ~/dev/AppPortCpp/bin
objdump -p AppGui.exe > ../doc/imports20260429.txt
diff -u ../doc/imports.txt ../doc/imports20260429.txt - не четаемо
colordiff -u ../doc/imports.txt ../doc/imports20260429.txt - bash: colordiff: command not found
в вскоде шифтом выделить два файла и правя мышь - компаре селектед
????
git diff ../doc/imports.txt - сравнит историю гита.

grep -RhoP '^#include\s*<\K[^>]+' ./src ./include | sort -u

maxko@MAX_NOTE UCRT64 ~/dev/AppPortCpp
$ grep -RhoP '^#include\s*<\K[^>]+' ./src ./include | sort -u
commdlg.h
cstdio
cwchar
filesystem
fstream
guiddef.h
iostream
psapi.h
shellapi.h
sstream
stdio.h
string
tlhelp32.h
vector
WebView2.h
windows.h
winsock2.h
wrl/client.h
ws2tcpip.h

grep -RhoP '^#include\s*[<"]\K[^>"]+' ./src ./include \
| sort \
| uniq -c \
| sort -nr > ./doc/include_audit.txt

grep -RhoP '^#include\s*[<]\K[^>]+' ./src ./include \
| sort \
| uniq -c \
| sort -nr > ./doc/include_audit.txt

maxko@MAX_NOTE UCRT64 ~/dev/AppPortCpp
grep -RhoP '^#include\s*[<]\K[^>]+' ./src ./include \
| sort \
| uniq -c > ./doc/include_audit.txt

for f in ./src/*.cpp; do
    echo "=== $f ==="
    g++ -MM "$f"
done

С подключеным флагом смаке     dwmapi
40-60кб рама за чёрный верх окна

альтернатива двмапи без флагов в смаке
// Определяем сигнатуру функции
typedef HRESULT(WINAPI* PFN_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);

// ... внутри WndProc ...

HWND hwnd = CreateWindowExW( /* ... */ );

if (!hwnd)
    return false;

// --- ПОЗДНЕЕ СВЯЗЫВАНИЕ DWMAPI ---
HMODULE hDwmApi = LoadLibraryW(L"dwmapi.dll");
if (hDwmApi) {
    PFN_DwmSetWindowAttribute pDwmSetWindowAttribute = 
        (PFN_DwmSetWindowAttribute)GetProcAddress(hDwmApi, "DwmSetWindowAttribute");
        
    if (pDwmSetWindowAttribute) {
        BOOL useDarkMode = TRUE;
        pDwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
    }
    FreeLibrary(hDwmApi);
}
// ---------------------------------
ShowWindow(hwnd, SW_SHOW);


Попытка анализа стринг
maxko@MAX_NOTE UCRT64 ~/dev/AppPortCpp
$ grep -rE "std::[a-z_0-9]+\s*\(|\.[a-z_0-9]+\s*\(" src/ include/ --include=*.{cpp,hpp,h} \
| grep -vE "include|if\s*\(|while\s*\(|for\s*\("
src/env_check.cpp:        str.c_str(),
src/env_check.cpp:        (int)str.size(),
src/env_check.cpp:        str.c_str(),
src/env_check.cpp:        (int)str.size(),
src/env_check.cpp://            (woutput.back() == L'\n' ||
src/env_check.cpp://             woutput.back() == L'\r'))
src/env_check.cpp://         woutput.pop_back();
src/env_check.cpp:           (woutput.back() == L'\n' ||
src/env_check.cpp:            woutput.back() == L'\r'))
src/env_check.cpp:        woutput.pop_back();
src/env_check.cpp:        path.c_str(),
src/env_check.cpp:        DWORD attrs = GetFileAttributesW(full_path.c_str());
src/env_check.cpp:        out_result.push_back(item);
src/env_check.cpp:            std::wstring(result.node.ok ? L"true" : L"false") +
src/env_check.cpp:            LogLauncherInfo(msg.c_str());
src/env_check.cpp:            LogLauncherError(msg.c_str());
src/env_check.cpp:            std::wstring(result.webview2.ok ? L"true" : L"false") +
src/env_check.cpp:            LogLauncherInfo(msg.c_str());
src/env_check.cpp:            LogLauncherError(msg.c_str());
src/env_check.cpp:            std::wstring(dll.exists ? L"true" : L"false");
src/env_check.cpp:            LogLauncherInfo(msg.c_str());
src/env_check.cpp:            LogLauncherError(msg.c_str());
src/load_config.cpp://     buffer << file.rdbuf();
src/load_config.cpp://     return buffer.str();
src/load_config.cpp://     buffer << file.rdbuf();
src/load_config.cpp://     return buffer.str();
src/load_config.cpp:    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
src/load_config.cpp:    text.resize((size_t)(size.QuadPart / sizeof(wchar_t)));
src/load_config.cpp:    BOOL ok = ReadFile(file, text.data(), (DWORD)size.QuadPart, &readed, NULL);
src/load_config.cpp:    text.resize(readed / sizeof(wchar_t));
src/load_config.cpp:    size_t key_pos = json.find(pattern);
src/load_config.cpp:    size_t colon_pos = json.find(L":", key_pos);
src/load_config.cpp:    size_t first_quote = json.find(L"\"", colon_pos + 1);
src/load_config.cpp:    size_t second_quote = json.find(L"\"", first_quote + 1);
src/load_config.cpp:    return json.substr(first_quote + 1, second_quote - first_quote - 1);
src/load_config.cpp:    size_t key_pos = json.find(pattern);
src/load_config.cpp:    size_t colon_pos = json.find(L":", key_pos);
src/load_config.cpp:    size_t true_pos = json.find(L"true", colon_pos);
src/logsystem.cpp:    return std::wstring(buffer);
src/logsystem.cpp:    int utf8_size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
src/logsystem.cpp:    utf8.resize(utf8_size - 1);
src/logsystem.cpp:    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, utf8.data(), utf8_size, nullptr, nullptr);
src/logsystem.cpp:    BOOL ok = WriteFile(h, utf8.c_str(), (DWORD)utf8.size(), &written, nullptr);
src/logsystem.cpp:             ts.c_str(),
src/logsystem.cpp:    wprintf(L"logfile_launcher=%ls\n", cfg.launcher_file.c_str());
src/logsystem.cpp:            cfg.launcher_file.c_str(),
src/logsystem.cpp:            wprintf(L"ErroCreating logfile_launcher=%ls\n", cfg.launcher_file.c_str());
src/logsystem.cpp:            wprintf(L"file was created logfile_launcher=%ls\n", cfg.launcher_file.c_str());
src/main.cpp:                   LogLauncherInfo(q.system_node_path.c_str());
src/main.cpp:                   LogLauncherInfo(q.custom_node_path.c_str());
src/main.cpp:   // wprintf(L"This writed to logfile=%ls\n", cfg.paths.root_dir.c_str());
src/node_question.cpp:                system_text.c_str(),
src/node_question.cpp:                g_model->custom_node_path.c_str(),
src/node_rt.cpp:            cfg.logs.node_file.c_str(),
src/node_rt.cpp:                    cfg.logs.node_file.c_str());
src/node_rt.cpp:    // wchar_t cmd[] = L"\"C:\\Program Files\\nodejs\\node.exe\" --permission --allow-fs-read=./* --allow-fs-write=./* --allow-fs-read=C:\\dev_node\\trilium_dev --allow-fs-write=C:\\dev_node\\trilium_dev --allow-child-process --allow-addons -e \"import { fileURLToPath } from 'url';console.log(fileURLToPath(import.meta.url)); console.log(process.env); console.log(process.execArgv);process.stdin.resume(); process.stdin.on('data', ()=>process.exit(0));\"";
src/save_config.cpp:        cfg.paths.config_file.c_str(),
src/save_config.cpp:        cfg.paths.root_dir.c_str(),
src/save_config.cpp:        cfg.paths.bin_dir.c_str(),
src/save_config.cpp:        cfg.paths.logs_dir.c_str(),
src/save_config.cpp:        cfg.paths.config_file.c_str(),
src/save_config.cpp:        cfg.paths.node.c_str(),
src/save_config.cpp:        cfg.node.exe.c_str(),
src/save_config.cpp:        cfg.node.working_dir.c_str(),
src/save_config.cpp:        cfg.node.entry_script.c_str(),
src/save_config.cpp:        cfg.node.args.c_str(),
src/save_config.cpp:        cfg.edge.user_data_dir.c_str());
src/save_config.cpp:    file << json.str();
src/save_config.cpp:    file.close();

maxko@MAX_NOTE UCRT64 ~/dev/AppPortCpp