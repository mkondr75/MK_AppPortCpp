cmake_minimum_required(VERSION 3.20)

# НЕ ПИШЕМ ТУТ set(CMAKE_CXX_COMPILER) — это работа пресетов!

project(AppGui LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
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
	src/mem_tool.cpp
)

# 2. Создание EXE
add_executable(AppGui WIN32 ${SOURCES})

# 3. Настройки компиляции
target_compile_definitions(AppGui PRIVATE UNICODE _UNICODE)
set_target_properties(AppGui PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)

# Разделяем флаги для Clang и GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(AppGui PRIVATE -Og -g -gcodeview)
    else()
       target_compile_options(AppGui PRIVATE -O3 -gcodeview) # сливает всю инфу в пдб файл а ехе стирильный
        # target_compile_options(AppGui PRIVATE -O3) # пдб порезаный ехе тотже байт в байт
        # Убирает все отладочные секции из финального бинарника
        target_link_options(AppGui PRIVATE "-Wl,-s")
		target_link_options(AppGui PRIVATE "-Wl,--gc-sections" "-static") 
    endif()
    # Специфично для Clang + LLD
    target_link_options(AppGui PRIVATE "-fuse-ld=lld" "-Wl,-pdb,${CMAKE_CURRENT_SOURCE_DIR}/bin/AppGui.pdb")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(AppGui PRIVATE -Og -g)
    else()
        target_compile_options(AppGui PRIVATE -O3)
    endif()
endif()

# Общие флаги оптимизации размера
target_compile_options(AppGui PRIVATE "-ffunction-sections" "-fdata-sections")
target_link_options(AppGui PRIVATE "-Wl,--gc-sections")

# 4. Пути к заголовкам (UCRT64)
target_include_directories(AppGui PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "C:/msys64/ucrt64/include"
)

# 5. Линковка библиотек
#target_link_options(AppGui PRIVATE "-nodefaultlibs") - faill
target_link_libraries(AppGui PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/WebView2Loader.dll"
     ole32 uuid user32 gdi32 ws2_32 
)
#    comctl32 dwmapi ole32 uuid user32 gdi32 ws2_32 

# 6. Вывод и Map-файл
set_target_properties(AppGui PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
target_link_options(AppGui PRIVATE "-Wl,--Map,${CMAKE_CURRENT_SOURCE_DIR}/bin/AppGui.map")

# 7. Копирование DLL
# add_custom_command(TARGET AppGui POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#     "${CMAKE_CURRENT_SOURCE_DIR}/lib/WebView2Loader.dll"
#     "$<TARGET_FILE_DIR:AppGui>/"
# )
# 8. СКРИПТ ПОДДЕЛКИ ФОРМАТА (POST_BUILD)
# Находим путь к PowerShell программно или задаем вручную
set(POWERSHELL_PATH "C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe")
add_custom_command(TARGET AppGui POST_BUILD
    COMMAND ${POWERSHELL_PATH} -ExecutionPolicy Bypass -File 
        "${CMAKE_CURRENT_SOURCE_DIR}/scripts/fix_map2.ps1" 
        -MapPath "${CMAKE_CURRENT_SOURCE_DIR}/bin/AppGui.map"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Converting LLD map file..."
    VERBATIM
)