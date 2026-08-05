#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "sync_manager.h"
#include "httplib.h"
#include "json.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

static std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

SyncManager::SyncManager(const std::string& host, int port, const std::string& key)
    : backendHost(host), backendPort(port), apiKey(key) {}

bool SyncManager::syncResidentsFromBackend(DatabaseManager& db) {
    try {
        httplib::Client cli(backendHost, backendPort);
        cli.set_connection_timeout(2, 0); // 2 seconds timeout
        cli.set_read_timeout(2, 0);

        httplib::Headers headers = {
            {"X-API-Key", apiKey}
        };

        auto res = cli.Get("/api/device/residents", headers);
        if (!res || res->status != 200) {
            std::cerr << "[SYNC LOI] Khong the tai danh sach cu dan tu backend. Status: " 
                      << (res ? std::to_string(res->status) : "No Response / Offline") << std::endl;
            return false;
        }

        auto residentsJson = json::parse(res->body);
        if (!residentsJson.is_array()) return false;

        auto localResidents = db.getAllResidents();
        int addedCount = 0;

        for (const auto& item : residentsJson) {
            std::string name = item.value("name", "");
            std::string apartment = item.value("apartment", "");
            int targetFloor = item.value("target_floor", item.value("targetFloor", 1));

            if (name.empty()) continue;

            bool exists = false;
            for (const auto& lr : localResidents) {
                if (lr.name == name) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                if (db.addResident(name, apartment, targetFloor)) {
                    addedCount++;
                }
            }
        }

        if (addedCount > 0) {
            std::cout << "[SYNC] Da dong bo " << addedCount << " cu dan moi tu Backend." << std::endl;
        }
        return true;

    } catch (const std::exception& e) {
        std::cerr << "[SYNC EXCEPTION] " << e.what() << std::endl;
        return false;
    }
}

bool SyncManager::sendAccessLog(int residentId, int floor) {
    try {
        httplib::Client cli(backendHost, backendPort);
        cli.set_connection_timeout(1, 500000); // 1.5 seconds timeout

        httplib::Headers headers = {
            {"X-API-Key", apiKey}
        };

        json payload = {
            {"residentId", residentId},
            {"floor", floor},
            {"timestamp", getCurrentTimestamp()}
        };

        auto res = cli.Post("/api/device/access-logs", headers, payload.dump(), "application/json");
        if (res && res->status == 200) {
            std::cout << "[SYNC] Da gui access-log cu dan ID: " << residentId << " tang " << floor << std::endl;
            return true;
        } else {
            std::cerr << "[SYNC LOI] Gui access-log that bai. Code: " 
                      << (res ? std::to_string(res->status) : "Timeout / Offline") << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "[SYNC EXCEPTION] sendAccessLog: " << e.what() << std::endl;
        return false;
    }
}

bool SyncManager::sendAlert(const std::string& reason) {
    try {
        httplib::Client cli(backendHost, backendPort);
        cli.set_connection_timeout(1, 500000);

        httplib::Headers headers = {
            {"X-API-Key", apiKey}
        };

        json payload = {
            {"reason", reason},
            {"timestamp", getCurrentTimestamp()}
        };

        auto res = cli.Post("/api/device/alerts", headers, payload.dump(), "application/json");
        if (res && res->status == 200) {
            std::cout << "[SYNC] Da gui canh bao: " << reason << std::endl;
            return true;
        } else {
            std::cerr << "[SYNC LOI] Gui alert that bai." << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "[SYNC EXCEPTION] sendAlert: " << e.what() << std::endl;
        return false;
    }
}
