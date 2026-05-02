#pragma once

#include "Scene/Base.h"

class PaperScene : public BaseScene {
public:
    explicit PaperScene(DeviceRegistry& devices);

    SceneTaskId feed(float mm, uint16_t speedMmS, uint32_t timeoutMs = 0);
    SceneTaskId feedUntilMark(float maxMm, uint16_t speedMmS, uint32_t timeoutMs = 0);
    SceneTaskId runProfile(uint8_t profileId, uint32_t timeoutMs = 0);
    SceneTaskId stop(uint32_t timeoutMs = 0);
};
