#pragma once

#include "Bus/CanFrame.h"

// ICanBus - минимальный интерфейс физической CAN-шины.
// Его задача ограничена отправкой/приемом кадров; смысл команд device здесь
// намеренно не появляется.
class ICanBus {
public:
    virtual ~ICanBus() = default;

    // Запускает физическую шину с параметрами из config.can.
    virtual bool begin(const CanBusConfig& cfg) = 0;

    // Отправляет один CAN-кадр. Возвращает false, если драйвер не готов или
    // кадр не удалось поставить в очередь.
    virtual bool send(const CanFrame& frame) = 0;

    // Читает один входящий кадр без блокировки.
    virtual bool receive(CanFrame& frame) = 0;

    // Дает реализации шины место для короткой периодической обработки.
    virtual void process() = 0;

    // Возвращает текущее состояние физического драйвера.
    virtual CanBusStatus status() const = 0;
};
