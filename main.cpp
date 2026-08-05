#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>       // Win32 API - dieu khien cong COM
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp> // LBPH Face Recognizer
#include "database.h"
#include "device_server.h"
#include "sync_manager.h"

using namespace std;
using namespace cv;
using namespace cv::face;

// -------------------------------------------------------
// Hang so cau hinh he thong
// -------------------------------------------------------
const double RECOGNITION_THRESHOLD = 80.0; // Nguong nhan dien (cang nho cang ketat)
const int    FACE_IMG_SIZE          = 100;  // Kich thuoc chuan hoa anh khuon mat
const int    ENROLLMENT_SAMPLES     = 30;   // So mau anh can chup khi dang ky
const string MODEL_FILE_PATH        = "lbph_model.yml"; // Duong dan file luu mo hinh LBPH

// -------------------------------------------------------
// Trạng thái thiết bị (Device State) & Bien dung chung Thread-safe
// -------------------------------------------------------
enum class DeviceMode { RECOGNIZING, ENROLLING };
std::atomic<DeviceMode> deviceMode{DeviceMode::RECOGNIZING};
std::atomic<int> pendingResidentId{-1};   // Id cu dan dang cho enroll
std::atomic<int> enrollProgress{0};       // Tien do enroll 0..30
std::mutex elevatorMutex;                 // Mutex cho du lieu thang may

// -------------------------------------------------------
// Ham ho tro loai bo dau tieng Viet cho OpenCV putText
// -------------------------------------------------------
std::string removeVietnameseAccents(const std::string& str) {
    std::string res;
    res.reserve(str.size());
    for (size_t i = 0; i < str.size(); ) {
        unsigned char c = (unsigned char)str[i];
        if (c < 0x80) {
            res += (char)c;
            i++;
        } else {
            std::string sub;
            if ((c & 0xE0) == 0xC0 && i + 1 < str.size()) {
                sub = str.substr(i, 2);
                i += 2;
            } else if ((c & 0xF0) == 0xE0 && i + 2 < str.size()) {
                sub = str.substr(i, 3);
                i += 3;
            } else if ((c & 0xF8) == 0xF0 && i + 3 < str.size()) {
                sub = str.substr(i, 4);
                i += 4;
            } else {
                i++;
                continue;
            }

            if (sub == "à" || sub == "á" || sub == "ả" || sub == "ã" || sub == "ạ" ||
                sub == "â" || sub == "ầ" || sub == "ấ" || sub == "ẩ" || sub == "ẫ" || sub == "ậ" ||
                sub == "ă" || sub == "ằ" || sub == "ắ" || sub == "ẳ" || sub == "ẵ" || sub == "ặ") res += 'a';
            else if (sub == "À" || sub == "Á" || sub == "Ả" || sub == "Ã" || sub == "Ạ" ||
                     sub == "Â" || sub == "Ầ" || sub == "Ấ" || sub == "Ẩ" || sub == "Ẫ" || sub == "Ậ" ||
                     sub == "Ă" || sub == "Ằ" || sub == "Ắ" || sub == "Ẳ" || sub == "Ẵ" || sub == "Ặ") res += 'A';
            else if (sub == "è" || sub == "é" || sub == "ẻ" || sub == "ẽ" || sub == "ẹ" ||
                     sub == "ê" || sub == "ề" || sub == "ế" || sub == "ể" || sub == "ễ" || sub == "ệ") res += 'e';
            else if (sub == "È" || sub == "É" || sub == "Ẻ" || sub == "Ẽ" || sub == "Ẹ" ||
                     sub == "Ê" || sub == "Ề" || sub == "Ế" || sub == "Ể" || sub == "Ễ" || sub == "Ệ") res += 'E';
            else if (sub == "ì" || sub == "í" || sub == "ỉ" || sub == "ĩ" || sub == "ị") res += 'i';
            else if (sub == "Ì" || sub == "Í" || sub == "Ỉ" || sub == "Ĩ" || sub == "Ị") res += 'I';
            else if (sub == "ò" || sub == "ó" || sub == "ỏ" || sub == "õ" || sub == "ọ" ||
                     sub == "ô" || sub == "ồ" || sub == "ố" || sub == "ổ" || sub == "ỗ" || sub == "ộ" ||
                     sub == "ơ" || sub == "ờ" || sub == "ớ" || sub == "ở" || sub == "ỡ" || sub == "ợ") res += 'o';
            else if (sub == "Ò" || sub == "Ó" || sub == "Ỏ" || sub == "Õ" || sub == "Ọ" ||
                     sub == "Ô" || sub == "Ồ" || sub == "Ố" || sub == "Ổ" || sub == "Ỗ" || sub == "Ộ" ||
                     sub == "Ơ" || sub == "Ờ" || sub == "Ớ" || sub == "Ở" || sub == "Ỡ" || sub == "Ợ") res += 'O';
            else if (sub == "ù" || sub == "ú" || sub == "ủ" || sub == "ũ" || sub == "ụ" ||
                     sub == "ư" || sub == "ừ" || sub == "ứ" || sub == "ử" || sub == "ữ" || sub == "ự") res += 'u';
            else if (sub == "Ù" || sub == "Ú" || sub == "Ủ" || sub == "Ũ" || sub == "Ụ" ||
                     sub == "Ư" || sub == "Ừ" || sub == "Ứ" || sub == "Ử" || sub == "Ữ" || sub == "Ự") res += 'U';
            else if (sub == "ỳ" || sub == "ý" || sub == "ỷ" || sub == "ỹ" || sub == "ỵ") res += 'y';
            else if (sub == "Ỳ" || sub == "Ý" || sub == "Ỷ" || sub == "Ỹ" || sub == "Ỵ") res += 'Y';
            else if (sub == "đ") res += 'd';
            else if (sub == "Đ") res += 'D';
            else res += ' ';
        }
    }
    return res;
}

