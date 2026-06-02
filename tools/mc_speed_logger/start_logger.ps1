param(
    [string]$Port = ""
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

if ([string]::IsNullOrWhiteSpace($Port)) {
    $Port = Read-Host "Enter COM port (example COM11)"
}

Write-Host "Using port: $Port"
python .\logger.py --port $Port 2>&1 | Tee-Object -FilePath .\logger_last_run.txt
Read-Host "Press Enter to close"
