#pragma once

#include "Bus/ICanBus.h"
#include "Can/CanProtocol.h"

// StmCan - протокол будущих STM32-device. На первом срезе он кодирует короткие
// команды в компактные CAN-кадры и принимает heartbeat 0x700 + address.
class StmCan : public CanProtocol {
public:
    explicit StmCan(ICanBus& bus) : bus_(bus) {}

    const char* name() const override { return "stm.v1"; }
    bool supports(DeviceCommand command) const override;
    bool sendTask(const DeviceNode& node, const DeviceTask& task) override;
    bool sendConfigure(const DeviceNode& node, const ConfigureTask& task) override;
    bool handleFrame(const CanFrame& frame, DeviceRegistry& registry) override;
    void process(DeviceRegistry& registry, uint32_t nowMs) override;

private:
    ICanBus& bus_;
};
