$LauncherName = "NOCLOSE64" # Имя вашего .exe без расширения
$Parent = Get-Process $LauncherName -ErrorAction SilentlyContinue

if ($Parent) {
    # Получаем ID всех дочерних процессов рекурсивно
    function Get-ChildProcesses($ParentId) {
        $Children = Get-CimInstance Win32_Process -Filter "ParentProcessId=$ParentId"
        foreach ($Child in $Children) {
            $Child.ProcessId
            Get-ChildProcesses $Child.ProcessId
        }
    }

    $Ids = @($Parent.Id) + (Get-ChildProcesses $Parent.Id)
    $Total = Get-Process -Id $Ids | Measure-Object -Property WorkingSet -Sum
    
    [PSCustomObject]@{
        Launcher = $LauncherName
        ProcessCount = $Ids.Count
        TotalRAM_MB = [Math]::Round($Total.Sum / 1MB, 2)
    }
	(Get-Process NOCLOSE64).PrivateMemorySize64 / 1MB
} else {
    Write-Host "NOCLOSE64.EXE не запущен"
}