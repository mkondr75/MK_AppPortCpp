# Команда выполнится автоматически ПОСЛЕ сборки AppGui.exe
#add_custom_command(TARGET AppGui POST_BUILD
#    COMMAND powershell -ExecutionPolicy Bypass -Command "
#        $path = '$<TARGET_FILE_DIR:AppGui>/AppGui.map';
        $path = "C:\msys64\home\maxko\dev\AppPortCpp\bin\AppGui.map";
        if (!(Test-Path $path)) { return };
        
        $raw = Get-Content $path;
        $out = New-Object System.Collections.Generic.List[string];
        
        # Добавляем тот самый магический заголовок
        $out.Add('             VMA              LMA     Size Align Out     In      Symbol');
        
        foreach ($line in $raw) {
            # Ищем строки, начинающиеся с 8-значного HEX адреса (формат LLD Windows)
            if ($line -match '^([0-9a-fA-F]{8,16})\s+([0-9a-fA-F]{1,16})\s+(.*)') {
                $addr  = $matches[1]; # Адрес (станет VMA и LMA)
                $size  = $matches[2]; # Размер
                $rest  = $matches[3]; # Остаток (Align, Out, In...)
                
                # Собираем строку: VMA (addr) + LMA (addr) + Size + Остальное
                $out.Add("$addr $addr $size $rest");
            }
            # Пропускаем старый заголовок LLD, чтобы не дублировался
            elseif ($line -match '^Address\s+Size') { continue }
            else {
                # Сохраняем пустые строки или комментарии как есть
                if (![string]::IsNullOrWhiteSpace($line)) { $out.Add($line) }
            }
        }
        [System.IO.File]::WriteAllLines($path, $out);
#    "
#    COMMENT "Converting LLD Map to Extension-friendly format..."
#)