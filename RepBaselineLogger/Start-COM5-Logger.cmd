@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0SaveReadings.ps1" -PortName COM5
pause
