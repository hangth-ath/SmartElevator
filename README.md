# 🏢 Smart Elevator Camera System (Hệ thống Camera Thang máy Thông minh)

Ứng dụng C++ mô phỏng hệ thống thang máy thông minh tích hợp camera nhận diện khuôn mặt cư dân. Hệ thống tự động phát hiện cư dân qua camera (OpenCV LBPH), truy vấn tầng đăng ký mặc định trong cơ sở dữ liệu SQLite3, hiển thị mô phỏng 2D quá trình di chuyển của thang máy và gửi tín hiệu điều khiển qua cổng nối tiếp Serial COM (Win32 API).

---

## 🚀 Các tính năng chính

1. **Nhận diện khuôn mặt cư dân (AI Face Recognition)**:
   - Sử dụng thuật toán **LBPH (Local Binary Patterns Histograms)** từ thư viện `opencv_contrib`.
   - Nhận diện cư dân theo thời gian thực từ Webcam/Camera.
   - Tự động nhận biết người lạ (hiển thị khung cảnh báo màu đỏ).

2. **Quản lý dữ liệu cư dân (SQLite3 Database)**:
   - Lưu trữ thông tin cá nhân (ID, Tên, Căn hộ, Tầng mặc định) vào file SQLite (`elevator_system.db`).
   - Quản lý và lưu trữ các mẫu ảnh khuôn mặt trong thư mục `faces/`.
   - Tự động nạp dữ liệu cư dân mẫu khi mở ứng dụng lần đầu.

3. **Chế độ đăng ký & Huấn luyện khuôn mặt (Face Enrollment & Auto-Train)**:
   - Hỗ trợ chụp trực tiếp tập mẫu khuôn mặt (30 ảnh mẫu) từ camera.
   - Tự động huấn luyện lại và lưu mô hình AI ra file cache (`lbph_model.yml`).

4. **Mô phỏng giao diện Thang máy 2D (OpenCV GUI)**:
   - Hiển thị trực quan cabin thang máy di chuyển qua các tầng.
   - Mô phỏng động tác đóng/mở cửa thang máy trượt 2 cánh.
   - Hiển thị hình ảnh & thông tin căn hộ của cư dân bên trong cabin.

5. **Giao tiếp Cổng nối tiếp (Serial Port / COM)**:
   - Tự động kết nối tới cổng `COM3` với tốc độ Baud `9600`.
   - Gửi tín hiệu tầng tương ứng (`'1'` đến `'9'`,...) tới phần cứng hoặc mô phỏng.
   - **Tự động chuyển sang chế độ DEMO** nếu không tìm thấy cổng COM (vẫn chạy đầy đủ tính năng nhận diện & GUI).

---

## 📁 Cấu trúc thư mục dự án

```text
SmartElevator/
├── CMakeLists.txt                 # Cấu hình biên dịch CMake
├── main.cpp                       # Luồng xử lý chính, Camera, AI, GUI thang máy & Serial COM
├── database.h / database.cpp      # Quản lý cơ sở dữ liệu SQLite3 và lưu ảnh khuôn mặt
├── haarcascade_frontalface_alt.xml# File mô hình phát hiện khuôn mặt Haar Cascade của OpenCV
├── README.md                      # Tài liệu hướng dẫn sử dụng
├── .gitignore                     # Danh sách bỏ qua khi commit Git
└── build/                         # Thư mục chứa sản phẩm sau biên dịch (tự tạo)
    ├── SmartElevatorCamera.exe    # File ứng dụng thực thi
    ├── run.bat                    # Script khởi chạy ứng dụng (thêm DLL PATH & chạy exe)
    ├── elevator_system.db         # Cơ sở dữ liệu SQLite (tự sinh ra khi chạy)
    ├── lbph_model.yml             # Mô hình AI đã huấn luyện (tự sinh ra khi chạy)
    └── faces/                     # Thư mục chứa tập ảnh khuôn mặt (tự sinh ra khi chạy)
```

---

## 🛠️ Yêu cầu hệ thống & Môi trường cài đặt

### 1. Hệ điều hành & Trình biên dịch
- **Hệ điều hành**: Windows 10 / 11 (do ứng dụng sử dụng Win32 API `<windows.h>` để điều khiển Cổng COM).
- **Trình biên dịch**: GCC (MSYS2 UCRT64 / MinGW-w64) hỗ trợ **C++17** hoặc MSVC.

### 2. Các công cụ & Thư viện cần thiết
Nếu sử dụng **MSYS2 UCRT64** (khuyên dùng), mở **MSYS2 UCRT64 Terminal** và cài đặt các gói sau:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-make
pacman -S mingw-w64-ucrt-x86_64-opencv
pacman -S mingw-w64-ucrt-x86_64-sqlite3
```

*(Lưu ý: Gói `opencv` trên MSYS2 đã bao gồm module `opencv_contrib/face` cần thiết cho thuật toán LBPH).*

---

## 🔨 Hướng dẫn Biên dịch và Chạy ứng dụng

### Bước 1: Clone kho lưu trữ
```bash
git clone <link_github_cua_ban>
cd SmartElevator
```

### Bước 2: Biên dịch ứng dụng bằng CMake
Mở terminal (**Command Prompt** hoặc **MSYS2 UCRT64 Terminal**) tại thư mục dự án và chạy các lệnh sau:

```cmd
# 1. Tạo thư mục build và di chuyển vào
mkdir build
cd build

