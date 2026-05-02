#pragma once

#include "Bus/ICanBus.h"
#include "Can/CanProtocol.h"

// MksCan - узкий адаптер для MKS Servo57D_CAN.
// В отличие от STM32-device, MKS не умеет крупные сценарные команды, поэтому
// unsupported-команды возвращают Rejected еще до отправки.
class MksCan : public CanProtocol {
public:
    explicit MksCan(ICanBus& bus) : bus_(bus) {}

    const char* name() const override { return "mks"; }
    bool supports(DeviceCommand command) const override;
    bool sendTask(const DeviceNode& node, const DeviceTask& task) override;
    bool sendConfigure(const DeviceNode& node, const ConfigureTask& task) override;
    bool handleFrame(const CanFrame& frame, DeviceRegistry& registry) override;
    void process(DeviceRegistry& registry, uint32_t nowMs) override;

private:
    ICanBus& bus_;
    uint32_t lastPollMs_ = 0;
};
