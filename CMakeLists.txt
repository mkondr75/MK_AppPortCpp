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
    src/app.rc
    src/wk_flags.cpp
    src/logsystem.cpp
    src/node_rt.cpp
    src/env_check.cpp
    src/init_config.cpp
    src/save_config.cpp
    src/load_config.cpp
    src/node_question.cpp
)

# 2. Создание EXE (WIN32 убирает консоль, аналог -mwindows)
add_executable(AppGui WIN32 ${SOURCES}
    src/node_question.cpp)

# 3. Настройки компиляции
target_compile_definitions(AppGui PRIVATE UNICODE _UNICODE)
#target_compile_options(AppGui PRIVATE -O3 -Wall)

# Если хочешь специфичные флаги для дебага (например, -Og для GCC)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(AppGui PRIVATE -Og -g)
else()
    target_compile_options(AppGui PRIVATE -O3)
endif()

# 4. Пути к заголовкам
target_include_directories(AppGui PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "C:/msys64/ucrt64/include"
)

# 5. Линковка библиотек
# Указываем полный путь к DLL, как в твоем батнике
target_link_libraries(AppGui PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/WebView2Loader.dll"
    comctl32
    dwmapi
    ole32
    uuid
    user32
    gdi32
    ws2_32
#    uxtheme
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
