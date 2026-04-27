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