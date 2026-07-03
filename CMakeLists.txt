cmake_minimum_required(VERSION 3.10)
project(SmartElevatorCamera)

# Yêu cầu chuẩn C++17
set(CMAKE_CXX_STANDARD 17)
set(OpenCV_DIR "C:/msys64/ucrt64/lib/cmake/opencv4")
# Tìm kiếm thư viện OpenCV và SQLite3 trong hệ thống
find_package(OpenCV REQUIRED)
find_package(SQLite3 REQUIRED)

# Thêm thư mục include của OpenCV và SQLite3
include_directories(${OpenCV_INCLUDE_DIRS} ${SQLite3_INCLUDE_DIRS})

# Tạo file thực thi từ main.cpp và database.cpp
add_executable(SmartElevatorCamera main.cpp database.cpp)

# Liên kết (Link) các thư viện của OpenCV và SQLite3 vào file thực thi
target_link_libraries(SmartElevatorCamera ${OpenCV_LIBS} ${SQLite3_LIBRARIES})