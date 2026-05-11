#pragma once

#include <Arduino.h>
#include <memory>

#include "CanNetwork.h"
#include "backends/esp32/Esp32TwaiBus.h"

class CanBus {
public:
    bool begin();
    bool isReady() const { return ready_; }
    canfw::Esp32TwaiBus& bus();
    canfw::CanNetwork& network();
    String lastError() const { return lastError_; }

private:
    bool setError(const String& message);

    std::unique_ptr<canfw::Esp32TwaiBus> bus_;
    std::unique_ptr<canfw::CanNetwork> network_;
    bool ready_ = false;
    String lastError_;
};
