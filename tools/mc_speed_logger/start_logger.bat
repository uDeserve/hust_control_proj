@echo off
setlocal
cd /d "%~dp0"

set PORT=%1
if "%PORT%"=="" (
  set /p PORT=Enter COM port (example COM5): 
)

echo Using port: %PORT%
where python
python logger.py --port %PORT% > logger_last_run.txt 2>&1
type logger_last_run.txt

pause
