$mapPath = "C:\msys64\home\maxko\dev\AppPortCpp\bin\AppGui.map"
$header = "             VMA              LMA     Size Align Out     In      Symbol"

# Читаем все строки из файла
$lines = Get-Content $mapPath

# Готовим новый массив строк
$newContent = New-Object System.Collections.Generic.List[string]
$newContent.Add($header)

foreach ($line in $lines) {
    # Пропускаем пустые строки или саму строку заголовка, если она уже там есть
    if ([string]::IsNullOrWhiteSpace($line) -or $line.Contains("VMA              LMA")) { continue }

    # Если строка начинается с адреса (8-16 шестнадцатеричных символов)
    if ($line -match "^([0-9a-fA-F]{8,16})\s+(.*)") {
        # Дублируем адрес (VMA == LMA)
        $addr = $matches[1]
        $rest = $matches[2]
        $newContent.Add("$addr $addr $rest")
    } else {
        # Если это просто текст (имена функций и т.д.), добавляем как есть
        $newContent.Add($line)
    }
}

# Сохраняем обратно в UTF8 без BOM (важно для многих парсеров)
[System.IO.File]::WriteAllLines($mapPath, $newContent)