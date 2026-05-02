#pragma once

#include <Arduino.h>

#include "Device/DeviceRegistry.h"
#include "Scene/SceneTask.h"

class BaseScene {
public:
    explicit BaseScene(DeviceRegistry& devices);
    virtual ~BaseScene() = default;

    SceneTaskStatus status(SceneTaskId taskId) const;

protected:
    DeviceTaskId sendToRole(Role role, DeviceCommand command, const DeviceParams& params = DeviceParams(), uint32_t timeoutMs = 0);
    DeviceTaskId sendToDevice(DeviceName name, DeviceCommand command, const DeviceParams& params = DeviceParams(), uint32_t timeoutMs = 0);
    SceneTaskId trackOne(DeviceTaskId taskId);
    SceneTaskId trackMany(const DeviceTaskId* taskIds, uint8_t count);
    DeviceStatus roleStatus(Role role) const;
    bool roleReady(Role role) const;
    String deviceError(Role role, const char* action) const;

    DeviceRegistry& devices_;

private:
    static constexpr uint8_t MaxSceneTasks = 16;

    static SceneTaskId nextSceneTaskId_;
    static SceneTask tasks_[MaxSceneTasks];
    static uint8_t nextSlot_;

    const SceneTask* findTask(SceneTaskId taskId) const;
    static SceneTaskStatus mapDeviceStatus(DeviceTaskStatus status);
    static SceneTaskStatus mergeStatus(SceneTaskStatus current, SceneTaskStatus next);
};
