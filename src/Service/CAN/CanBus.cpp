#include "Service/CAN/CanBus.h"

#include "Service/Log.h"

namespace {

constexpr int kCanTxPin = 17;
constexpr int kCanRxPin = 18;
constexpr int kCanBitrateKbps = 500;

} // namespace

CanBus& CanBus::instance() {
    static CanBus bus;
    return bus;
}

bool CanBus::begin() {
    if (ready_) return true;

    bus_.reset(new canfw::Esp32TwaiBus(kCanTxPin, kCanRxPin, canfw::CanBitrate::K500));
    network_.reset(new canfw::CanNetwork(*bus_));

    ready_ = bus_->begin();
    if (!ready_) {
        return setError("CAN bus begin failed");
    }

    Log::D("[CAN] started: TX=%d RX=%d bitrate=%d",
           kCanTxPin,
           kCanRxPin,
           kCanBitrateKbps);
    return true;
}

canfw::Esp32TwaiBus& CanBus::bus() {
    return *bus_;
}

canfw::CanNetwork& CanBus::network() {
    return *network_;
}

bool CanBus::setError(const String& message) {
    lastError_ = message;
    Log::E("[CAN] %s", message.c_str());
    return false;
}
