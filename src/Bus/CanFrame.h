#pragma once

#include <Arduino.h>

// CanFrame хранит один низкоуровневый CAN-кадр без знания о протоколе device.
// Этот тип нужен как общий язык между физической TWAI-шиной, роутером и
// протоколами MKS/STM, чтобы верхние слои не зависели от esp-idf структур.
struct CanFrame {
    uint32_t id = 0;
    uint8_t size = 0;
    uint8_t data[8] = {0};
    bool extended = false;
    bool rtr = false;
};

// CanBusConfig описывает только физические параметры шины головного ESP32.
// Device-адреса, команды и heartbeat живут выше, в CanRouter и DeviceRegistry.
struct CanBusConfig {
    int txPin = 17;
    int rxPin = 18;
    uint32_t bitrate = 500000;
    bool listenOnly = false;
};

// CanBusStatus дает boot/process простой способ показать состояние шины без
// протаскивания driver/twai.h через весь проект.
enum class CanBusStatus : uint8_t {
    NotStarted,
    Ready,
    Error
};
