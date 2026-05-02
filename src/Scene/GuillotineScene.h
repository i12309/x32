#pragma once

#include "Scene/Base.h"

class GuillotineScene : public BaseScene {
public:
    explicit GuillotineScene(DeviceRegistry& devices);

    SceneTaskId cut(uint16_t speedMmS = 0, uint32_t timeoutMs = 0);
    SceneTaskId checkReady(uint32_t timeoutMs = 0);
    SceneTaskId stop(uint32_t timeoutMs = 0);
};