// -------------------------------------------------------
// Thong tin & Trang thai Thang may
// -------------------------------------------------------
enum ElevatorState {
    ELEVATOR_IDLE,
    ELEVATOR_DOOR_OPENING_BEFORE,
    ELEVATOR_BOARDING,           // Cu dan dang vao, cua mo
    ELEVATOR_DOOR_CLOSING_BEFORE,
    ELEVATOR_MOVING,             // Thang may dang di chuyen
    ELEVATOR_ARRIVED,            // Da den tang, cua mo
    ELEVATOR_DOOR_CLOSING_AFTER
};

struct Elevator {
    ElevatorState state = ELEVATOR_IDLE;
    int currentFloor = 1;
    int targetFloor = 1;
    string passengerName = "";
    string passengerApartment = "";
    Mat passengerFace;
    
    // Thong tin cua
    const int maxDoorWidth = 120; // Cua moi ben rong 120px (tong 240px)
    int currentDoorOpenWidth = 0; // 0: dong hoan toan, 120: mo hoan toan
    
    // Thong tin di chuyen
    double currentVisualFloor = 1.0;
    double stateTimeStart = 0.0;
    int totalFloors = 10;
};

// -------------------------------------------------------
// Ham cau hinh cong COM
// -------------------------------------------------------
HANDLE initSerialPort(const string& portName) {
    HANDLE hSerial = CreateFileA(portName.c_str(),
        GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        cerr << "Loi: Khong the mo cong " << portName << ". Kiem tra lai Virtual Serial Port!" << endl;
        return INVALID_HANDLE_VALUE;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        cerr << "Loi: Khong lay duoc trang thai cong COM!" << endl;
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        cerr << "Loi: Khong the thiet lap cau hinh cong COM!" << endl;
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout         = 50;
    timeouts.ReadTotalTimeoutConstant    = 50;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.WriteTotalTimeoutConstant   = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        cerr << "Loi: Khong the thiet lap Timeouts cho cong COM!" << endl;
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    return hSerial;
}

void sendData(HANDLE hSerial, char data) {
    DWORD bytesWritten;
    WriteFile(hSerial, &data, 1, &bytesWritten, NULL);
}

// -------------------------------------------------------
// Che do DANG KY khuon mat cho mot cu dan
// -------------------------------------------------------
void enrollFace(VideoCapture& cap, CascadeClassifier& faceCascade,
                DatabaseManager& db, const Resident& resident) {

    int sampleCount = db.getFaceImageCount(resident.id);
    enrollProgress = sampleCount;
    cout << "\n[DANG KY] Bat dau chup khuon mat cho: " << resident.name
         << " (da co " << sampleCount << "/" << ENROLLMENT_SAMPLES << " mau)" << endl;

    Mat frame, gray;
    string windowName = "Dang ky khuon mat - " + removeVietnameseAccents(resident.name);

    while (true) {
        if (deviceMode.load() != DeviceMode::ENROLLING) {
            cout << "[DANG KY] Nhan lenh huy dang ky tu xa (Cancel). Thoat." << endl;
            break;
        }

        cap >> frame;
        if (frame.empty()) break;

        cvtColor(frame, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, gray);

        vector<Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, Size(80, 80));

        for (const auto& face : faces) {
            rectangle(frame, face, Scalar(255, 165, 0), 2);
        }

        string statusText = "Mau: " + to_string(sampleCount) + "/" + to_string(ENROLLMENT_SAMPLES);
        putText(frame, statusText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 200, 255), 2);
        putText(frame, "Dang tu dong chup | [Q] de thoat", Point(10, 60), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);

        imshow(windowName, frame);

        int key = waitKey(30) & 0xFF;

        if (!faces.empty()) {
            Mat faceROI = gray(faces[0]);
            Mat resized;
            resize(faceROI, resized, Size(FACE_IMG_SIZE, FACE_IMG_SIZE));

            if (db.saveFaceImage(resident.id, resized, sampleCount)) {
                sampleCount++;
                enrollProgress = sampleCount;
                cout << "[DANG KY] Da luu mau " << sampleCount << "/" << ENROLLMENT_SAMPLES << endl;
            }

            if (sampleCount >= ENROLLMENT_SAMPLES) {
                cout << "[DANG KY] Da du " << ENROLLMENT_SAMPLES << " mau. Hoan thanh dang ky!" << endl;
                break;
            }
        }

        if (key == 'q' || key == 'Q') {
            cout << "[DANG KY] Nguoi dung thoat khi co " << sampleCount << " mau." << endl;
            break;
        }
    }

    destroyWindow(windowName);
}

