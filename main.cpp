#include <iostream>
#include <string>
#include <windows.h>       // Win32 API - dieu khien cong COM
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp> // LBPH Face Recognizer
#include "database.h"

using namespace std;
using namespace cv;
using namespace cv::face;

// -------------------------------------------------------
// Hang so cau hinh he thong
// -------------------------------------------------------
const double RECOGNITION_THRESHOLD = 80.0; // Nguong nhan dien (cang nho cang ketat)
const int    FACE_IMG_SIZE          = 100;  // Kich thuoc chuan hoa anh khuon mat
const int    ENROLLMENT_SAMPLES     = 30;   // So mau anh can chup khi dang ky

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
// Nguoi dung: bam 's' de chup mau, 'q' de thoat
// -------------------------------------------------------
void enrollFace(VideoCapture& cap, CascadeClassifier& faceCascade,
                DatabaseManager& db, const Resident& resident) {

    int sampleCount = db.getFaceImageCount(resident.id);
    cout << "\n[DANG KY] Bat dau chup khuon mat cho: " << resident.name
         << " (da co " << sampleCount << "/" << ENROLLMENT_SAMPLES << " mau)" << endl;
    cout << "[DANG KY] Bam 'S' de chup mau | Bam 'Q' de hoan thanh dang ky." << endl;

    Mat frame, gray;
    string windowName = "Dang ky khuon mat - " + resident.name;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cvtColor(frame, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, gray);

        vector<Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, Size(80, 80));

        // Ve khung va huong dan
        for (const auto& face : faces) {
            rectangle(frame, face, Scalar(255, 165, 0), 2);
        }

        string statusText = "Mau: " + to_string(sampleCount) + "/" + to_string(ENROLLMENT_SAMPLES);
        putText(frame, statusText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 200, 255), 2);
        putText(frame, "Bam [S] de chup | [Q] de hoan thanh", Point(10, 60), FONT_HERSHEY_SIMPLEX, 0.55, Scalar(255, 255, 255), 1);

        imshow(windowName, frame);

        int key = waitKey(30) & 0xFF;

        if ((key == 's' || key == 'S') && !faces.empty()) {
            // Chup anh khuon mat dau tien tim duoc
            Mat faceROI = gray(faces[0]);
            Mat resized;
            resize(faceROI, resized, Size(FACE_IMG_SIZE, FACE_IMG_SIZE));

            if (db.saveFaceImage(resident.id, resized, sampleCount)) {
                sampleCount++;
                cout << "[DANG KY] Da luu mau " << sampleCount << "/" << ENROLLMENT_SAMPLES << endl;
            }

            if (sampleCount >= ENROLLMENT_SAMPLES) {
                cout << "[DANG KY] Da du " << ENROLLMENT_SAMPLES << " mau. Hoan thanh dang ky!" << endl;
                break;
            }
        }

        if (key == 'q' || key == 'Q') {
            cout << "[DANG KY] Da thoat khi co " << sampleCount << " mau." << endl;
            break;
        }
    }

    destroyWindow(windowName);
}

// -------------------------------------------------------
// Huan luyen mo hinh LBPH tu du lieu trong DB
// -------------------------------------------------------
Ptr<LBPHFaceRecognizer> trainRecognizer(DatabaseManager& db) {
    FaceTrainingData data = db.loadAllFaceTrainingData();

    if (data.images.empty()) {
        cerr << "[NHAN DIEN] Khong co du lieu khuon mat de huan luyen!" << endl;
        return nullptr;
    }

    auto recognizer = LBPHFaceRecognizer::create();
    recognizer->train(data.images, data.labels);
    cout << "[NHAN DIEN] Da huan luyen mo hinh LBPH voi "
         << data.images.size() << " anh khuon mat." << endl;
    return recognizer;
}

