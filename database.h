#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sqlite3.h>
#include <opencv2/opencv.hpp>

// Struct bieu dien thong tin cu dan
struct Resident {
    int id;
    std::string name;
    std::string apartment;
    int targetFloor;

    // Nap chong toan tu << de xuat thong tin Resident
    friend std::ostream& operator<<(std::ostream& os, const Resident& r) {
        os << "ID: " << r.id << " | Ten: " << r.name << " | Phong: " << r.apartment << " | Tang: " << r.targetFloor;
        return os;
    }
};

// Ham template de hien thi danh sach (vector) bat ky ho tro toan tu <<
template<typename T>
void printDataList(const std::vector<T>& dataVec) {
    if (dataVec.empty()) {
        std::cout << "  (Danh sach trong)" << std::endl;
        return;
    }
    for (const auto& item : dataVec) {
        std::cout << "  - " << item << std::endl;
    }
}


// Struct chua du lieu huan luyen nhan dien khuon mat
struct FaceTrainingData {
    std::vector<cv::Mat> images;  // Danh sach anh khuon mat
    std::vector<int> labels;      // Label tuong ung voi ID cu dan
    std::map<int, std::string> labelToName; // Anh xa label -> ten cu dan
};

// Lop quan ly co so du lieu SQLite
class DatabaseManager {
private:
    sqlite3* db;
    bool isConnected;
    std::string faceImagesDir; // Thu muc luu anh khuon mat

public:
    DatabaseManager();
    ~DatabaseManager();

    // Mo va dong co so du lieu
    bool openDatabase(const std::string& dbPath);
    void closeDatabase();

    // Khoi tao bang du lieu va thu muc luu anh
    bool createTables();
    void setFaceImagesDir(const std::string& dir);

    // Thao tac voi bang residents
    bool addResident(const std::string& name, const std::string& apartment, int targetFloor);
    bool deleteResident(int id);
    int getFloorByName(const std::string& name);
    int getFloorByResidentId(int residentId);
    std::string getNameById(int residentId);
    std::vector<Resident> getAllResidents();
    void printAllResidents();
    bool isEmpty();

    // Quan ly anh khuon mat de huan luyen
    bool saveFaceImage(int residentId, const cv::Mat& faceImg, int sampleIndex);
    FaceTrainingData loadAllFaceTrainingData();
    bool hasFaceData(int residentId);
    bool hasAnyFaceData();
    int getFaceImageCount(int residentId);
    std::string getFaceDir() const { return faceImagesDir; }
};

#endif // DATABASE_H
