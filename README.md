# 🏢 SmartElevator – Hệ thống Quản lý Thang máy Thông minh

Hệ thống tích hợp **nhận diện khuôn mặt bằng AI**, **quản lý cư dân chung cư**, và **giám sát thang máy thời gian thực**. Gồm 3 thành phần độc lập hoạt động phối hợp với nhau:

| Thành phần | Công nghệ | Cổng |
|---|---|---|
| 📷 **C++ Edge Device** (Camera + Thang máy) | C++17 · OpenCV · SQLite3 | HTTP `8080` |
| ⚙️ **Central Backend** (API Server) | Node.js · Express · SQLite3 | `3000` |
| 🖥️ **Web Admin UI** (Giao diện quản trị) | React · TypeScript · Vite | `5173` |

---

## 🗺️ Kiến trúc tổng quan

```
┌──────────────────────────────────────────────────────────────┐
│                        ADMIN (Trình duyệt)                   │
│                    http://localhost:5173                      │
└─────────────────────────┬────────────────────────────────────┘
                          │ REST API
                          ▼
┌──────────────────────────────────────────────────────────────┐
│              Central Backend (Node.js/Express)                │
│                    http://localhost:3000                      │
│  - Quản lý cư dân, hóa đơn, bảo trì, thông báo             │
│  - Proxy lệnh đến thiết bị biên C++                          │
│  - Lưu trữ access log, cảnh báo an ninh                     │
└────────────────────┬─────────────────────────────────────────┘
                     │ HTTP (X-API-Key / Proxy)
                     ▼
┌──────────────────────────────────────────────────────────────┐
│                  C++ Edge Device (Camera Server)              │
│                    http://localhost:8080                      │
│  - Nhận diện khuôn mặt qua webcam (LBPH · OpenCV)           │
│  - Điều khiển thang máy (Serial COM3 · 9600 baud)           │
│  - Hiển thị UI thang máy trực tiếp trên màn hình             │
└──────────────────────────────────────────────────────────────┘
```

---

## 🚀 Tính năng chính

### 📷 Thiết bị biên C++ (Edge Device)
- **Nhận diện khuôn mặt LBPH**: Tự động nhận diện cư dân qua webcam và điều khiển thang máy đến tầng đăng ký.
- **State machine thang máy**: Mô phỏng đầy đủ: mở cửa → vào cabin → đóng cửa → di chuyển → đến nơi → mở cửa.
- **Giao tiếp Serial COM3**: Gửi lệnh điều khiển tới phần cứng thang máy qua cổng COM (9600 baud).
- **HTTP API nhúng (Port 8080)**: Nhận lệnh đăng ký khuôn mặt từ xa từ Backend.
- **Đồng bộ cư dân**: Tự động tải danh sách cư dân từ Central Backend khi khởi động.
- **Cảnh báo an ninh**: Phát hiện người lạ và gửi cảnh báo về Backend (tối đa 1 lần/5 giây).

### ⚙️ Central Backend (Node.js)
- **Quản lý cư dân (CRUD)**: Thêm, sửa, xóa cư dân và thông tin căn hộ.
- **Kích hoạt đăng ký khuôn mặt từ xa**: Proxy lệnh từ Web UI → Backend → C++ Device.
- **Quản lý hóa đơn & thanh toán**: Tạo, theo dõi và xác nhận thanh toán.
- **Yêu cầu bảo trì**: Tiếp nhận và xử lý yêu cầu sửa chữa.
- **Thông báo**: Gửi thông báo toàn tòa nhà hoặc từng căn hộ.
- **Nhật ký truy cập & Cảnh báo an ninh**: Lưu lịch sử ra vào thang máy.

### 🖥️ Web Admin UI (React)
- **Dashboard**: Tổng quan thang máy, số cư dân, log gần đây.
- **Quản lý cư dân**: Danh sách, kích hoạt đăng ký khuôn mặt, theo dõi tiến độ.
- **Giám sát thang máy thời gian thực**: Trạng thái, tầng hiện tại, tầng đích.
- **Hóa đơn & Thanh toán**, **Bảo trì**, **Thông báo & Cảnh báo**.

---

## 🛠️ Yêu cầu hệ thống

### Chung
- **Hệ điều hành**: Windows 10/11 (C++ sử dụng Win32 API)
- **Webcam**: Camera USB hoặc camera tích hợp

