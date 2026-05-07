#!/bin/bash
# ./scripts/find_shell_parent.sh bin/AppGui.exe
TARGET_EXE=$1

if [ -z "$TARGET_EXE" ]; then
    echo "Usage: ./find_shell_parent.sh <file.exe>"
    exit 1
fi

echo "--- Анализ зависимостей для: $TARGET_EXE ---"

# 1. Получаем список прямых DLL (убираем лишние пробелы и префиксы)
DIRECT_DLLS=$(objdump -p "$TARGET_EXE" | grep "DLL Name" | awk '{print $3}' | sort -u)

for dll in $DIRECT_DLLS; do
    # Поиск пути к DLL: сначала в локальной папке, потом в System32
    DLL_PATH=""
    if [ -f "./$dll" ]; then
        DLL_PATH="./$dll"
    elif [ -f "/c/Windows/System32/$dll" ]; then
        DLL_PATH="/c/Windows/System32/$dll"
    elif [ -f "/c/Windows/SysWOW64/$dll" ]; then
        DLL_PATH="/c/Windows/SysWOW64/$dll"
    fi

    if [ -n "$DLL_PATH" ]; then
        # Ищем упоминание shell32.dll внутри найденной библиотеки
        CHECK=$(objdump -p "$DLL_PATH" | grep -i "DLL Name: shell32.dll")
        
        if [ -n "$CHECK" ]; then
            echo "[!] НАЙДЕН РОДИТЕЛЬ: $dll ($DLL_PATH) -> тянет shell32.dll"
        else
            echo "[ ] Чисто: $dll"
        fi
    else
        echo "[?] Пропущено: $dll (путь не найден в стандартных папках)"
    fi
done

echo "--- Анализ завершен ---"