#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <string>
#include <vector>
#include "database.h"

class SyncManager {
private:
    std::string backendHost;
    int backendPort;
    std::string apiKey;

public:
    SyncManager(const std::string& host = "localhost", int port = 3000, const std::string& key = "DEV_SECRET_KEY_123");

    // 1. Dong bo danh sach cu dan tu Backend ve SQLite cuc bo
    bool syncResidentsFromBackend(DatabaseManager& db);

    // 2. Gui nhat ky nhan dien cu dan thanh cong ve Backend
    bool sendAccessLog(int residentId, int floor);

    // 3. Gui canh bao nhan dien Nguoi la ve Backend
    bool sendAlert(const std::string& reason);
};

#endif // SYNC_MANAGER_H
