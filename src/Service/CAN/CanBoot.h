#pragma once

#include <Arduino.h>

#include "Service/CAN/CanBus.h"
#include "Service/CAN/CanHelper.h"

class CanBoot {
public:
    bool discover(CanBus& bus);
    String lastError() const { return lastError_; }

private:
    bool setError(const String& message);

    CanHelper helper_;
    String lastError_;
};
