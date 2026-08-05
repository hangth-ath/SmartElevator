@echo off
title SmartElevator Launcher
echo =======================================================
echo     KHOI CHAY HE THONG QUAN LY CHUNG CU SMARTELEVATOR
echo =======================================================

echo [0/3] Don dep tien trinh va cong cu (Port 3000, 5173)...
taskkill /f /im node.exe /im SmartElevatorCamera.exe >nul 2>&1
timeout /t 1 /nobreak >nul

echo [1/3] Dang khoi chay Central Backend (Port 3000)...
start "SmartElevator Backend" powershell -ExecutionPolicy Bypass -NoExit -Command "$env:Path = 'C:\Exercsise - Copy - Copy - Copy\node-bin;' + $env:Path; Set-Location 'C:\Exercsise - Copy - Copy - Copy\backend'; node dist/server.js"

echo [2/3] Dang khoi chay Web Frontend Admin UI (Port 5173)...
start "SmartElevator Frontend" powershell -ExecutionPolicy Bypass -NoExit -Command "$env:Path = 'C:\Exercsise - Copy - Copy - Copy\node-bin;' + $env:Path; Set-Location 'C:\Exercsise - Copy - Copy - Copy\frontend'; npm run preview"

echo [3/3] Dang khoi chay Camera C++ Edge Device...
start "SmartElevator Camera C++" powershell -ExecutionPolicy Bypass -NoExit -Command "Set-Location 'C:\Exercsise - Copy - Copy - Copy'; .\build\SmartElevatorCamera.exe"

echo.
echo =======================================================
echo   DA KHOI CHAY THANH CONG TOAN BO HE THONG!
echo   - Web Admin UI: http://localhost:5173
echo   - Backend API : http://localhost:3000
echo   - Camera C++  : HTTP Server Port 8080
echo =======================================================
