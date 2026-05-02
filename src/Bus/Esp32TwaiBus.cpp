#include "Bus/Esp32TwaiBus.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <driver/twai.h>
#endif

#include "Service/Log.h"

namespace {

#if defined(ARDUINO_ARCH_ESP32)
twai_timing_config_t timingForBitrate(uint32_t bitrate) {
    switch (bitrate) {
        case 125000: return TWAI_TIMING_CONFIG_125KBITS();
        case 250000: return TWAI_TIMING_CONFIG_250KBITS();
        case 500000: return TWAI_TIMING_CONFIG_500KBITS();
        case 1000000: return TWAI_TIMING_CONFIG_1MBITS();
        default: return TWAI_TIMING_CONFIG_500KBITS();
    }
}
#endif

} // namespace

bool Esp32TwaiBus::begin(const CanBusConfig& cfg) {
#if defined(ARDUINO_ARCH_ESP32)
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(
        static_cast<gpio_num_t>(cfg.txPin),
        static_cast<gpio_num_t>(cfg.rxPin),
        cfg.listenOnly ? TWAI_MODE_LISTEN_ONLY : TWAI_MODE_NORMAL
    );
    twai_timing_config_t timing = timingForBitrate(cfg.bitrate);
    twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t err = twai_driver_install(&general, &timing, &filter);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        status_ = CanBusStatus::Error;
        Log::E("[CAN] TWAI driver install failed: %d", static_cast<int>(err));
        return false;
    }

    err = twai_start();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        status_ = CanBusStatus::Error;
        Log::E("[CAN] TWAI start failed: %d", static_cast<int>(err));
        return false;
    }

    status_ = CanBusStatus::Ready;
    Log::L("[CAN] TWAI started tx=%d rx=%d bitrate=%lu", cfg.txPin, cfg.rxPin, static_cast<unsigned long>(cfg.bitrate));
    return true;
#else
    (void)cfg;
    status_ = CanBusStatus::Error;
    return false;
#endif
}

bool Esp32TwaiBus::send(const CanFrame& frame) {
#if defined(ARDUINO_ARCH_ESP32)
    if (status_ != CanBusStatus::Ready) return false;

    twai_message_t message = {};
    message.identifier = frame.id;
    message.extd = frame.extended ? 1 : 0;
    message.rtr = frame.rtr ? 1 : 0;
    message.data_length_code = frame.size > 8 ? 8 : frame.size;
    for (uint8_t i = 0; i < message.data_length_code; ++i) {
        message.data[i] = frame.data[i];
    }

    return twai_transmit(&message, 0) == ESP_OK;
#else
    (void)frame;
    return false;
#endif
}

bool Esp32TwaiBus::receive(CanFrame& frame) {
#if defined(ARDUINO_ARCH_ESP32)
    if (status_ != CanBusStatus::Ready) return false;

    twai_message_t message = {};
    if (twai_receive(&message, 0) != ESP_OK) return false;

    frame.id = message.identifier;
    frame.extended = message.extd != 0;
    frame.rtr = message.rtr != 0;
    frame.size = message.data_length_code > 8 ? 8 : message.data_length_code;
    for (uint8_t i = 0; i < frame.size; ++i) {
        frame.data[i] = message.data[i];
    }
    return true;
#else
    (void)frame;
    return false;
#endif
}

void Esp32TwaiBus::process() {
    // На текущем этапе TWAI читается CanRouter::process(), а физической шине
    // не нужна отдельная периодическая работа.
}
