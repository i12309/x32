#pragma once

#include "Scene/Base.h"

class CheckScene : public BaseScene {
public:
    explicit CheckScene(DeviceRegistry& devices);

    SceneTaskId all(uint32_t timeoutMs = 0);
    SceneTaskId role(Role role, uint32_t timeoutMs = 0);
};