// -------------------------------------------------------
// Huan luyen mo hinh LBPH tu du lieu trong DB
// -------------------------------------------------------
Ptr<LBPHFaceRecognizer> trainRecognizer(DatabaseManager& db,
                                        const string& modelPath = MODEL_FILE_PATH) {
    FaceTrainingData data = db.loadAllFaceTrainingData();

    if (data.images.empty()) {
        cerr << "[NHAN DIEN] Khong co du lieu khuon mat de huan luyen!" << endl;
        return nullptr;
    }

    auto recognizer = LBPHFaceRecognizer::create();
    recognizer->train(data.images, data.labels);
    cout << "[NHAN DIEN] Da huan luyen mo hinh LBPH voi "
         << data.images.size() << " anh khuon mat." << endl;

    recognizer->save(modelPath);
    cout << "[NHAN DIEN] Da luu mo hinh LBPH ra file: " << modelPath << endl;

    return recognizer;
}

// -------------------------------------------------------
// Tai mo hinh LBPH da duoc luu tu file
// -------------------------------------------------------
Ptr<LBPHFaceRecognizer> loadRecognizer(const string& modelPath = MODEL_FILE_PATH) {
    DWORD attr = GetFileAttributesA(modelPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        return nullptr;
    }

    try {
        auto recognizer = LBPHFaceRecognizer::create();
        recognizer->read(modelPath);
        cout << "[NHAN DIEN] Da tai mo hinh LBPH tu file: " << modelPath << endl;
        return recognizer;
    } catch (const cv::Exception& e) {
        cerr << "[NHAN DIEN] Loi khi doc file mo hinh: " << e.what() << endl;
        return nullptr;
    }
}

// -------------------------------------------------------
// Kiem tra thoi gian chinh sua file
// -------------------------------------------------------
LARGE_INTEGER getFileModTime(const string& filePath) {
    LARGE_INTEGER result = {0};
    HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ,
                               FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return result;
    FILETIME ftWrite;
    if (GetFileTime(hFile, NULL, NULL, &ftWrite)) {
        result.LowPart  = ftWrite.dwLowDateTime;
        result.HighPart = (LONG)ftWrite.dwHighDateTime;
    }
    CloseHandle(hFile);
    return result;
}

// -------------------------------------------------------
// So sanh: model file co moi hon tat ca anh huan luyen?
// -------------------------------------------------------
bool isModelUpToDate(const string& modelPath, const string& facesDir) {
    LARGE_INTEGER modelTime = getFileModTime(modelPath);
    if (modelTime.QuadPart == 0) return false;

    WIN32_FIND_DATAA ffd;
    string searchPath = facesDir + "\\*\\*.jpg";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) return true;

    bool modelIsNewer = true;
    do {
        LARGE_INTEGER imgTime = {0};
        imgTime.LowPart  = ffd.ftLastWriteTime.dwLowDateTime;
        imgTime.HighPart = (LONG)ffd.ftLastWriteTime.dwHighDateTime;
        if (imgTime.QuadPart > modelTime.QuadPart) {
            modelIsNewer = false;
            break;
        }
    } while (FindNextFileA(hFind, &ffd));
    FindClose(hFind);

    return modelIsNewer;
}

// -------------------------------------------------------
// Ham thoi gian thuc te (ms)
// -------------------------------------------------------
double getTimeMs() {
    return (double)getTickCount() * 1000.0 / getTickFrequency();
}

