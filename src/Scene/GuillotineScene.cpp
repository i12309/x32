#include "Scene/GuillotineScene.h"

GuillotineScene::GuillotineScene(DeviceRegistry& devices) : BaseScene(devices) {}

SceneTaskId GuillotineScene::cut(uint16_t speedMmS, uint32_t timeoutMs) {
    DeviceParams params;
    params.u.guillotineCut.cutSpeedMmS = speedMmS;
    return trackOne(sendToRole(Role::Guillotine, DeviceCommand::GuillotineCut, params, timeoutMs));
}

SceneTaskId GuillotineScene::checkReady(uint32_t timeoutMs) {
    return trackOne(sendToRole(Role::Guillotine, DeviceCommand::Check, DeviceParams(), timeoutMs));
}

SceneTaskId GuillotineScene::stop(uint32_t timeoutMs) {
    return trackOne(sendToRole(Role::Guillotine, DeviceCommand::Stop, DeviceParams(), timeoutMs));
}
