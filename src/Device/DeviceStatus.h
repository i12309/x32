#pragma once

#include <Arduino.h>

// DeviceTaskStatus отражает состояние одного задания на соседнем CAN-device.
// Rejected трактуется как ошибка конфигурации/протокола, а Timeout/Failed - как
// проблема связи или выполнения.
enum class DeviceTaskStatus : uint8_t {
    Unknown,
    Queued,
    Sent,
    Accepted,
    Running,
    Done,
    Failed,
    Timeout,
    Rejected
};

// DeviceStatus - срез текущего состояния соседнего device для BOOT, UI и Scene.
struct DeviceStatus {
    String deviceName;
    bool online = false;
    bool ready = false;
    bool busy = false;
    bool error = false;
    bool incompatible = false;
    String errorText;
};
