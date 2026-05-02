#pragma once

#include "Bus/CanFrame.h"
#include "Device/DeviceNode.h"

class DeviceRegistry;

// CanProtocol - общий интерфейс протокольного адаптера поверх ICanBus.
// MKS и STM используют одну физическую шину, но кодируют команды по-разному.
class CanProtocol {
public:
    virtual ~CanProtocol() = default;

    // Имя должно совпадать с config.devices.<name>.protocol.
    virtual const char* name() const = 0;

    // Проверяет, умеет ли протокол выполнить команду напрямую.
    virtual bool supports(DeviceCommand command) const = 0;

    // Отправляет обычное runtime-задание.
    virtual bool sendTask(const DeviceNode& node, const DeviceTask& task) = 0;

    // Отправляет configure-payload или короткий запрос configure на этапе boot.
    virtual bool sendConfigure(const DeviceNode& node, const ConfigureTask& task) = 0;

    // Обрабатывает входящий кадр, если он принадлежит этому протоколу.
    virtual bool handleFrame(const CanFrame& frame, DeviceRegistry& registry) = 0;

    // Периодическая работа протокола: например polling MKS, у которого нет
    // собственного heartbeat.
    virtual void process(DeviceRegistry& registry, uint32_t nowMs) = 0;
};
