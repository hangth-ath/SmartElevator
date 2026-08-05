#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <cassert>
#include <thread>
#include "sync_manager.h"
#include "database.h"
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

int main() {
    std::cout << "[TEST] Bắt đầu test suite cho SyncManager với Mock Backend Server..." << std::endl;

    // 1. Dựng Mock Backend HTTP Server trên port 3001
    httplib::Server mockBackend;
    std::atomic<bool> receivedAccessLog{false};
    std::atomic<bool> receivedAlert{false};

    mockBackend.Get("/api/device/residents", [](const httplib::Request& req, httplib::Response& res) {
        if (req.get_header_value("X-API-Key") != "DEV_SECRET_KEY_123") {
            res.status = 401;
            return;
        }
        json resList = json::array({
            {{"id", 100}, {"name", "Mock Resident A"}, {"apartment", "P1001"}, {"target_floor", 10}},
            {{"id", 101}, {"name", "Mock Resident B"}, {"apartment", "P1202"}, {"target_floor", 12}}
        });
        res.set_content(resList.dump(), "application/json");
        res.status = 200;
    });

    mockBackend.Post("/api/device/access-logs", [&receivedAccessLog](const httplib::Request& req, httplib::Response& res) {
        if (req.get_header_value("X-API-Key") != "DEV_SECRET_KEY_123") {
            res.status = 401;
            return;
        }
        auto j = json::parse(req.body);
        if (j.contains("residentId") && j.contains("floor")) {
            receivedAccessLog = true;
        }
        json ok = {{"status", "ok"}};
        res.set_content(ok.dump(), "application/json");
        res.status = 200;
    });

    mockBackend.Post("/api/device/alerts", [&receivedAlert](const httplib::Request& req, httplib::Response& res) {
        if (req.get_header_value("X-API-Key") != "DEV_SECRET_KEY_123") {
            res.status = 401;
            return;
        }
        auto j = json::parse(req.body);
        if (j.contains("reason")) {
            receivedAlert = true;
        }
        json ok = {{"status", "ok"}};
        res.set_content(ok.dump(), "application/json");
        res.status = 200;
    });

    std::thread serverThread([&mockBackend]() {
        mockBackend.listen("0.0.0.0", 3001);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 2. Khởi tạo DB SQLite test
    DatabaseManager db;
    db.setFaceImagesDir("faces_sync_test");
    if (!db.openDatabase("test_sync.db") || !db.createTables()) {
        std::cerr << "[TEST LOI] Không thể tạo DB test sync!" << std::endl;
        mockBackend.stop();
        serverThread.join();
        return -1;
    }

    SyncManager syncManager("localhost", 3001, "DEV_SECRET_KEY_123");

    // Test 1: Đồng bộ residents từ Mock Backend -> SQLite local
    std::cout << "[TEST 1] Sync residents from backend..." << std::endl;
    bool syncSuccess = syncManager.syncResidentsFromBackend(db);
    assert(syncSuccess);

    auto residents = db.getAllResidents();
    assert(residents.size() >= 2);
    bool foundA = false;
    for (const auto& r : residents) {
        if (r.name == "Mock Resident A") foundA = true;
    }
    assert(foundA);
    std::cout << "  -> PASSED: Synchronized new residents into local SQLite." << std::endl;

    // Test 2: Gửi Access Log
    std::cout << "[TEST 2] Send Access Log..." << std::endl;
    bool accessLogSuccess = syncManager.sendAccessLog(100, 10);
    assert(accessLogSuccess);
    assert(receivedAccessLog.load());
    std::cout << "  -> PASSED: Access log received by Mock Backend." << std::endl;

    // Test 3: Gửi Alert
    std::cout << "[TEST 3] Send Alert (Nguoi la)..." << std::endl;
    bool alertSuccess = syncManager.sendAlert("Nguoi la! Khong co quyen truy cap.");
    assert(alertSuccess);
    assert(receivedAlert.load());
    std::cout << "  -> PASSED: Alert received by Mock Backend." << std::endl;

    // Test 4: Khi Backend offline -> SyncManager không crash
    mockBackend.stop();
    if (serverThread.joinable()) serverThread.join();

    std::cout << "[TEST 4] Backend offline resilience test..." << std::endl;
    bool offlineSync = syncManager.syncResidentsFromBackend(db);
    assert(!offlineSync); // Tra ve false nhung KHONG crash app
    std::cout << "  -> PASSED: Handled offline backend gracefully without crashing." << std::endl;

    db.closeDatabase();

    std::cout << "\n=================================================" << std::endl;
    std::cout << "[TEST THANH CONG] ALL 4 SYNC TESTS PASSED 100%!" << std::endl;
    std::cout << "=================================================\n" << std::endl;

    return 0;
}