// -------------------------------------------------------
// Cap nhat logic thang may (State Machine)
// -------------------------------------------------------
bool updateElevator(Elevator& elev, bool foundResident, const Resident& res, const Mat& faceCrop) {
    std::lock_guard<std::mutex> lock(elevatorMutex);
    double now = getTimeMs();
    double elapsed = now - elev.stateTimeStart;
    bool triggered = false;

    switch (elev.state) {
        case ELEVATOR_IDLE:
            elev.currentDoorOpenWidth = 0;
            elev.currentVisualFloor = elev.currentFloor;
            if (foundResident) {
                elev.targetFloor = res.targetFloor;
                elev.passengerName = res.name;
                elev.passengerApartment = res.apartment;
                if (!faceCrop.empty()) {
                    elev.passengerFace = faceCrop.clone();
                } else {
                    elev.passengerFace = Mat();
                }
                
                elev.state = ELEVATOR_DOOR_OPENING_BEFORE;
                elev.stateTimeStart = now;
                triggered = true;
            }
            break;

        case ELEVATOR_DOOR_OPENING_BEFORE:
            elev.currentDoorOpenWidth = (int)((elapsed / 1000.0) * elev.maxDoorWidth);
            if (elev.currentDoorOpenWidth >= elev.maxDoorWidth) {
                elev.currentDoorOpenWidth = elev.maxDoorWidth;
                elev.state = ELEVATOR_BOARDING;
                elev.stateTimeStart = now;
            }
            break;

        case ELEVATOR_BOARDING:
            elev.currentDoorOpenWidth = elev.maxDoorWidth;
            if (elapsed >= 2000.0) {
                elev.state = ELEVATOR_DOOR_CLOSING_BEFORE;
                elev.stateTimeStart = now;
            }
            break;

        case ELEVATOR_DOOR_CLOSING_BEFORE:
            elev.currentDoorOpenWidth = elev.maxDoorWidth - (int)((elapsed / 1000.0) * elev.maxDoorWidth);
            if (elev.currentDoorOpenWidth <= 0) {
                elev.currentDoorOpenWidth = 0;
                if (elev.currentFloor == elev.targetFloor) {
                    elev.state = ELEVATOR_ARRIVED;
                } else {
                    elev.state = ELEVATOR_MOVING;
                }
                elev.stateTimeStart = now;
            }
            break;

        case ELEVATOR_MOVING: {
            elev.currentDoorOpenWidth = 0;
            double travelTimePerFloor = 1500.0;
            int floorDiff = abs(elev.targetFloor - elev.currentFloor);
            double totalTravelTime = floorDiff * travelTimePerFloor;
            
            double progress = elapsed / totalTravelTime;
            if (progress >= 1.0) {
                progress = 1.0;
                elev.currentVisualFloor = elev.targetFloor;
                elev.currentFloor = elev.targetFloor;
                elev.state = ELEVATOR_ARRIVED;
                elev.stateTimeStart = now;
            } else {
                elev.currentVisualFloor = elev.currentFloor + progress * (elev.targetFloor - elev.currentFloor);
            }
            break;
        }

        case ELEVATOR_ARRIVED:
            if (elapsed <= 1000.0) {
                elev.currentDoorOpenWidth = (int)((elapsed / 1000.0) * elev.maxDoorWidth);
            } else {
                elev.currentDoorOpenWidth = elev.maxDoorWidth;
            }
            
            if (elapsed >= 3000.0) {
                elev.state = ELEVATOR_DOOR_CLOSING_AFTER;
                elev.stateTimeStart = now;
            }
            break;

        case ELEVATOR_DOOR_CLOSING_AFTER:
            elev.currentDoorOpenWidth = elev.maxDoorWidth - (int)((elapsed / 1000.0) * elev.maxDoorWidth);
            if (elev.currentDoorOpenWidth <= 0) {
                elev.currentDoorOpenWidth = 0;
                elev.state = ELEVATOR_IDLE;
                elev.passengerName = "";
                elev.passengerApartment = "";
                elev.passengerFace = Mat();
                elev.stateTimeStart = now;
            }
            break;
    }
    return triggered;
}

