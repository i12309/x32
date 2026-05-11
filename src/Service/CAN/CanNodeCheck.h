#pragma once

#include <Arduino.h>

#include "Service/CAN/CanBus.h"
#include "protocols/mgmt/Mgmt.h"

class CanNodeCheck {
public:
    static CanNodeCheck& instance();

    // Этап 5: при необходимости запустить self-test, затем дождаться готовности всех нод.
    bool checkAfterBoot(CanBus& bus, bool runSelfTest);
    String lastError() const { return lastError_; }

private:
    // Запустить self-test одной ноды и дождаться SelfTestResult.
    bool selfTestNode(Mgmt::Client& mgmt, const String& nodeName, uint16_t canID);
    // Дождаться ReadyReport одной ноды после apply/self-test.
    bool waitReadyNode(Mgmt::Client& mgmt, const String& nodeName, uint16_t canID);
    bool setError(const String& message);

    String lastError_;
};
