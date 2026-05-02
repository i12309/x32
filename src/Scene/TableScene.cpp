#include "Scene/TableScene.h"

TableScene::TableScene(DeviceRegistry& devices) : BaseScene(devices) {}

SceneTaskId TableScene::up(uint32_t timeoutMs) {
    return trackOne(sendToRole(Role::Table, DeviceCommand::TableUp, DeviceParams(), timeoutMs));
}

SceneTaskId TableScene::down(uint32_t timeoutMs) {
    return trackOne(sendToRole(Role::Table, DeviceCommand::TableDown, DeviceParams(), timeoutMs));
}

SceneTaskId TableScene::checkPosition(uint32_t timeoutMs) {
    return trackOne(sendToRole(Role::Table, DeviceCommand::Check, DeviceParams(), timeoutMs));
}

SceneTaskId TableScene::stop(uint32_t timeoutMs) {
    return trackOne(sendToRole(Role::Table, DeviceCommand::Stop, DeviceParams(), timeoutMs));
}
