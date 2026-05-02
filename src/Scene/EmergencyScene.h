#pragma once

#include "Scene/Base.h"

class EmergencyScene : public BaseScene {
public:
    explicit EmergencyScene(DeviceRegistry& devices);

    SceneTaskId stopAll(uint32_t timeoutMs = 0);
    SceneTaskId resetErrors(uint32_t timeoutMs = 0);
};
