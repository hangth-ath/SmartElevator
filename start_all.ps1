# =========================================================
# File Khởi Chạy Tự Động Toàn Bộ Hệ Thống SmartElevator
# =========================================================

Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host "    KHOI CHAY HE THONG QUAN LY CHUNG CU SMARTELEVATOR  " -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan

# 0. Tắt các tiến trình node hoặc camera đang chạy ngầm để giải phóng port 3000, 5173
Write-Host "[0/3] Don dep cong va tien trinh cu (Port 3000, 5173)..." -ForegroundColor Gray
Get-Process -Name node, SmartElevatorCamera -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 1

# 1. Thêm Node.js portable vào biến môi trường PATH
$env:Path = "C:\Exercsise - Copy - Copy - Copy\node-bin;" + $env:Path

# 2. Khởi chạy Central Backend Server (Port 3000)
Write-Host "[1/3] Dang khoi chay Central Backend (Port 3000)..." -ForegroundColor Yellow
Start-Process powershell -ArgumentList "-NoExit", "-Command", "`$env:Path = 'C:\Exercsise - Copy - Copy - Copy\node-bin;' + `$env:Path; Set-Location 'C:\Exercsise - Copy - Copy - Copy\backend'; node dist/server.js"

# 3. Khởi chạy Web Frontend Admin UI (Port 5173)
Write-Host "[2/3] Dang khoi chay Web Frontend Admin UI (Port 5173)..." -ForegroundColor Yellow
Start-Process powershell -ArgumentList "-NoExit", "-Command", "`$env:Path = 'C:\Exercsise - Copy - Copy - Copy\node-bin;' + `$env:Path; Set-Location 'C:\Exercsise - Copy - Copy - Copy\frontend'; npm run preview"

# 4. Khởi chạy Ứng dụng C++ SmartElevator Camera
Write-Host "[3/3] Dang khoi chay Camera C++ Edge Device..." -ForegroundColor Yellow
Start-Process powershell -ArgumentList "-NoExit", "-Command", "Set-Location 'C:\Exercsise - Copy - Copy - Copy'; .\build\SmartElevatorCamera.exe"

Write-Host ""
Write-Host "=======================================================" -ForegroundColor Green
Write-Host "  DA KHOI CHAY THANH CONG TOAN BO HE THONG!" -ForegroundColor Green
Write-Host "  -> Web Admin UI: http://localhost:5173" -ForegroundColor Green
Write-Host "  -> Backend API : http://localhost:3000" -ForegroundColor Green
Write-Host "  -> Camera C++  : HTTP Server Port 8080" -ForegroundColor Green
Write-Host "=======================================================" -ForegroundColor Green
