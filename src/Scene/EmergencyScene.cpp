#include "Scene/EmergencyScene.h"

EmergencyScene::EmergencyScene(DeviceRegistry& devices) : BaseScene(devices) {}

SceneTaskId EmergencyScene::stopAll(uint32_t timeoutMs) {
    DeviceTaskId ids[SceneTask::MaxDeviceTasks] = {};
    uint8_t count = 0;

    for (uint8_t i = 0; i < devices_.count() && count < SceneTask::MaxDeviceTasks; ++i) {
        const DeviceNode* node = devices_.deviceAt(i);
        if (node == nullptr) continue;
        ids[count++] = sendToDevice(node->name, DeviceCommand::Stop, DeviceParams(), timeoutMs);
    }
    return trackMany(ids, count);
}

SceneTaskId EmergencyScene::resetErrors(uint32_t timeoutMs) {
    DeviceTaskId ids[SceneTask::MaxDeviceTasks] = {};
    uint8_t count = 0;

    for (uint8_t i = 0; i < devices_.count() && count < SceneTask::MaxDeviceTasks; ++i) {
        const DeviceNode* node = devices_.deviceAt(i);
        if (node == nullptr) continue;
        ids[count++] = sendToDevice(node->name, DeviceCommand::ResetError, DeviceParams(), timeoutMs);
    }
    return trackMany(ids, count);
}
