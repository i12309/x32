#pragma once

#include "Scene/Base.h"

class TableScene : public BaseScene {
public:
    explicit TableScene(DeviceRegistry& devices);

    SceneTaskId up(uint32_t timeoutMs = 0);
    SceneTaskId down(uint32_t timeoutMs = 0);
    SceneTaskId checkPosition(uint32_t timeoutMs = 0);
    SceneTaskId stop(uint32_t timeoutMs = 0);
};
