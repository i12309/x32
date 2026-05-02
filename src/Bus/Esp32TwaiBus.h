#pragma once

#include "Bus/ICanBus.h"

// Esp32TwaiBus связывает общий ICanBus с TWAI-драйвером ESP32.
// Класс не знает о DeviceRegistry, MKS, STM и командах станка: только pins,
// bitrate и сырые CAN-кадры.
class Esp32TwaiBus : public ICanBus {
public:
    // Запускает TWAI. При ошибке оставляет статус Error, но не перезагружает
    // устройство: boot должен сам решить, блокировать ли работу без CAN.
    bool begin(const CanBusConfig& cfg) override;

    // Отправляет кадр через TWAI.
    bool send(const CanFrame& frame) override;

    // Принимает кадр из TWAI без ожидания.
    bool receive(CanFrame& frame) override;

    // Сейчас дополнительная обработка не нужна, метод оставлен для единого loop.
    void process() override;

    // Возвращает последний известный статус шины.
    CanBusStatus status() const override { return status_; }

private:
    CanBusStatus status_ = CanBusStatus::NotStarted;
};
