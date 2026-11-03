@echo off
setlocal
if exist "%~dp0dist\first-flight\RoadstersRace.exe" (
    start "" "%~dp0dist\first-flight\RoadstersRace.exe"
    exit /b 0
)
echo Build the game first:
echo powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\Build-Windows.ps1" -Package
pause
exit /b 1
