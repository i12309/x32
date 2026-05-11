#pragma once

#include <Arduino.h>

#include "Service/CAN/CanBus.h"
#include "Service/CAN/CanHelper.h"
#include "protocols/mgmt/Mgmt.h"

class CanBoot {
public:
    static CanBoot& instance();

    // Этап 3: дождаться BootHello от всех нод и назначить им рабочие CAN ID.
    bool discover(CanBus& bus);
    // Этап 4: передать каждой назначенной ноде ее JSON-конфиг и применить его.
    bool configure(CanBus& bus);
    String lastError() const { return lastError_; }

private:
    // Передать полный JSON одной ноде: payload, apply.
    bool sendNodeConfig(Mgmt::Client& mgmt,
                        const String& nodeName,
                        uint16_t canID,
                        const String& payload);
    // Попросить ноду применить уже принятый config и дождаться ApplyResult.
    bool sendApplyConfigWithResult(Mgmt::Client& mgmt,
                                   const String& nodeName,
                                   uint16_t canID);
    bool setError(const String& message);

    String lastError_;
};