// -------------------------------------------------------
// MAIN
// -------------------------------------------------------
int main() {
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

    // 3. CO SO DU LIEU SQLITE
    DatabaseManager db;
    db.setFaceImagesDir("faces");
    if (!db.openDatabase("elevator_system.db") || !db.createTables()) {
        cerr << "Loi: Khong the khoi tao co so du lieu!" << endl;
        if (useSerial) CloseHandle(hSerial);
        return -1;
    }

    // Tu dong nap du lieu mau neu DB trong
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

    // 5. CHE DO DANG KY KHUON MAT (neu chua co du lieu)
    if (!db.hasAnyFaceData()) {
        cout << "\n======================================================" << endl;
        cout << "[CHE DO DANG KY] Chua co du lieu khuon mat trong he thong." << endl;
        cout << "Ban can chup khuon mat cho tung cu dan de he thong hoat dong." << endl;
        cout << "======================================================\n" << endl;

        auto residents = db.getAllResidents();
        for (const auto& r : residents) {
            cout << "\nCo muon dang ky khuon mat cho [" << r.name << " - Tang " << r.targetFloor << "]? (y/n): ";
            char choice;
            cin >> choice;
            if (choice == 'y' || choice == 'Y') {
                enrollFace(cap, faceCascade, db, r);
            }
        }
    } else {
        // Cho phep dang ky them neu nguoi dung muon
        cout << "\n[MENU] Bam [E] + Enter de vao che do dang ky them khuon mat." << endl;
        cout << "[MENU] Bam [Enter] de bat dau quet camera ngay." << endl;
        string input;
        getline(cin, input);
        if (input == "E" || input == "e") {
            auto residents = db.getAllResidents();
            for (const auto& r : residents) {
                cout << "\nDang ky them khuon mat cho [" << r.name << "]? (y/n): ";
                char choice;
                cin >> choice;
                if (choice == 'y' || choice == 'Y') {
                    enrollFace(cap, faceCascade, db, r);
                }
            }
        }
    }

    // 6. HUAN LUYEN MO HINH NHAN DIEN KHUON MAT (LBPH)
    cout << "\n[NHAN DIEN] Dang huan luyen mo hinh AI tu co so du lieu..." << endl;
    auto recognizer = trainRecognizer(db);
    if (!recognizer) {
        cerr << "[LOI] Chua co du lieu khuon mat! Hay dang ky khuon mat truoc." << endl;
        if (useSerial) CloseHandle(hSerial);
        return -1;
    }

    // Lay anh xa label -> ten cu dan de hien thi
    FaceTrainingData trainingData = db.loadAllFaceTrainingData();

    cout << "\n[HE THONG] San sang! Dang quet camera thang may..." << endl;
    cout << "[HE THONG] Bam [Q] de thoat | Bam [E] de dang ky them khuon mat.\n" << endl;

    // 7. VONG LAP CHINH - PHAT HIEN & NHAN DIEN KHUON MAT
    Mat frame, gray;
    vector<Rect> faces;
    bool isPressed = false;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cvtColor(frame, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, gray);
        faceCascade.detectMultiScale(gray, faces, 1.1, 5, 0, Size(80, 80));

        bool foundResident = false;
        int  bestFloor     = -1;
        string bestName    = "";

        for (size_t i = 0; i < faces.size(); i++) {
            Mat faceROI = gray(faces[i]);
            Mat resized;
            resize(faceROI, resized, Size(FACE_IMG_SIZE, FACE_IMG_SIZE));

            // Chay mo hinh LBPH de du doan danh tinh
            int    predictedLabel = -1;
            double confidence     = 0.0;
            recognizer->predict(resized, predictedLabel, confidence);

            bool isKnown = (predictedLabel != -1 && confidence < RECOGNITION_THRESHOLD);

            // --- Ve khung mau sac khac nhau theo ket qua ---
            Scalar boxColor   = isKnown ? Scalar(0, 255, 0)    : Scalar(0, 0, 255);   // Xanh / Do
            string label;

            if (isKnown) {
                string resName = db.getNameById(predictedLabel);
                int    floor   = db.getFloorByResidentId(predictedLabel);
                label = "Cu dan: " + resName + " | Tang: " + to_string(floor)
                        + " [" + to_string((int)confidence) + "]";

                // Ghi nhan cu dan dau tien tim thay de bam nut
                if (!foundResident) {
                    foundResident = true;
                    bestFloor     = floor;
                    bestName      = resName;
                }
            } else {
                label = "Nguoi la! Khong co quyen [" + to_string((int)confidence) + "]";
            }

            rectangle(frame, faces[i], boxColor, 2);
            putText(frame, label,
                    Point(faces[i].x, faces[i].y - 10),
                    FONT_HERSHEY_SIMPLEX, 0.55, boxColor, 2);
        }

        // --- GUI LENH TANG THANG MAY ---
        if (foundResident && !isPressed) {
            cout << "[AI] Nhan dien: " << bestName
                 << " -> Kich hoat tang " << bestFloor << "!" << endl;

            char floorChar = '0' + bestFloor;
            if (useSerial) {
                sendData(hSerial, floorChar);
            } else {
                cout << "[DEMO] Gui lenh gia lap tang '" << floorChar << "' qua COM." << endl;
            }
            isPressed = true;

        } else if (faces.empty() || !foundResident) {
            if (isPressed) {
                isPressed = false;
                if (faces.empty()) {
                    cout << "[HE THONG] Khong co ai - San sang." << endl;
                } else {
                    cout << "[HE THONG] Chi phat hien nguoi la - Khong kich hoat." << endl;
                }
            }
        }

        // --- HIEN THI CAMERA ---
        string title = "He thong Camera AI Thang may  |  'Q':Thoat  'E':Dang ky them";
        namedWindow(title, WINDOW_AUTOSIZE);
        imshow(title, frame);

        int key = waitKey(30) & 0xFF;
        if (key == 'q' || key == 'Q') break;

        // Cho phep dang ky them khuon mat khi dang chay
        if (key == 'e' || key == 'E') {
            auto residents = db.getAllResidents();
            for (const auto& r : residents) {
                cout << "\nDang ky them cho [" << r.name << "]? (y/n): ";
                char choice;
                cin >> choice;
                if (choice == 'y' || choice == 'Y') {
                    enrollFace(cap, faceCascade, db, r);
                }
            }
            // Huan luyen lai mo hinh sau khi dang ky them
            cout << "\n[NHAN DIEN] Dang huan luyen lai mo hinh..." << endl;
            recognizer = trainRecognizer(db);
            trainingData = db.loadAllFaceTrainingData();
        }
    }

    // 8. DON DEP
    cap.release();
    destroyAllWindows();
    if (useSerial) CloseHandle(hSerial);
    cout << "Da dong ket noi he thong." << endl;
    return 0;
}