#pragma once

#include <Arduino.h>

#include "Service/CAN/CanBus.h"
#include "Service/CAN/CanHelper.h"
#include "protocols/mgmt/Mgmt.h"

class CanBoot {
public:
    static CanBoot& instance();

    // Этап 3: дождаться BootHello от всех нод и назначить им рабочие CAN ID.
    bool discover(CanBus& bus);
    // Этап 4: передать каждой назначенной ноде ее JSON-конфиг и применить его.
    bool configure(CanBus& bus);
    String lastError() const { return lastError_; }

private:
    // Передать полный JSON одной ноде: begin, chunks, end, apply.
    bool sendNodeConfig(canfw::ICanBus& rawBus,
                        Mgmt::Client& mgmt,
                        const String& nodeName,
                        uint16_t canID,
                        const String& payload);
    // Отправить ConfigBegin и дождаться ConfigAck или ConfigError.
    bool sendConfigBeginWithAck(canfw::ICanBus& rawBus,
                                Mgmt::Client& mgmt,
                                const String& nodeName,
                                uint16_t canID,
                                uint32_t size,
                                uint16_t crc);
    // Отправить один ConfigChunk и проверить ACK именно для его sequence.
    bool sendConfigChunkWithAck(canfw::ICanBus& rawBus,
                                Mgmt::Client& mgmt,
                                const String& nodeName,
                                uint16_t canID,
                                uint16_t sequence,
                                const uint8_t* data,
                                uint8_t len);
    // Завершить передачу payload и дождаться подтверждения финального чанка.
    bool sendConfigEndWithAck(canfw::ICanBus& rawBus,
                              Mgmt::Client& mgmt,
                              const String& nodeName,
                              uint16_t canID,
                              uint16_t totalChunks);
    // Попросить ноду применить уже принятый config и дождаться ApplyResult.
    bool sendApplyConfigWithResult(Mgmt::Client& mgmt,
                                   const String& nodeName,
                                   uint16_t canID);
    bool setError(const String& message);

    String lastError_;
};