# 2. Tạo file cấu hình build với CMake
cmake -G "MinGW Makefiles" ..

# 3. Biên dịch chương trình
cmake --build .
```

*Sau khi biên dịch thành công, file `SmartElevatorCamera.exe` sẽ xuất hiện trong thư mục `build/`.*

### Bước 3: Chuẩn bị file dữ liệu
Đảm bảo file phân loại khuôn mặt `haarcascade_frontalface_alt.xml` nằm ở thư mục chạy chương trình (`build/`). Copy file từ thư mục gốc vào `build/` nếu chưa có:

```cmd
copy ..\haarcascade_frontalface_alt.xml .
```

### Bước 4: Chạy chương trình

#### **Cách 1: Chạy qua file `run.bat` (Khuyên dùng)**
Do ứng dụng cần liên kết với các thư viện DLL (OpenCV, SQLite3...), file `run.bat` đã được cấu hình đường dẫn `PATH` tự động:

```cmd
run.bat
```

#### **Cách 2: Chạy trực tiếp qua Executable**
```cmd
SmartElevatorCamera.exe
```
*(Lưu ý: Nếu báo thiếu DLL, hãy thêm `C:\msys64\ucrt64\bin` vào biến môi trường `PATH` của Windows).*

---

## 🎮 Hướng dẫn Sử dụng & Phím tắt

### 1. Khởi động ứng dụng
- Chương trình sẽ tự động mở cổng **COM3**. Nếu không tìm thấy thiết bị kết nối cổng COM3, hệ thống xuất cảnh báo `[CANH BAO] Khong the ket noi cong COM3. Chay o che do DEMO` và tiếp tục hoạt động bình thường.
- Tự động nạp danh sách cư dân mẫu nếu cơ sở dữ liệu trống (*Tuan A - P502 - Tầng 5*, *Tuan B - P804 - Tầng 8*, *Tuan C - P1005 - Tầng 10*).
- Tự động kết nối và mở Webcam máy tính.

### 2. Đăng ký khuôn mặt (Enrollment)
- Nếu hệ thống chưa có dữ liệu ảnh khuôn mặt, ứng dụng sẽ hỏi bạn có muốn chụp ảnh đăng ký cho từng cư dân hay không.
- Khi giao diện camera đăng ký hiện ra:
  - Nhấn **`S`** (Save): Chụp 1 mẫu ảnh khuôn mặt (cần chụp đủ 30 mẫu ảnh).
  - Nhấn **`Q`** (Quit): Hoàn thành đăng ký cho cư dân đó.
- Sau khi hoàn thành đăng ký, ứng dụng tự động **huấn luyện mô hình AI (LBPH)** và lưu vào cache `lbph_model.yml`.

### 3. Điều khiển trong quá trình chạy
Trên màn hình quẹt Camera chính:
- Nhấn **`E`**: Mở lại menu đăng ký thêm khuôn mặt cho cư dân.
- Nhấn **`Q`**: Thoát ứng dụng.

### 4. Quy trình hoạt động của Thang máy
1. Khi có người đứng trước Camera, hệ thống khoanh vùng khuôn mặt.
2. Nếu là **Cư dân đã đăng ký** (khung xanh lá): Nhận diện ID -> Tìm Tầng đăng ký mặc định -> Kích hoạt Thang máy -> Mở cửa cabin -> Cư dân bước vào -> Đóng cửa -> Thang máy di chuyển đến tầng đích -> Mở cửa tại tầng đích -> Gửi tín hiệu điều khiển tầng qua Cổng COM.
3. Nếu là **Người lạ** (khung đỏ): Hiển thị thông báo "Nguoi la! Khong co quyen" và không kích hoạt thang máy.

---

## ❓ Xử lý sự cố thường gặp (Troubleshooting)

| Sự cố | Nguyên nhân | Cách khắc phục |
| :--- | :--- | :--- |
| `Loi: Khong tim thay haarcascade_frontalface_alt.xml!` | Thiếu file Haar Cascade trong thư mục làm việc `build/`. | Copy file `haarcascade_frontalface_alt.xml` vào cùng thư mục chứa `SmartElevatorCamera.exe` (`build/`). |
| `Loi: Khong the mo bat ky webcam nao!` | Camera đang bị phần mềm khác sử dụng (Zoom, Teams, Browser) hoặc bị khóa quyền. | Đóng các ứng dụng đang dùng camera và kiểm tra *Windows Settings -> Privacy & Security -> Camera*. |
| Thiếu DLL (`libopencv_core...dll`, `sqlite3.dll`...) | Windows không tìm thấy các file thư viện DLL của MSYS2. | Chạy ứng dụng qua file `run.bat` hoặc thêm `C:\msys64\ucrt64\bin` vào biến môi trường `PATH`. |
| `Khong the ket noi cong COM3` | Chưa kết nối phần cứng Cổng COM hoặc chưa cài Virtual Serial Port. | Không ảnh hưởng. Hệ thống tự chuyển sang **Chế độ DEMO** và vẫn hoạt động đầy đủ tính năng AI & Giao diện thang máy. |

---


