@echo off
setlocal
rem Latest headlessly verified client; native visual qualification remains pending.
rem Update this target only after a successful build and focused checks.
set "clientDir=%~dp0build\mouse-widgets-native\bin"
if not exist "%clientDir%\RoadstersRace.exe" (
    echo The latest verified Roadsters client is missing.
    echo Expected: %clientDir%\RoadstersRace.exe
    echo See docs\CLIENT-LAUNCHER-2026-11-03.md for build and verification details.
    pause
    exit /b 1
)
start "" /D "%clientDir%" "%clientDir%\RoadstersRace.exe"
exit /b %errorlevel%
