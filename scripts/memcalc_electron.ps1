$LauncherName = "NOCLOSE64"
$Parent = Get-Process $LauncherName -ErrorAction SilentlyContinue

function Get-ChildProcesses($ParentId) {
    $Children = Get-CimInstance Win32_Process -Filter "ParentProcessId=$ParentId"
    foreach ($Child in $Children) {
        $Child.ProcessId
        Get-ChildProcesses $Child.ProcessId
    }
}

if ($Parent) {
    $Ids = @($Parent.Id) + (Get-ChildProcesses $Parent.Id)
    $Processes = Get-CimInstance Win32_Process | Where-Object { $Ids -contains $_.ProcessId }

    $WebView = $Processes | Where-Object { $_.Name -eq "msedgewebview2.exe" }
    $Node    = $Processes | Where-Object { $_.Name -like "node*" }
    $Other   = $Processes | Where-Object { $_.Name -notin @("msedgewebview2.exe") -and $_.Name -notlike "node*" }

    function SumMB($list) {
        ($list | Measure-Object -Property WorkingSetSize -Sum).Sum / 1MB
    }

    [PSCustomObject]@{
        Total_MB   = [Math]::Round((SumMB $Processes),2)
        WebView_MB = [Math]::Round((SumMB $WebView),2)
        Node_MB    = [Math]::Round((SumMB $Node),2)
        Other_MB   = [Math]::Round((SumMB $Other),2)
        Count      = $Ids.Count
    }
}