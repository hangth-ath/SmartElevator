#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <cassert>
#include "device_server.h"
#include "database.h"
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

// Define global atomic variables for testing
enum class DeviceMode { RECOGNIZING, ENROLLING };
std::atomic<DeviceMode> deviceMode{DeviceMode::RECOGNIZING};
std::atomic<int> pendingResidentId{-1};
std::atomic<int> enrollProgress{0};

int main() {
    std::cout << "[TEST] Khoi tao test suite cho DeviceServer..." << std::endl;

    // 1. Khoi tao DB SQLite test
    DatabaseManager db;
    db.setFaceImagesDir("faces_test");
    if (!db.openDatabase("test_elevator.db") || !db.createTables()) {
        std::cerr << "[TEST LOI] Khong the khoi tao test database!" << std::endl;
        return -1;
    }

    if (db.isEmpty()) {
        db.addResident("Test Resident A", "P101", 1);
        db.addResident("Test Resident B", "P202", 2);
    }

    auto residents = db.getAllResidents();
    assert(!residents.empty());
    int validId = residents[0].id;
    std::cout << "[TEST] Resident id hop le: " << validId << " (" << residents[0].name << ")" << std::endl;

    // 2. Khoi dong DeviceServer tai port 8085
    DeviceServer server(8085);
    bool started = server.start(db, []() {
        return ElevatorStatusSnapshot{"IDLE", 1, 1};
    });
    assert(started);

    // Cho server khoi tao listen thread
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    httplib::Client cli("http://localhost:8085");
    cli.set_read_timeout(5, 0);

    // Test 1: GET /device/status (RECOGNIZING)
    {
        std::cout << "[TEST 1] GET /device/status (Default RECOGNIZING)..." << std::endl;
        auto res = cli.Get("/device/status");
        assert(res && res->status == 200);
        auto j = json::parse(res->body);
        assert(j["mode"] == "RECOGNIZING");
        assert(j["enrollProgress"] == 0);
        assert(j["elevatorState"] == "IDLE");
        std::cout << "  -> PASSED: " << res->body << std::endl;
    }

    // Test 2: POST /device/start-enrollment voi ID hop le
    {
        std::cout << "[TEST 2] POST /device/start-enrollment (Valid Resident ID)..." << std::endl;
        json reqBody = {{"residentId", validId}};
        auto res = cli.Post("/device/start-enrollment", reqBody.dump(), "application/json");
        assert(res && res->status == 200);
        assert(deviceMode.load() == DeviceMode::ENROLLING);
        assert(pendingResidentId.load() == validId);
        std::cout << "  -> PASSED: " << res->body << std::endl;
    }

    // Test 3: POST /device/start-enrollment (Khi dang ENROLLING - Mong doi 409 Conflict)
    {
        std::cout << "[TEST 3] POST /device/start-enrollment (Device Busy -> Expect 409 Conflict)..." << std::endl;
        json reqBody = {{"residentId", validId}};
        auto res = cli.Post("/device/start-enrollment", reqBody.dump(), "application/json");
        assert(res && res->status == 409);
        std::cout << "  -> PASSED: " << res->body << std::endl;
    }

    // Test 4: POST /device/cancel-enrollment
    {
        std::cout << "[TEST 4] POST /device/cancel-enrollment..." << std::endl;
        auto res = cli.Post("/device/cancel-enrollment");
        assert(res && res->status == 200);
        assert(deviceMode.load() == DeviceMode::RECOGNIZING);
        assert(pendingResidentId.load() == -1);
        std::cout << "  -> PASSED: " << res->body << std::endl;
    }

    // Test 5: POST /device/start-enrollment (Resident ID khong ton tai -> Expect 404 Not Found)
    {
        std::cout << "[TEST 5] POST /device/start-enrollment (Invalid Resident ID 9999 -> Expect 404)..." << std::endl;
        json reqBody = {{"residentId", 9999}};
        auto res = cli.Post("/device/start-enrollment", reqBody.dump(), "application/json");
        assert(res && res->status == 404);
        std::cout << "  -> PASSED: " << res->body << std::endl;
    }

    server.stop();
    db.closeDatabase();

    std::cout << "\n=================================================" << std::endl;
    std::cout << "[TEST THANH CONG] ALL 5 ENDPOINT TESTS PASSED 100%!" << std::endl;
    std::cout << "=================================================\n" << std::endl;

    return 0;
}
