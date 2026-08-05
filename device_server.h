#ifndef DEVICE_SERVER_H
#define DEVICE_SERVER_H

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include "database.h"

// Struct snapshot trang thai thang may cho endpoint /device/status
struct ElevatorStatusSnapshot {
    std::string elevatorState;
    int currentFloor;
    int targetFloor;
};

// Typedef callback de lay snapshot thang may tu main loop
typedef std::function<ElevatorStatusSnapshot()> ElevatorStatusGetter;

class DeviceServer {
private:
    int port;
    std::thread serverThread;
    std::atomic<bool> isRunning{false};

public:
    DeviceServer(int port = 8080);
    ~DeviceServer();

    // Khoi dong HTTP server tren std::thread rieng (khong chan main loop)
    bool start(DatabaseManager& db, ElevatorStatusGetter statusGetter);
    void stop();
};

#endif // DEVICE_SERVER_H
