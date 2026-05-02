#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Device/DeviceStatus.h"
#include "Device/DeviceTask.h"

// DeviceNode - одна запись из config.devices. Имя и protocol указывают на
// строки внутри Core::config.doc, поэтому этот объект их не владеет.
struct DeviceNode {
    DeviceName name = nullptr;
    uint8_t address = 0;
    const char* protocol = nullptr;
    bool required = false;
    JsonObjectConst configPayload;

    DeviceStatus status;
    DeviceTaskId activeTaskId = 0;
    DeviceTaskStatus activeTaskStatus = DeviceTaskStatus::Unknown;
    uint32_t activeTaskDeadlineMs = 0;
    uint32_t lastHeartbeatMs = 0;
    uint8_t expectedProtocolMajor = 1;
    uint8_t expectedProtocolMinor = 0;
};
