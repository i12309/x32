#include "Scene/PaperScene.h"

PaperScene::PaperScene(DeviceRegistry& devices) : BaseScene(devices) {}

SceneTaskId PaperScene::feed(float mm, uint16_t speedMmS, uint32_t timeoutMs) {
    DeviceParams params;
    params.u.paperFeed.mm = mm;
    params.u.paperFeed.speedMmS = speedMmS;
    return trackOne(sendToRole(Role::Paper, DeviceCommand::PaperFeed, params, timeoutMs));
}

SceneTaskId PaperScene::feedUntilMark(float maxMm, uint16_t speedMmS, uint32_t timeoutMs) {
    DeviceParams params;
    params.u.paperFeed.mm = maxMm;
    params.u.paperFeed.speedMmS = speedMmS;
    return trackOne(sendToRole(Role::Paper, DeviceCommand::PaperFeedUntilMark, params, timeoutMs));
}

SceneTaskId PaperScene::runProfile(uint8_t profileId, uint32_t timeoutMs) {
    DeviceParams params;
    params.u.profileRun.profileId = profileId;
    return trackOne(sendToRole(Role::Paper, DeviceCommand::ProfileRun, params, timeoutMs));
}

SceneTaskId PaperScene::stop(uint32_t timeoutMs) {
    return trackOne(sendToRole(Role::Paper, DeviceCommand::Stop, DeviceParams(), timeoutMs));
}