// -------------------------------------------------------
// Ve va hien thi Giao dien Thang may
// -------------------------------------------------------
void drawElevatorUI(const Elevator& elev) {
    Mat ui = Mat::zeros(700, 500, CV_8UC3);
    
    for (int y = 0; y < ui.rows; y++) {
        double rFactor = (double)y / ui.rows;
        uchar b = (uchar)(30 + rFactor * 15);
        uchar g = (uchar)(24 + rFactor * 10);
        uchar r = (uchar)(20 + rFactor * 5);
        ui.row(y).setTo(Scalar(b, g, r));
    }

    rectangle(ui, Rect(10, 10, 480, 680), Scalar(80, 80, 80), 2);
    putText(ui, "SMART ELEVATOR CONTROLLER", Point(90, 45), FONT_HERSHEY_DUPLEX, 0.7, Scalar(0, 220, 255), 2);
    line(ui, Point(20, 60), Point(480, 60), Scalar(100, 100, 100), 1);

    rectangle(ui, Rect(150, 80, 200, 100), Scalar(30, 30, 30), FILLED);
    rectangle(ui, Rect(150, 80, 200, 100), Scalar(100, 100, 100), 2);
    
    int displayFloor = (int)(elev.currentVisualFloor + 0.5);
    string floorStr = (displayFloor < 10) ? "0" + to_string(displayFloor) : to_string(displayFloor);
    Scalar ledColor = (elev.state == ELEVATOR_MOVING) ? Scalar(0, 140, 255) : Scalar(0, 230, 115);
    putText(ui, floorStr, Point(200, 155), FONT_HERSHEY_DUPLEX, 2.0, ledColor, 4);

    double now = getTimeMs();
    bool blink = ((int)(now / 400) % 2 == 0);
    
    if (elev.state == ELEVATOR_MOVING) {
        if (elev.targetFloor > elev.currentFloor) {
            vector<Point> pts = {Point(310, 145), Point(330, 145), Point(320, 105)};
            if (!blink) fillPoly(ui, pts, ledColor);
        } else if (elev.targetFloor < elev.currentFloor) {
            vector<Point> pts = {Point(310, 105), Point(330, 105), Point(320, 145)};
            if (!blink) fillPoly(ui, pts, ledColor);
        }
    }

    rectangle(ui, Rect(30, 200, 440, 120), Scalar(45, 40, 35), FILLED);
    rectangle(ui, Rect(30, 200, 440, 120), Scalar(80, 80, 80), 1);
    
    if (elev.state == ELEVATOR_IDLE) {
        putText(ui, "TRANG THAI: CHO NHAN DIEN...", Point(50, 240), FONT_HERSHEY_DUPLEX, 0.55, Scalar(200, 200, 200), 1);
        putText(ui, "Vui long dung truoc camera quet khuon mat", Point(50, 270), FONT_HERSHEY_DUPLEX, 0.5, Scalar(150, 150, 150), 1);
        putText(ui, "de thang may tu dong dua len tang.", Point(50, 290), FONT_HERSHEY_DUPLEX, 0.5, Scalar(150, 150, 150), 1);
    } else {
        string stateStr = "";
        Scalar statusColor = Scalar(255, 255, 255);
        if (elev.state == ELEVATOR_DOOR_OPENING_BEFORE || elev.state == ELEVATOR_DOOR_CLOSING_BEFORE) {
            stateStr = "CUA DANG DI CHUYEN...";
            statusColor = Scalar(0, 255, 255);
        } else if (elev.state == ELEVATOR_BOARDING) {
            stateStr = "MOI CU DAN BUOC VAO CABIN";
            statusColor = Scalar(0, 255, 0);
        } else if (elev.state == ELEVATOR_MOVING) {
            stateStr = "DANG DI CHUYEN...";
            statusColor = Scalar(0, 140, 255);
        } else if (elev.state == ELEVATOR_ARRIVED) {
            stateStr = "DA DEN TANG " + to_string(elev.targetFloor) + " - MOI BUOC RA!";
            statusColor = Scalar(0, 255, 0);
        } else if (elev.state == ELEVATOR_DOOR_CLOSING_AFTER) {
            stateStr = "QUET MOI...";
            statusColor = Scalar(200, 200, 200);
        }

        string cleanName = removeVietnameseAccents(elev.passengerName);
        putText(ui, "CU DAN: " + cleanName, Point(45, 230), FONT_HERSHEY_DUPLEX, 0.55, Scalar(0, 220, 255), 1);
        putText(ui, "APARTMENT: " + elev.passengerApartment + " | TANG DICH: " + to_string(elev.targetFloor), Point(45, 255), FONT_HERSHEY_DUPLEX, 0.55, Scalar(255, 255, 255), 1);
        putText(ui, "TRANG THAI: " + stateStr, Point(45, 290), FONT_HERSHEY_DUPLEX, 0.55, statusColor, 1);
    }

    int shaftX = 55;
    int shaftYStart = 370;
    int shaftYEnd = 650;
    int shaftHeight = shaftYEnd - shaftYStart;
    
    line(ui, Point(shaftX, shaftYStart), Point(shaftX, shaftYEnd), Scalar(100, 100, 100), 2);
    for (int i = 1; i <= elev.totalFloors; i++) {
        double ratio = (double)(i - 1) / (elev.totalFloors - 1);
        int tickY = shaftYEnd - (int)(ratio * shaftHeight);
        line(ui, Point(shaftX - 8, tickY), Point(shaftX + 8, tickY), Scalar(150, 150, 150), 1);
        putText(ui, to_string(i), Point(shaftX - 25, tickY + 5), FONT_HERSHEY_DUPLEX, 0.45, Scalar(180, 180, 180), 1);
    }
    
    double visualRatio = (elev.currentVisualFloor - 1.0) / (elev.totalFloors - 1.0);
    int cabinY = shaftYEnd - (int)(visualRatio * shaftHeight);
    circle(ui, Point(shaftX, cabinY), 8, Scalar(0, 140, 255), FILLED);
    circle(ui, Point(shaftX, cabinY), 10, Scalar(255, 255, 255), 1);

    int cabX = 180;
    int cabY = 350;
    int cabW = 240;
    int cabH = 300;
    
    rectangle(ui, Rect(cabX, cabY, cabW, cabH), Scalar(20, 20, 20), FILLED);
    line(ui, Point(cabX, cabY), Point(cabX + 40, cabY + 40), Scalar(60, 60, 60), 1);
    line(ui, Point(cabX + cabW, cabY), Point(cabX + cabW - 40, cabY + 40), Scalar(60, 60, 60), 1);
    rectangle(ui, Rect(cabX + 40, cabY + 40, cabW - 80, cabH - 80), Scalar(30, 30, 30), FILLED);
    rectangle(ui, Rect(cabX + 40, cabY + 40, cabW - 80, cabH - 80), Scalar(55, 55, 55), 1);

    if (!elev.passengerFace.empty()) {
        Mat faceResized;
        resize(elev.passengerFace, faceResized, Size(100, 120));
        
        int faceX = cabX + (cabW - faceResized.cols) / 2;
        int faceY = cabY + 60;
        
        if (faceX > 0 && faceY > 0 && faceX + faceResized.cols < ui.cols && faceY + faceResized.rows < ui.rows) {
            faceResized.copyTo(ui(Rect(faceX, faceY, faceResized.cols, faceResized.rows)));
            rectangle(ui, Rect(faceX, faceY, faceResized.cols, faceResized.rows), Scalar(0, 220, 255), 1);
            
            line(ui, Point(faceX - 10, faceY + faceResized.rows + 30), Point(faceX + faceResized.cols + 10, faceY + faceResized.rows + 30), Scalar(0, 220, 255), 2);
            line(ui, Point(faceX, faceY + faceResized.rows), Point(faceX - 10, faceY + faceResized.rows + 30), Scalar(0, 220, 255), 2);
            line(ui, Point(faceX + faceResized.cols, faceY + faceResized.rows), Point(faceX + faceResized.cols + 10, faceY + faceResized.rows + 30), Scalar(0, 220, 255), 2);
        }
    }

    int doorW = cabW / 2 - elev.currentDoorOpenWidth;
    if (doorW > 0) {
        rectangle(ui, Rect(cabX, cabY, doorW, cabH), Scalar(120, 125, 130), FILLED);
        rectangle(ui, Rect(cabX + 2, cabY + 2, doorW - 4, cabH - 4), Scalar(80, 85, 90), FILLED);
        line(ui, Point(cabX + doorW - 5, cabY + 50), Point(cabX + doorW - 5, cabY + cabH - 50), Scalar(180, 185, 190), 2);

        int rightDoorX = cabX + cabW / 2 + elev.currentDoorOpenWidth;
        rectangle(ui, Rect(rightDoorX, cabY, doorW, cabH), Scalar(120, 125, 130), FILLED);
        rectangle(ui, Rect(rightDoorX + 2, cabY + 2, doorW - 4, cabH - 4), Scalar(80, 85, 90), FILLED);
        line(ui, Point(rightDoorX + 5, cabY + 50), Point(rightDoorX + 5, cabY + cabH - 50), Scalar(180, 185, 190), 2);
    }
    
    rectangle(ui, Rect(cabX, cabY, cabW, cabH), Scalar(150, 150, 150), 3);

    string windowName = "Giao dien Thang may UI";
    namedWindow(windowName, WINDOW_AUTOSIZE);
    imshow(windowName, ui);
}

