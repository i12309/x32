#include "Scene/CheckScene.h"

CheckScene::CheckScene(DeviceRegistry& devices) : BaseScene(devices) {}

SceneTaskId CheckScene::all(uint32_t timeoutMs) {
    DeviceTaskId ids[SceneTask::MaxDeviceTasks] = {};
    uint8_t count = 0;

    for (uint8_t i = 0; i < devices_.count() && count < SceneTask::MaxDeviceTasks; ++i) {
        const DeviceNode* node = devices_.deviceAt(i);
        if (node == nullptr || !node->required) continue;
        ids[count++] = sendToDevice(node->name, DeviceCommand::Check, DeviceParams(), timeoutMs);
    }
    return trackMany(ids, count);
}

SceneTaskId CheckScene::role(Role role, uint32_t timeoutMs) {
    return trackOne(sendToRole(role, DeviceCommand::SelfTest, DeviceParams(), timeoutMs));
}
