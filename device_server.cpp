#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "device_server.h"
#include "httplib.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

// External variables tu main.cpp
enum class DeviceMode { RECOGNIZING, ENROLLING };
extern std::atomic<DeviceMode> deviceMode;
extern std::atomic<int> pendingResidentId;
extern std::atomic<int> enrollProgress;

DeviceServer::DeviceServer(int p) : port(p), isRunning(false) {}

DeviceServer::~DeviceServer() {
    stop();
}

bool DeviceServer::start(DatabaseManager& db, ElevatorStatusGetter statusGetter) {
    if (isRunning) return true;

    isRunning = true;

    serverThread = std::thread([this, &db, statusGetter]() {
        httplib::Server svr;

        // Ho tro CORS cho tat ca cac request
        svr.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Headers", "Content-Type, Authorization, X-API-Key"},
            {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"}
        });

        svr.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
        });

        // 1. POST /device/start-enrollment
        svr.Post("/device/start-enrollment", [&db](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Content-Type", "application/json");
            if (deviceMode.load() == DeviceMode::ENROLLING) {
                json errJson = {
                    {"error", "Device is busy enrolling another resident"}
                };
                res.status = 409;
                res.set_content(errJson.dump(), "application/json");
                return;
            }
            int residentId = -1;
            try {
                auto bodyJson = json::parse(req.body);
                if (bodyJson.contains("residentId")) {
                    residentId = bodyJson["residentId"].get<int>();
                }
            } catch (const std::exception& e) {
                json errJson = {{"error", std::string("Invalid JSON body: ") + e.what()}};
                res.status = 400;
                res.set_content(errJson.dump(), "application/json");
                return;
            }
            if (residentId <= 0) {
                json errJson = {{"error", "Missing or invalid residentId"}};
                res.status = 400;
                res.set_content(errJson.dump(), "application/json");
                return;
            }
            // Kiem tra residentId ton tai trong SQLite
            std::string name = db.getNameById(residentId);
            if (name.empty()) {
                json errJson = {{"error", "Resident not found in device database"}};
                res.status = 404;
                res.set_content(errJson.dump(), "application/json");
                return;
            }

            // Kich hoat che do ENROLLING
            pendingResidentId = residentId;
            enrollProgress = 0;
            deviceMode = DeviceMode::ENROLLING;

            std::cout << "[HTTP SERVER] Bat dau luong enrollment cho Cư dan ID: " << residentId << " (" << name << ")" << std::endl;

            json successJson = {
                {"status", "ok"},
                {"message", "Enrollment started"},
                {"residentId", residentId},
                {"name", name}
            };
            res.status = 200;
            res.set_content(successJson.dump(), "application/json");
        });

        // 2. GET /device/status
        svr.Get("/device/status", [statusGetter](const httplib::Request&, httplib::Response& res) {
            res.set_header("Content-Type", "application/json");

            std::string modeStr = (deviceMode.load() == DeviceMode::ENROLLING) ? "ENROLLING" : "RECOGNIZING";
            ElevatorStatusSnapshot snapshot = statusGetter ? statusGetter() : ElevatorStatusSnapshot{"IDLE", 1, 1};
            json statusJson = {
                {"mode", modeStr},
                {"enrollProgress", enrollProgress.load()},
                {"elevatorState", snapshot.elevatorState},
                {"currentFloor", snapshot.currentFloor},
                {"targetFloor", snapshot.targetFloor}
            };
            res.status = 200;
            res.set_content(statusJson.dump(), "application/json");
        });
        // 3. POST /device/cancel-enrollment
        svr.Post("/device/cancel-enrollment", [](const httplib::Request&, httplib::Response& res) {
            res.set_header("Content-Type", "application/json");
            if (deviceMode.load() == DeviceMode::ENROLLING) {
                deviceMode = DeviceMode::RECOGNIZING;
                pendingResidentId = -1;
                enrollProgress = 0;
                std::cout << "[HTTP SERVER] Da nhan lenh huy dang ky (cancel-enrollment)." << std::endl;
            }
            json successJson = {
                {"status", "ok"},
                {"message", "Enrollment cancelled"}
            };
            res.status = 200;
            res.set_content(successJson.dump(), "application/json");
        });
        std::cout << "[HTTP SERVER] Dang lang nghe tai http://0.0.0.0:" << port << " ..." << std::endl;

        if (!svr.listen("0.0.0.0", port)) {
            std::cerr << "[HTTP SERVER LOI] Khong the bind port " << port 
                      << ". Server bi ngung, ung dung VÂN chay o che do RECOGNIZING!" << std::endl;
            this->isRunning = false;
        }
    });
    return true;
}
void DeviceServer::stop() {
    if (!isRunning) return;
    isRunning = false;
    // std::thread running httplib listen is blocking until app exit or forced stop
    if (serverThread.joinable()) {
        serverThread.detach(); // Detach loop thread on shutdown
    }
}
