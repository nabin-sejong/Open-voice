@echo off
:: OpenVoice AAC Communicator — Launcher
:: Double-click this file (or run it in a terminal) to start OpenVoice.
:: It sets the working directory correctly so phrases and profile are found.

title OpenVoice AAC Communicator

:: Change to the folder this .bat lives in (works from any location)
cd /d "%~dp0"

:: Check that openvoice.exe is present
if not exist "openvoice.exe" (
    echo ERROR: openvoice.exe not found in this folder.
    echo Please make sure you are running this from the OpenVoice folder.
    pause
    exit /b 1
)

:: Check PowerShell is available (required for speech synthesis)
where powershell >nul 2>&1
if errorlevel 1 (
    echo ERROR: PowerShell is not found on this system.
    echo OpenVoice requires Windows PowerShell 5.1 or later.
    echo It is included with Windows 10 and Windows 11 by default.
    pause
    exit /b 1
)

:: Launch the application
openvoice.exe

:: On exit, pause so any error message is visible
if errorlevel 1 (
    echo.
    echo OpenVoice exited with an error (code %errorlevel%).
    pause
)