// -------------------------------------------------------
// MAIN
// -------------------------------------------------------
int main() {
    // Cau hinh Windows Console sang UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 1. CONG COM
    string portName = "\\\\.\\COM3";
    HANDLE hSerial = initSerialPort(portName);
    bool useSerial = true;
    if (hSerial == INVALID_HANDLE_VALUE) {
        cout << "\n[CANH BAO] Khong the ket noi cong COM3. Chay o che do DEMO." << endl;
        useSerial = false;
    } else {
        cout << "[COM] Da ket noi thanh cong voi cong " << portName << endl;
    }

    // 2. HAAR CASCADE - PHAT HIEN KHUON MAT
    CascadeClassifier faceCascade;
    if (!faceCascade.load("haarcascade_frontalface_alt.xml")) {
        cerr << "Loi: Khong tim thay haarcascade_frontalface_alt.xml!" << endl;
        if (useSerial) CloseHandle(hSerial);
        return -1;
    }

    // 3. CO SO DU LIEU SQLITE & SYNC MANAGER
    DatabaseManager db;
    db.setFaceImagesDir("faces");
    if (!db.openDatabase("elevator_system.db") || !db.createTables()) {
        cerr << "Loi: Khong the khoi tao co so du lieu!" << endl;
        if (useSerial) CloseHandle(hSerial);
        return -1;
    }

    SyncManager syncManager("localhost", 3000, "DEV_SECRET_KEY_123");
    syncManager.syncResidentsFromBackend(db);

    if (db.isEmpty()) {
        cout << "[DATABASE] Co so du lieu dang trong. Dang nap cu dan mau..." << endl;
        db.addResident("Tuan A", "P502", 5);
        db.addResident("Tuan B", "P804", 8);
        db.addResident("Tuan C", "P1005", 10);
    }
    db.printAllResidents();

    // 4. MO WEBCAM
    VideoCapture cap;
    bool cameraOpened = false;
    int tryIndices[] = {0, 1, 2};
    int tryBackends[] = {CAP_DSHOW, CAP_ANY};

    cout << "\n[HE THONG] Dang khoi tao camera..." << endl;
    for (int idx : tryIndices) {
        for (int backend : tryBackends) {
            string bName = (backend == CAP_DSHOW) ? "CAP_DSHOW" : "CAP_ANY";
            cout << "Dang thu mo camera index " << idx << " voi backend " << bName << "..." << endl;
            (backend == CAP_ANY) ? cap.open(idx) : cap.open(idx, backend);
            if (cap.isOpened()) {
                cout << "-> Mo CAMERA thanh cong: index " << idx << " / " << bName << endl;
                cameraOpened = true; break;
            }
        }
        if (cameraOpened) break;
    }

    if (!cameraOpened) {
        cerr << "\n[LOI] Khong the mo bat ky webcam nao!" << endl;
        cerr << "  1. Tat cac ung dung dang dung camera (Zoom, Teams, Chrome, v.v.)" << endl;
        cerr << "  2. Kiem tra Windows Settings -> Privacy -> Camera." << endl;
        if (useSerial) CloseHandle(hSerial);
        return -1;
    }

    // 5. KHOI DONG HTTP SERVER NHUNG (DEVICE SERVER)
    Elevator elevator;
    DeviceServer deviceServer(8080);
    deviceServer.start(db, [&elevator]() {
        std::lock_guard<std::mutex> lock(elevatorMutex);
        string stateStr = "";
        switch (elevator.state) {
            case ELEVATOR_IDLE: stateStr = "IDLE"; break;
            case ELEVATOR_DOOR_OPENING_BEFORE: stateStr = "DOOR_OPENING_BEFORE"; break;
            case ELEVATOR_BOARDING: stateStr = "BOARDING"; break;
            case ELEVATOR_DOOR_CLOSING_BEFORE: stateStr = "DOOR_CLOSING_BEFORE"; break;
            case ELEVATOR_MOVING: stateStr = "MOVING"; break;
            case ELEVATOR_ARRIVED: stateStr = "ARRIVED"; break;
            case ELEVATOR_DOOR_CLOSING_AFTER: stateStr = "DOOR_CLOSING_AFTER"; break;
            default: stateStr = "UNKNOWN"; break;
        }
        return ElevatorStatusSnapshot{ stateStr, elevator.currentFloor, elevator.targetFloor };
    });

    // 6. NAP / HUAN LUYEN MO HINH NHAN DIEN KHUON MAT (LBPH)
    Ptr<LBPHFaceRecognizer> recognizer;

    if (isModelUpToDate(MODEL_FILE_PATH, "faces")) {
        cout << "\n[NHAN DIEN] Mo hinh LBPH con hieu luc. Dang tai tu file cache..." << endl;
        recognizer = loadRecognizer(MODEL_FILE_PATH);
        if (!recognizer) {
            cout << "[NHAN DIEN] File mo hinh bi loi. Tien hanh huan luyen lai..." << endl;
            recognizer = trainRecognizer(db, MODEL_FILE_PATH);
        }
    } else {
        if (db.hasAnyFaceData()) {
            cout << "\n[NHAN DIEN] Co du lieu anh moi. Dang huan luyen lai mo hinh AI..." << endl;
            recognizer = trainRecognizer(db, MODEL_FILE_PATH);
        } else {
            cout << "\n[NHAN DIEN] Chua co du lieu khuon mat. He thong san sang cho dang ky..." << endl;
        }
    }

    cout << "\n[HE THONG] San sang! Dang nhap va quet camera thang may (Remote HTTP Server Port 8080)..." << endl;
    cout << "[HE THONG] Bam [Q] tren cua so camera de thoat.\n" << endl;

    // 7. VONG LAP CHINH - PHAT HIEN & NHAN DIEN KHUON MAT (RECOGNIZING / ENROLLING)
    Mat frame, gray;
    vector<Rect> faces;
    vector<Resident> residentsList = db.getAllResidents();
    double lastAlertTime = 0.0;

    while (true) {
        if (deviceMode.load() == DeviceMode::ENROLLING) {
            int targetId = pendingResidentId.load();
            Resident targetResident;
            bool foundTarget = false;
            for (const auto& r : residentsList) {
                if (r.id == targetId) {
                    targetResident = r;
                    foundTarget = true;
                    break;
                }
            }

            if (foundTarget) {
                cout << "\n[MAIN LOOP] Chuyen sang che do ENROLLING cho cư dan ID: " << targetId << " (" << targetResident.name << ")" << endl;
                enrollFace(cap, faceCascade, db, targetResident);
                
                cout << "\n[NHAN DIEN] Hoan thanh enrollment. Dang huan luyen lai mo hinh..." << endl;
                recognizer = trainRecognizer(db, MODEL_FILE_PATH);
                residentsList = db.getAllResidents();
            } else {
                cerr << "\n[MAIN LOOP LOI] Khong tim thay resident voi ID: " << targetId << " trong danh sach!" << endl;
            }

            pendingResidentId = -1;
            deviceMode = DeviceMode::RECOGNIZING;
            cout << "[MAIN LOOP] Da quay lai che do RECOGNIZING." << endl;
            continue;
        }

        cap >> frame;
        if (frame.empty()) break;

        cvtColor(frame, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, gray);
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, Size(80, 80));

        bool foundResident = false;
        int  bestFloor     = -1;
        string bestName    = "";
        
        Resident bestResident;
        Mat bestFaceCrop;

        for (size_t i = 0; i < faces.size(); i++) {
            Mat faceROI = gray(faces[i]);
            Mat resized;
            resize(faceROI, resized, Size(FACE_IMG_SIZE, FACE_IMG_SIZE));

            int    predictedLabel = -1;
            double confidence     = 0.0;
            bool   isKnown        = false;

            if (recognizer) {
                recognizer->predict(resized, predictedLabel, confidence);
                isKnown = (predictedLabel != -1 && confidence < RECOGNITION_THRESHOLD);
            }

            Scalar boxColor   = isKnown ? Scalar(0, 255, 0) : Scalar(0, 0, 255);
            string label;

            if (isKnown) {
                string resName = db.getNameById(predictedLabel);
                int    floor   = db.getFloorByResidentId(predictedLabel);
                string cleanName = removeVietnameseAccents(resName);
                label = "Cu dan: " + cleanName + " | Tang: " + to_string(floor)
                        + " [" + to_string((int)confidence) + "]";

                if (!foundResident) {
                    foundResident = true;
                    bestFloor     = floor;
                    bestName      = resName;
                    
                    Rect faceRect = faces[i] & Rect(0, 0, frame.cols, frame.rows);
                    bestFaceCrop = frame(faceRect).clone();
                    
                    for (const auto& r : residentsList) {
                        if (r.id == predictedLabel) {
                            bestResident = r;
                            break;
                        }
                    }
                }
            } else {
                label = "Nguoi la! Khong co quyen [" + to_string((int)confidence) + "]";
                
                double currentTime = getTimeMs();
                if (currentTime - lastAlertTime > 5000.0) {
                    syncManager.sendAlert("Phat hien Nguoi la khong co quyen truy cap thang may!");
                    lastAlertTime = currentTime;
                }
            }

            rectangle(frame, faces[i], boxColor, 2);
            putText(frame, label,
                    Point(faces[i].x, faces[i].y - 10),
                    FONT_HERSHEY_SIMPLEX, 0.55, boxColor, 2);
        }

        bool triggerSignal = (foundResident && elevator.state == ELEVATOR_IDLE);
        
        if (updateElevator(elevator, triggerSignal, bestResident, bestFaceCrop)) {
            cout << "[AI] Nhan dien: " << elevator.passengerName
                 << " -> Kich hoat tang " << elevator.targetFloor << "!" << endl;

            syncManager.sendAccessLog(bestResident.id, elevator.targetFloor);

            char floorChar = '0' + elevator.targetFloor;
            if (useSerial) {
                sendData(hSerial, floorChar);
            } else {
                cout << "[DEMO] Gui lenh gia lap tang '" << floorChar << "' qua COM." << endl;
            }
        }

        drawElevatorUI(elevator);

        string title = "He thong Camera AI Thang may  |  'Q':Thoat";
        namedWindow(title, WINDOW_AUTOSIZE);
        imshow(title, frame);

        int key = waitKey(30) & 0xFF;
        if (key == 'q' || key == 'Q') break;
    }

    deviceServer.stop();
    cap.release();
    destroyAllWindows();
    if (useSerial) CloseHandle(hSerial);
    cout << "Da dong ket noi he thong." << endl;
    return 0;
}