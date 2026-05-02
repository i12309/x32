#pragma once

#include <Arduino.h>

#include "Device/DeviceTask.h"

using SceneTaskId = uint32_t;

enum class SceneTaskStatus : uint8_t {
    Unknown,
    Queued,
    Running,
    Done,
    Failed,
    Timeout,
    Rejected
};

struct SceneTask {
    static constexpr uint8_t MaxDeviceTasks = 8;

    SceneTaskId id = 0;
    DeviceTaskId deviceTasks[MaxDeviceTasks] = {};
    uint8_t count = 0;
};
