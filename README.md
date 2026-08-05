# Smart Elevator Camera System (Hệ thống Camera Thang máy Thông minh)

Dự án này là một ứng dụng C++ mô phỏng hệ thống thang máy thông minh tích hợp camera nhận diện khuôn mặt cư dân. Hệ thống tự động xác định cư dân qua camera, truy vấn tầng đăng ký mặc định của họ trong cơ sở dữ liệu SQLite3, và điều khiển thang máy di chuyển đến tầng tương ứng, đồng thời gửi tín hiệu điều khiển qua cổng COM nối tiếp (Serial Port).

---

## 🚀 Các tính năng chính

1. **Nhận diện khuôn mặt cư dân**: Sử dụng thuật toán nhận diện khuôn mặt LBPH (Local Binary Patterns Histograms) từ thư viện OpenCV.
2. **Quản lý dữ liệu cư dân (CRUD)**: Lưu trữ thông tin cá nhân (ID, tên, căn hộ, tầng đăng ký mặc định) và tập dữ liệu khuôn mặt vào cơ sở dữ liệu SQLite3.
3. **Mô phỏng thang máy trực quan**: Giao diện hiển thị trực quan trạng thái thang máy (trạng thái mở/đóng cửa, tầng hiện tại, tầng đích, hình ảnh cư dân bước vào) sử dụng GUI của OpenCV.
4. **Giao tiếp cổng COM (Serial Port)**: Kết nối và gửi lệnh điều khiển thang máy tới phần cứng bên ngoài (hoặc phần mềm mô phỏng cổng COM ảo).

---

## 📁 Cấu trúc thư mục dự án nên đẩy lên GitHub

Để người đọc dễ dàng hiểu và chạy được dự án, bạn nên đẩy lên các file và thư mục sau:

| Tên File/Thư mục | Mô tả | Trạng thái đẩy lên |
| :--- | :--- | :--- |
| `main.cpp` | Chứa luồng xử lý chính, máy trạng thái thang máy, giao diện đồ họa và cấu hình giao tiếp cổng COM. | **Bắt buộc** |
| `database.h` & `database.cpp` | Khai báo và định nghĩa lớp quản lý cơ sở dữ liệu SQLite3 và tập dữ liệu huấn luyện khuôn mặt. | **Bắt buộc** |
| `CMakeLists.txt` | File cấu hình CMake giúp người khác dễ dàng cài đặt thư viện và build dự án trên máy của họ. | **Bắt buộc** |
| `.gitignore` | Định nghĩa các file tạm thời, file build (`build/`), thư mục môi trường ảo Python (`docx_venv/`), và file cấu hình IDE (`.vscode/`) để không bị đẩy nhầm lên GitHub. | **Bắt buộc** |
| `bao_cao_du_an.tex` | File báo cáo dự án định dạng LaTeX. Nếu đây là đồ án hoặc bài tập lớn, bạn có thể đẩy lên file này cùng các file tuần liên quan. | *Khuyến khích* |
| `README.md` | Tài liệu hướng dẫn này để hiển thị trên trang chủ kho lưu trữ GitHub. | **Bắt buộc** |

### Các thư mục/file **KHÔNG** nên đẩy lên GitHub (đã được cấu hình trong `.gitignore`):
- Thư mục `build/` (Chứa các file tạm khi biên dịch và file thực thi `.exe`).
- Thư mục `.vscode/` (Chứa cấu hình cá nhân của phần mềm soạn thảo VS Code).
- Thư mục `docx_venv/` (Môi trường ảo Python của cá nhân bạn).
- Các file mô hình đã huấn luyện như `lbph_model.yml` hoặc file cơ sở dữ liệu SQLite thực tế chứa dữ liệu cá nhân (trừ khi bạn muốn cung cấp một cơ sở dữ liệu mẫu nhỏ).

---

## 🛠️ Yêu cầu hệ thống & Thư viện liên kết

Để biên dịch và chạy dự án này, máy tính cần cài đặt các công cụ sau:

- **Hệ điều hành**: Windows (do sử dụng thư viện `windows.h` để quản lý cổng COM).
- **Trình biên dịch**: GCC (hỗ trợ C++17) hoặc MSVC (Visual Studio).
- **CMake**: Phiên bản 3.10 trở lên.
- **Thư viện OpenCV**: Yêu cầu cài đặt thêm gói `opencv_contrib` (có chứa module `face` để nhận diện khuôn mặt).
- **Thư viện SQLite3**: Để lưu trữ và quản lý cơ sở dữ liệu.

---

## 🔨 Hướng dẫn biên dịch và chạy dự án

1. **Clone dự án từ GitHub**:
   ```bash
   git clone <link_github_cua_ban>
   cd <ten_thu_muc_du_an>
   ```

2. **Tạo thư mục build và chạy CMake**:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

3. **Chạy chương trình**:
   Sau khi biên dịch thành công, file thực thi sẽ được tạo ra trong thư mục `build/`. Bạn chạy trực tiếp:
   ```bash
   ./SmartElevatorCamera
   ```

*(Lưu ý: Đảm bảo bạn đã cấu hình đúng đường dẫn OpenCV trong `CMakeLists.txt` tương ứng với đường dẫn trên máy tính của bạn trước khi chạy CMake).*

---

## 📝 Tài liệu & Báo cáo liên quan
- Các file `.tex` trong dự án chứa chi tiết nội dung báo cáo thực tập và tiến độ hàng tuần của dự án. Bạn có thể biên dịch chúng sang file PDF bằng công cụ biên dịch LaTeX (như TeXstudio hoặc Overleaf) để đọc báo cáo chi tiết.