### C++ Edge Device
| Công cụ | Phiên bản |
|---|---|
| GCC (MinGW-w64 / MSYS2) | ≥ 13 (hỗ trợ C++17) |
| CMake | ≥ 3.10 |
| OpenCV + opencv_contrib | ≥ 4.x (cần module `face`) |
| SQLite3 | ≥ 3.x |

> **Khuyến nghị**: Cài đặt qua [MSYS2](https://www.msys2.org/) với lệnh:
> ```bash
> pacman -S mingw-w64-ucrt-x86_64-opencv mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-cmake
> ```

### Central Backend & Web Admin UI
| Công cụ | Phiên bản |
|---|---|
| Node.js | ≥ 18.x |
| npm | ≥ 9.x |

---

## 📦 Cài đặt & Chạy hệ thống

### ⚡ Cách nhanh nhất (Windows) – Chạy tất cả bằng 1 lệnh

> **Yêu cầu**: C++ đã được biên dịch trước (xem Bước 1 bên dưới).

Nhấp đôi vào file hoặc chạy trong terminal:
```bat
start_all.bat
```

Lệnh này tự động khởi động cả 3 thành phần:
- 🌐 Web Admin UI: **http://localhost:5173**
- ⚙️ Backend API: **http://localhost:3000**
- 📷 Camera C++: HTTP Server Port **8080**

---

### Bước 1 – Biên dịch C++ Edge Device

```bash
# Từ thư mục gốc dự án
mkdir build
cd build

# Cấu hình CMake
cmake .. -G "MinGW Makefiles"

# Biên dịch
cmake --build . --config Release
```

> ⚠️ **Lưu ý**: Mở file `CMakeLists.txt` và kiểm tra đường dẫn `OpenCV_DIR` trỏ đúng đến thư mục OpenCV trên máy bạn.
> Mặc định: `C:/msys64/ucrt64/lib/cmake/opencv4`

File thực thi được tạo tại: `build/SmartElevatorCamera.exe`

---

### Bước 2 – Cài đặt & Chạy Central Backend

```bash
cd backend

# Cài đặt dependencies
npm install

# Biên dịch TypeScript
npm run build

# Chạy server
npm start
```

Server sẽ chạy tại **http://localhost:3000**

Chế độ phát triển (không cần build):
```bash
npm run dev
```

---

### Bước 3 – Cài đặt & Chạy Web Admin UI

```bash
cd frontend

# Cài đặt dependencies
npm install

# Chạy chế độ phát triển
npm run dev
```

Giao diện mở tại **http://localhost:5173**

---

### Bước 4 – Chạy C++ Edge Device

```bash
# Từ thư mục gốc dự án
.\build\SmartElevatorCamera.exe
```

Hai cửa sổ sẽ xuất hiện:
1. **Cửa sổ Camera** – Ảnh webcam với khung nhận diện (xanh = nhận ra, đỏ = người lạ)
2. **Cửa sổ Thang máy UI** – Mô phỏng trạng thái thang máy thời gian thực

---

## 🐳 Triển khai bằng Docker (Tùy chọn)

> **Lưu ý**: Docker chỉ khởi động Backend và Frontend. C++ Edge Device phải chạy trực tiếp trên Windows (cần webcam và cổng COM).

```bash
docker-compose up -d
```

| Dịch vụ | URL |
|---|---|
| PostgreSQL | `localhost:5432` |
| Backend API | **http://localhost:3000** |
| Web Admin UI | **http://localhost:80** |

---

## 🎮 Hướng dẫn sử dụng

### 1. Đăng nhập Web Admin

Truy cập **http://localhost:5173** và đăng nhập bằng tài khoản admin.

### 2. Thêm cư dân mới

1. Vào menu **Quản lý Cư dân** → Nhấn **"Thêm cư dân"**
2. Nhập tên, số căn hộ, và tầng mặc định → **Lưu**

### 3. Đăng ký khuôn mặt cho cư dân

> **Yêu cầu**: C++ Edge Device đang chạy với webcam đã kết nối.

1. Trong danh sách cư dân, nhấn **"Đăng ký khuôn mặt"**
2. Yêu cầu cư dân đứng trước camera
3. Hệ thống tự động chụp **30 mẫu ảnh** khuôn mặt
4. Mô hình AI sẽ được huấn luyện lại tự động sau khi hoàn thành
5. Trạng thái **"Đã đăng ký"** sẽ cập nhật trong danh sách

### 4. Nhận diện tự động & Điều khiển thang máy

Khi C++ Edge Device đang chạy:
1. Cư dân đứng trước camera → Hệ thống nhận diện trong **< 1 giây**
2. Thang máy tự động: mở cửa (1s) → chờ vào (2s) → đóng cửa → di chuyển (1.5s/tầng) → mở cửa tại tầng đích (3s) → đóng lại
3. Nếu phát hiện **người lạ** → gửi cảnh báo an ninh về Backend

### 5. Giám sát thang máy & Nhật ký

- **Tab "Giám sát Thang máy"**: Trạng thái thời gian thực, tầng hiện tại, tiến độ enroll
- **Tab "Nhật ký Truy cập"**: Lịch sử cư dân sử dụng thang máy
- **Tab "Cảnh báo"**: Danh sách cảnh báo an ninh (người lạ)

### 6. Quản lý hóa đơn

1. Vào menu **Hóa đơn** → Tạo hóa đơn mới (kỳ, số tiền, ngày đến hạn)
2. Xác nhận thanh toán bằng nút **"Đã thanh toán"**

### 7. Xử lý yêu cầu bảo trì

1. Vào menu **Bảo trì**
2. Xem danh sách theo mức ưu tiên (`LOW` / `MEDIUM` / `HIGH`)
3. Cập nhật trạng thái: `PENDING` → `IN_PROGRESS` → `DONE`

---

## 🔌 API Reference

### Device API (C++ HTTP Server – Port 8080)

| Method | Endpoint | Mô tả |
|---|---|---|
| `GET` | `/device/status` | Lấy trạng thái thang máy và chế độ hoạt động |
| `POST` | `/device/start-enrollment` | Bắt đầu đăng ký khuôn mặt |
| `POST` | `/device/cancel-enrollment` | Hủy đăng ký khuôn mặt |

**Ví dụ `GET /device/status`:**
```json
{
  "mode": "RECOGNIZING",
  "enrollProgress": 0,
  "elevatorState": "IDLE",
  "currentFloor": 1,
  "targetFloor": 1
}
```

**Ví dụ `POST /device/start-enrollment`:**
```json
// Request body
{ "residentId": 3 }

// Response
{ "status": "ok", "message": "Enrollment started", "residentId": 3, "name": "Tuan C" }
```

### Central Backend API (Port 3000)

| Method | Endpoint | Mô tả |
|---|---|---|
| `POST` | `/api/auth/login` | Đăng nhập admin |
| `GET` | `/api/residents` | Lấy danh sách cư dân |
| `POST` | `/api/residents` | Thêm cư dân mới |
| `PUT` | `/api/residents/:id` | Cập nhật thông tin cư dân |
| `DELETE` | `/api/residents/:id` | Xóa cư dân |
| `POST` | `/api/residents/:id/start-enrollment` | Kích hoạt đăng ký khuôn mặt |
| `GET` | `/api/residents/:id/enrollment-status` | Kiểm tra tiến độ đăng ký |
| `GET` | `/api/bills` | Lấy danh sách hóa đơn |
| `POST` | `/api/bills` | Tạo hóa đơn mới |
| `POST` | `/api/bills/:id/pay` | Xác nhận thanh toán |
| `GET` | `/api/notifications` | Lấy danh sách thông báo |
| `POST` | `/api/notifications` | Gửi thông báo mới |
| `GET` | `/api/maintenance-requests` | Lấy yêu cầu bảo trì |
| `POST` | `/api/maintenance-requests` | Tạo yêu cầu bảo trì |
| `PUT` | `/api/maintenance-requests/:id` | Cập nhật trạng thái bảo trì |
| `GET` | `/api/access-logs` | Lịch sử truy cập thang máy |
| `GET` | `/api/alerts` | Danh sách cảnh báo an ninh |

**Device Sync Endpoints** (Yêu cầu header `X-API-Key: DEV_SECRET_KEY_123`):

| Method | Endpoint | Mô tả |
|---|---|---|
| `GET` | `/api/device/residents` | C++ Device tải danh sách cư dân |
| `POST` | `/api/device/access-logs` | C++ Device ghi nhật ký truy cập |
| `POST` | `/api/device/alerts` | C++ Device gửi cảnh báo an ninh |

---

## 📁 Cấu trúc thư mục

```
SmartElevator/
├── main.cpp                         # Vòng lặp chính: nhận diện, state machine thang máy
├── database.h / database.cpp        # Quản lý SQLite3 và dữ liệu khuôn mặt
├── device_server.h / .cpp           # HTTP server nhúng (cpp-httplib), Port 8080
├── sync_manager.h / .cpp            # Đồng bộ dữ liệu với Backend
├── CMakeLists.txt                   # Cấu hình build CMake
├── httplib.h                        # Thư viện HTTP header-only
├── json.hpp                         # Thư viện JSON (nlohmann)
├── haarcascade_frontalface_alt.xml  # Model Haar Cascade phát hiện khuôn mặt
├── lbph_model.yml                   # Model LBPH (tự sinh khi chạy)
├── elevator_system.db               # SQLite DB thiết bị biên (tự sinh)
├── faces/                           # Ảnh khuôn mặt huấn luyện (tự sinh)
├── start_all.bat                    # Script khởi động tất cả (Windows)
├── docker-compose.yml               # Docker Compose cho Backend + Frontend
├── backend/
│   ├── src/
│   │   ├── server.ts                # Express API server chính
│   │   └── database.ts             # Khởi tạo SQLite3 Backend
│   └── package.json
└── frontend/
    ├── src/
    │   ├── App.tsx                  # Routing và layout chính
    │   ├── types.ts                 # TypeScript type definitions
    │   └── components/
    │       ├── DashboardView.tsx        # Trang tổng quan
    │       ├── ResidentsView.tsx        # Quản lý cư dân + đăng ký khuôn mặt
    │       ├── ElevatorMonitorView.tsx  # Giám sát thang máy thời gian thực
    │       ├── BillsView.tsx            # Hóa đơn & thanh toán
    │       ├── MaintenanceView.tsx      # Yêu cầu bảo trì
    │       ├── NotificationsView.tsx    # Thông báo & cảnh báo
    │       └── AccessLogsView.tsx       # Nhật ký truy cập
    └── package.json
```

---

## ⚙️ Cấu hình

### Thay đổi cổng COM (Serial Port)

Mở `main.cpp` dòng ~591, thay `COM3` bằng cổng đúng trên máy:
```cpp
string portName = "\\\\.\\COM3";  // Đổi thành COM4, COM5, ...
```
Nếu không có cổng COM, hệ thống tự chạy ở **chế độ DEMO**.

### Thay đổi độ nhạy nhận diện

```cpp
// main.cpp
const double RECOGNITION_THRESHOLD = 80.0; // Nhỏ hơn = nghiêm ngặt hơn
const int    ENROLLMENT_SAMPLES     = 30;  // Tăng lên để chính xác hơn
```

### Cấu hình địa chỉ C++ Device cho Backend

```bash
# Nếu C++ Device chạy trên máy khác trong mạng nội bộ
DEVICE_IP=http://192.168.1.100:8080 npm start
```

---

## 🔧 Xử lý sự cố

| Vấn đề | Giải pháp |
|---|---|
| **Không tìm thấy webcam** | Đóng Zoom, Teams, Chrome... Kiểm tra **Windows Settings → Privacy → Camera** |
| **Lỗi COM3 không kết nối** | Hệ thống vẫn chạy ở chế độ DEMO. Xem cổng COM trong Device Manager |
| **Lỗi `haarcascade_frontalface_alt.xml` not found** | Đảm bảo file nằm cùng thư mục với `.exe` |
| **Lỗi OpenCV không tìm thấy khi build** | Cập nhật `OpenCV_DIR` trong `CMakeLists.txt` |
| **Backend lỗi kết nối DB** | Kiểm tra quyền ghi vào thư mục `backend/` |
| **Frontend không gọi được API** | Đảm bảo Backend đang chạy ở cổng 3000 |
| **Web Admin báo "C++ Device offline"** | Đảm bảo `SmartElevatorCamera.exe` đang chạy, Port 8080 không bị tường lửa chặn |
| **Nhận diện sai hoặc không nhận ra** | Đăng ký lại khuôn mặt dưới nhiều điều kiện ánh sáng, đủ 30 mẫu |

---
## 📝 Tài liệu & Báo cáo liên quan

| File | Nội dung |
|---|---|
| `bao_cao_du_an.tex` | Báo cáo tổng kết toàn bộ dự án |
| `bao_cao_thuc_tap_3_tuan.tex` | Báo cáo thực tập 3 tuần |
| `bao_cao_tuan1.tex` → `bao_cao_tuan5.tex` | Báo cáo tiến độ từng tuần |

Biên dịch sang PDF bằng [TeXstudio](https://www.texstudio.org/) hoặc [Overleaf](https://www.overleaf.com/).