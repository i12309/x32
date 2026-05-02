#pragma once

#include <Arduino.h>

#include "Bus/ICanBus.h"
#include "Can/CanProtocol.h"
#include "Device/DeviceRegistry.h"

// CanRouter связывает физическую шину, протоколы и DeviceRegistry.
// Он принимает сырые кадры, отдает их протоколам и обновляет registry через
// protocol-specific обработчики.
class CanRouter {
public:
    static constexpr uint8_t MaxProtocols = 4;

    CanRouter(ICanBus& bus, DeviceRegistry& registry);

    // Регистрирует протокол, имя которого должно совпадать с config protocol.
    bool bindProtocol(CanProtocol* protocol);

    // Запускает физическую шину и сохраняет результат для диагностики.
    bool begin(const CanBusConfig& config);

    // Отправка заданий из DeviceRegistry.
    bool sendTask(const DeviceNode& node, const DeviceTask& task);
    bool sendConfigure(const DeviceNode& node, const ConfigureTask& task);

    // Общий loop: прием кадров, polling протоколов и таймауты registry.
    void process();

    CanBusStatus status() const { return bus_.status(); }

private:
    CanProtocol* protocolFor(const char* name);

    ICanBus& bus_;
    DeviceRegistry& registry_;
    CanProtocol* protocols_[MaxProtocols] = {};
    uint8_t protocolCount_ = 0;
};
