#include "database.h"
#include <iostream>
#include <filesystem>
#include <sstream>
namespace fs = std::filesystem;
DatabaseManager::DatabaseManager() : db(nullptr), isConnected(false), faceImagesDir("faces") {}
DatabaseManager::~DatabaseManager() {
    closeDatabase();
}
bool DatabaseManager::openDatabase(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[DATABASE LOI] Khong the mo co so du lieu: " << sqlite3_errmsg(db) << std::endl;
        isConnected = false;
        return false;
    }
    isConnected = true;
    return true;
}
void DatabaseManager::closeDatabase() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
    isConnected = false;
}
void DatabaseManager::setFaceImagesDir(const std::string& dir) {
    faceImagesDir = dir;
}
bool DatabaseManager::createTables() {
    if (!isConnected) return false;

    // Tao bang luu tru thong tin cu dan
    const char* sql = "CREATE TABLE IF NOT EXISTS residents ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "name TEXT NOT NULL UNIQUE, "
                      "apartment TEXT NOT NULL, "
                      "target_floor INTEGER NOT NULL);";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[DATABASE LOI] Khong the tao bang: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    // Tao thu muc luu anh khuon mat
    if (!fs::exists(faceImagesDir)) {
        fs::create_directories(faceImagesDir);
        std::cout << "[DATABASE] Da tao thu muc luu anh khuon mat: " << faceImagesDir << std::endl;
    }
    return true;
}
bool DatabaseManager::addResident(const std::string& name, const std::string& apartment, int targetFloor) {
    if (!isConnected) return false;
    const char* sql = "INSERT INTO residents (name, apartment, target_floor) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DATABASE LOI] Prepare insert statement that bai: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, apartment.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, targetFloor);

    bool success = true;
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "[DATABASE LOI] Them cu dan " << name << " that bai (co the trung ten): " << sqlite3_errmsg(db) << std::endl;
        success = false;
    }
    sqlite3_finalize(stmt);
    return success;
}
bool DatabaseManager::deleteResident(int id) {
    if (!isConnected) return false;
    const char* sql = "DELETE FROM residents WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DATABASE LOI] Prepare delete statement that bai: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, id);
    bool success = true;
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "[DATABASE LOI] Xoa cu dan voi ID " << id << " that bai: " << sqlite3_errmsg(db) << std::endl;
        success = false;
    }
    sqlite3_finalize(stmt);
    return success;
}
int DatabaseManager::getFloorByName(const std::string& name) {
    if (!isConnected) return -1;
    const char* sql = "SELECT target_floor FROM residents WHERE name = ?;";
    sqlite3_stmt* stmt;
    int floor = -1;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DATABASE LOI] Prepare select statement that bai: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        floor = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return floor;
}
int DatabaseManager::getFloorByResidentId(int residentId) {
    if (!isConnected) return -1;
    const char* sql = "SELECT target_floor FROM residents WHERE id = ?;";
    sqlite3_stmt* stmt;
    int floor = -1;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DATABASE LOI] Prepare select by ID that bai: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    sqlite3_bind_int(stmt, 1, residentId);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        floor = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return floor;
}
std::string DatabaseManager::getNameById(int residentId) {
    if (!isConnected) return "";
    const char* sql = "SELECT name FROM residents WHERE id = ?;";
    sqlite3_stmt* stmt;
    std::string name = "";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DATABASE LOI] Prepare getNameById that bai: " << sqlite3_errmsg(db) << std::endl;
        return "";
    }
    sqlite3_bind_int(stmt, 1, residentId);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    sqlite3_finalize(stmt);
    return name;
}
std::vector<Resident> DatabaseManager::getAllResidents() {
    std::vector<Resident> residents;
    if (!isConnected) return residents;

    const char* sql = "SELECT id, name, apartment, target_floor FROM residents;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DATABASE LOI] Prepare select all that bai: " << sqlite3_errmsg(db) << std::endl;
        return residents;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Resident r;
        r.id = sqlite3_column_int(stmt, 0);
        r.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.apartment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        r.targetFloor = sqlite3_column_int(stmt, 3);
        residents.push_back(r);
    }
    sqlite3_finalize(stmt);
    return residents;
}
void DatabaseManager::printAllResidents() {
    auto list = getAllResidents();
    std::cout << "\n=============================================" << std::endl;
    std::cout << "DANH SACH CU DAN TRONG CO SO DU LIEU:" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    if (list.empty()) {
        std::cout << "(Chua co cu dan nao)" << std::endl;
    } else {
        for (const auto& r : list) {
            int imgCount = getFaceImageCount(r.id);
            std::cout << "ID: " << r.id
                      << " | Ten: " << r.name
                      << " | Phong: " << r.apartment
                      << " | Tang: " << r.targetFloor
                      << " | Anh khuon mat: " << imgCount << " mau" << std::endl;
        }
    }
    std::cout << "=============================================\n" << std::endl;
}
bool DatabaseManager::isEmpty() {
    if (!isConnected) return true;
    const char* sql = "SELECT COUNT(*) FROM residents;";
    sqlite3_stmt* stmt;
    int count = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return count == 0;
}
// ---- Quan ly anh khuon mat ----
bool DatabaseManager::saveFaceImage(int residentId, const cv::Mat& faceImg, int sampleIndex) {
    // Thu muc: faces/resident_<id>/
    std::string resDir = faceImagesDir + "/resident_" + std::to_string(residentId);
    if (!fs::exists(resDir)) {
        fs::create_directories(resDir);
    }
    std::string filePath = resDir + "/face_" + std::to_string(sampleIndex) + ".jpg";
    return cv::imwrite(filePath, faceImg);
}
int DatabaseManager::getFaceImageCount(int residentId) {
    std::string resDir = faceImagesDir + "/resident_" + std::to_string(residentId);
    if (!fs::exists(resDir)) return 0;
    int count = 0;
    for (const auto& entry : fs::directory_iterator(resDir)) {
        if (entry.path().extension() == ".jpg") {
            count++;
        }
    }
    return count;
}
bool DatabaseManager::hasFaceData(int residentId) {
    return getFaceImageCount(residentId) > 0;
}
bool DatabaseManager::hasAnyFaceData() {
    auto residents = getAllResidents();
    for (const auto& r : residents) {
        if (hasFaceData(r.id)) return true;
    }
    return false;
}
FaceTrainingData DatabaseManager::loadAllFaceTrainingData() {
    FaceTrainingData data;
    auto residents = getAllResidents();
    for (const auto& r : residents) {
        std::string resDir = faceImagesDir + "/resident_" + std::to_string(r.id);
        if (!fs::exists(resDir)) continue;
        data.labelToName[r.id] = r.name;

        for (const auto& entry : fs::directory_iterator(resDir)) {
            if (entry.path().extension() != ".jpg") continue;
            cv::Mat img = cv::imread(entry.path().string(), cv::IMREAD_GRAYSCALE);
            if (img.empty()) continue;

            // Chuan hoa kich thuoc anh truoc khi huan luyen
            cv::Mat resized;
            cv::resize(img, resized, cv::Size(100, 100));
            data.images.push_back(resized);
            data.labels.push_back(r.id);
        }
    }
    std::cout << "[DATABASE] Da tai " << data.images.size()
              << " anh khuon mat tu " << residents.size() << " cu dan." << std::endl;
    return data;
}
