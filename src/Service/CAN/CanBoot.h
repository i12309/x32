#pragma once

#include <Arduino.h>

#include "Service/CAN/CanBus.h"
#include "Service/CAN/CanHelper.h"

class CanBoot {
public:
    static CanBoot& instance();

    bool discover();
    bool discover(CanBus& bus);
    String lastError() const { return lastError_; }

private:
    bool setError(const String& message);

    String lastError_;
};
