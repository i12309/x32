#include "Service/CAN/CanBoot.h"

#include <vector>

#include "Core.h"
#include "Service/Log.h"
#include "core/CanWait.h"
#include "protocols/mgmt/Mgmt.h"

namespace {

constexpr uint32_t kAckTimeoutMs = 150;
constexpr uint32_t kCheckTimeoutMs = 300;
constexpr uint32_t kApplyTimeoutMs = 5000;
constexpr uint8_t kMaxConfigRetries = 3;
constexpr uint8_t kConfigChunkPayloadSize = 5;

struct ConfigResponse {
    bool ack = false;
    uint16_t sequence = 0;
    uint8_t errorCode = 0;
    uint16_t expectedSequence = 0;
};

struct ConfigResponseContext {
    uint16_t canID = 0;
};

// Окно discovery масштабируется от числа нод, но не становится меньше базовых 3 секунд.
uint32_t bootDiscoveryWindowMs() {
    const uint32_t perNode = kCheckTimeoutMs * 10;
    const uint32_t count = Core::config.nodes.empty()
        ? 1
        : static_cast<uint32_t>(Core::config.nodes.size());
    const uint32_t window = perNode * count;
    return window < 3000 ? 3000 : window;
}

uint16_t unpackU16Le(const uint8_t* data) {
    return static_cast<uint16_t>(uint16_t(data[0]) | (uint16_t(data[1]) << 8));
}

bool isBasicDataFrame(const canfw::CanFrame& frame) {
    return !frame.extended && !frame.remote && frame.dlc >= 1;
}

bool configResponseMatches(const canfw::CanFrame& frame, void* context) {
    ConfigResponseContext* match = static_cast<ConfigResponseContext*>(context);
    if (!match ||
        frame.id != match->canID ||
        !isBasicDataFrame(frame)) {
        return false;
    }

    const uint8_t command = frame.data[0];
    if (command == static_cast<uint8_t>(Mgmt::Command::ConfigAck)) {
        return frame.dlc >= 4;
    }
    if (command == static_cast<uint8_t>(Mgmt::Command::ConfigError)) {
        return frame.dlc >= 4;
    }
    return false;
}

bool waitForConfigResponse(canfw::ICanBus& bus,
                           uint16_t canID,
                           ConfigResponse& response,
                           uint32_t timeoutMs) {
    ConfigResponseContext context;
    context.canID = canID;

    // Mgmt::Client::waitForConfigAck() не различает timeout и ConfigError,
    // поэтому здесь ждем оба типа ответа и раскладываем их в общий результат.
    canfw::CanFrame frame;
    if (!canfw::waitForFrame(bus, frame, configResponseMatches, &context, timeoutMs)) {
        response = ConfigResponse();
        return false;
    }

    response = ConfigResponse();
    if (frame.data[0] == static_cast<uint8_t>(Mgmt::Command::ConfigAck)) {
        response.ack = frame.data[3] != 0;
        response.sequence = unpackU16Le(&frame.data[1]);
        return true;
    }

    response.ack = false;
    response.errorCode = frame.data[1];
    response.expectedSequence = unpackU16Le(&frame.data[2]);
    return true;
}

// ConfigEnd подтверждает последний реально принятый chunk, а не само число chunks.
uint16_t expectedAckSequence(Mgmt::Command command, uint16_t sequence, uint16_t totalChunks) {
    if (command == Mgmt::Command::ConfigChunk) return sequence;
    if (command == Mgmt::Command::ConfigEnd) return totalChunks == 0 ? 0 : static_cast<uint16_t>(totalChunks - 1);
    return 0;
}

} // namespace

CanBoot& CanBoot::instance() {
    static CanBoot boot;
    return boot;
}

bool CanBoot::discover(CanBus& bus) {
    if (!bus.begin()) return setError(bus.lastError());

    const size_t nodeCount = Core::config.nodes.size();
    if (nodeCount == 0) {
        return setError("CAN boot discovery failed: nodes list is empty");
    }

    Mgmt::Client mgmt(bus.network().bus());
    std::vector<bool> assigned(nodeCount, false);
    const uint32_t startedAt = millis();
    const uint32_t windowMs = bootDiscoveryWindowMs();

    Log::D("[CAN] boot discovery window: nodes=%u timeout=%u ms",
           static_cast<unsigned>(nodeCount),
           static_cast<unsigned>(windowMs));

    CanHelper& helper = CanHelper::instance();

    while (!helper.allBootNodesAssigned(assigned)) {
        const uint32_t elapsed = millis() - startedAt;
        if (elapsed >= windowMs) break;

        const uint32_t remaining = windowMs - elapsed;
        const uint32_t waitMs = remaining < kCheckTimeoutMs ? remaining : kCheckTimeoutMs;

        Mgmt::BootInfo boot;
        if (!mgmt.waitForBootHello(boot, waitMs)) {
            continue;
        }

        const String mac = helper.macToString(boot.mac);
        if (boot.protocolVersion != Mgmt::PROTOCOL_VERSION) {
            return setError(String("CAN boot discovery failed: incompatible Mgmt protocol from MAC ") +
                            mac + " version=" + String(boot.protocolVersion) +
                            " expected=" + String(Mgmt::PROTOCOL_VERSION));
        }

        String nodeName;
        uint16_t nodeCanID = 0;
        bool hasPayload = false;
        if (!Core::config.nodeBootInfoByMac(mac, nodeName, nodeCanID, hasPayload)) {
            return setError(String("CAN boot discovery failed: unknown node MAC ") + mac);
        }
        if (nodeCanID == 0 || !hasPayload) {
            return setError(String("CAN boot discovery failed: invalid node config for ") +
                            nodeName + " MAC " + mac);
        }

        size_t nodeIndex = nodeCount;
        for (size_t i = 0; i < nodeCount; ++i) {
            if (Core::config.nodes[i].name == nodeName) {
                nodeIndex = i;
                break;
            }
        }
        if (nodeIndex == nodeCount) {
            return setError(String("CAN boot discovery failed: node config disappeared for MAC ") + mac);
        }
        if (assigned[nodeIndex]) {
            continue;
        }

        if (!mgmt.sendAssignCanID(boot.mac, nodeCanID)) {
            return setError(String("CAN boot discovery failed: AssignCanID send failed for ") +
                            nodeName + " can=0x" + String(nodeCanID, HEX));
        }
        if (!mgmt.waitForAssignAck(nodeCanID, kAckTimeoutMs)) {
            return setError(String("CAN boot discovery failed: AssignAck timeout for ") +
                            nodeName + " can=0x" + String(nodeCanID, HEX) +
                            " MAC " + mac);
        }

        assigned[nodeIndex] = true;
        Log::D("[CAN] boot assigned: %s MAC=%s can=0x%s",
               nodeName.c_str(),
               mac.c_str(),
               String(nodeCanID, HEX).c_str());
    }

    if (!helper.allBootNodesAssigned(assigned)) {
        return setError(String("CAN boot discovery timeout, missing nodes: ") +
                        helper.missingBootNodes(assigned));
    }

    Log::D("[CAN] boot discovery complete");
    return true;
}

bool CanBoot::configure(CanBus& bus) {
    if (!bus.begin()) return setError(bus.lastError());

    if (Core::config.nodes.empty()) {
        return setError("CAN config transfer failed: nodes list is empty");
    }

    canfw::ICanBus& rawBus = bus.network().bus();
    Mgmt::Client mgmt(rawBus);
    for (const auto& node : Core::config.nodes) {
        if (node.canID == 0 || node.payload.length() == 0) {
            return setError(String("CAN config transfer failed: invalid node config for ") +
                            node.name + " can=0x" + String(node.canID, HEX));
        }

        if (!sendNodeConfig(rawBus, mgmt, node.name, node.canID, node.payload)) {
            return false;
        }
    }

    Log::D("[CAN] config transfer complete");
    return true;
}

bool CanBoot::sendNodeConfig(canfw::ICanBus& rawBus,
                             Mgmt::Client& mgmt,
                             const String& nodeName,
                             uint16_t canID,
                             const String& payload) {
    const uint32_t size = static_cast<uint32_t>(payload.length());
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(payload.c_str());
    // В classic CAN после command + sequence остается 5 байт полезной нагрузки.
    const uint32_t totalChunks32 = (size + kConfigChunkPayloadSize - 1) / kConfigChunkPayloadSize;

    if (size > 0x00FFFFFFUL) {
        return setError(String("CAN config transfer failed: payload too large for ") +
                        nodeName + " size=" + String(size));
    }
    if (totalChunks32 > 0xFFFFUL) {
        return setError(String("CAN config transfer failed: too many chunks for ") +
                        nodeName + " chunks=" + String(totalChunks32));
    }

    const uint16_t crc = Mgmt::crc16(bytes, size);
    const uint16_t totalChunks = static_cast<uint16_t>(totalChunks32);

    Log::D("[CAN] config transfer start: %s can=0x%s size=%u crc=0x%s chunks=%u",
           nodeName.c_str(),
           String(canID, HEX).c_str(),
           static_cast<unsigned>(size),
           String(crc, HEX).c_str(),
           static_cast<unsigned>(totalChunks));

    if (!sendConfigBeginWithAck(rawBus, mgmt, nodeName, canID, size, crc)) {
        return false;
    }

    for (uint16_t sequence = 0; sequence < totalChunks; ++sequence) {
        const uint32_t offset = static_cast<uint32_t>(sequence) * kConfigChunkPayloadSize;
        const uint32_t remaining = size - offset;
        // Последний chunk может быть короче 5 байт, поэтому dlc формируется по фактической длине.
        const uint8_t len = static_cast<uint8_t>(remaining < kConfigChunkPayloadSize
            ? remaining
            : kConfigChunkPayloadSize);

        if (!sendConfigChunkWithAck(rawBus, mgmt, nodeName, canID, sequence, &bytes[offset], len)) {
            return false;
        }
    }

    if (!sendConfigEndWithAck(rawBus, mgmt, nodeName, canID, totalChunks)) {
        return false;
    }

    if (!sendApplyConfigWithResult(mgmt, nodeName, canID)) {
        return false;
    }

    Log::D("[CAN] config applied: %s can=0x%s",
           nodeName.c_str(),
           String(canID, HEX).c_str());
    return true;
}

bool CanBoot::sendConfigBeginWithAck(canfw::ICanBus& rawBus,
                                     Mgmt::Client& mgmt,
                                     const String& nodeName,
                                     uint16_t canID,
                                     uint32_t size,
                                     uint16_t crc) {
    // Повторяем только временные сбои отправки/ожидания. ConfigError от ноды считается финальной ошибкой.
    for (uint8_t attempt = 1; attempt <= kMaxConfigRetries; ++attempt) {
        if (!mgmt.sendConfigBegin(canID, Mgmt::ConfigFormat::Json, size, crc)) {
            if (attempt < kMaxConfigRetries) continue;
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigBegin send failed");
        }

        ConfigResponse response;
        if (!waitForConfigResponse(rawBus, canID, response, kAckTimeoutMs)) {
            if (attempt < kMaxConfigRetries) continue;
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigBegin ack timeout");
        }
        if (!response.ack) {
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigBegin errorCode=" +
                            String(response.errorCode) + " expectedSequence=" +
                            String(response.expectedSequence));
        }
        if (response.sequence != expectedAckSequence(Mgmt::Command::ConfigBegin, 0, 0)) {
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigBegin unexpected ack sequence=" +
                            String(response.sequence));
        }
        return true;
    }
    return setError(String("CAN config transfer failed: ") + nodeName + " command=ConfigBegin");
}

bool CanBoot::sendConfigChunkWithAck(canfw::ICanBus& rawBus,
                                     Mgmt::Client& mgmt,
                                     const String& nodeName,
                                     uint16_t canID,
                                     uint16_t sequence,
                                     const uint8_t* data,
                                     uint8_t len) {
    // Повтор того же sequence безопасен: принимающая сторона либо подтвердит его, либо вернет ConfigError.
    for (uint8_t attempt = 1; attempt <= kMaxConfigRetries; ++attempt) {
        if (!mgmt.sendConfigChunk(canID, sequence, data, len)) {
            if (attempt < kMaxConfigRetries) continue;
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigChunk sequence=" +
                            String(sequence) + " send failed");
        }

        ConfigResponse response;
        if (!waitForConfigResponse(rawBus, canID, response, kAckTimeoutMs)) {
            if (attempt < kMaxConfigRetries) continue;
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigChunk sequence=" +
                            String(sequence) + " ack timeout");
        }
        if (!response.ack) {
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigChunk sequence=" +
                            String(sequence) + " errorCode=" +
                            String(response.errorCode) + " expectedSequence=" +
                            String(response.expectedSequence));
        }
        if (response.sequence != expectedAckSequence(Mgmt::Command::ConfigChunk, sequence, 0)) {
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigChunk sequence=" +
                            String(sequence) + " unexpected ack sequence=" +
                            String(response.sequence));
        }
        return true;
    }
    return setError(String("CAN config transfer failed: ") + nodeName + " command=ConfigChunk");
}

bool CanBoot::sendConfigEndWithAck(canfw::ICanBus& rawBus,
                                   Mgmt::Client& mgmt,
                                   const String& nodeName,
                                   uint16_t canID,
                                   uint16_t totalChunks) {
    // ConfigEnd передает количество chunks, а ACK возвращает sequence последнего принятого chunk.
    for (uint8_t attempt = 1; attempt <= kMaxConfigRetries; ++attempt) {
        if (!mgmt.sendConfigEnd(canID, totalChunks)) {
            if (attempt < kMaxConfigRetries) continue;
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigEnd send failed");
        }

        ConfigResponse response;
        if (!waitForConfigResponse(rawBus, canID, response, kAckTimeoutMs)) {
            if (attempt < kMaxConfigRetries) continue;
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigEnd ack timeout");
        }
        if (!response.ack) {
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigEnd errorCode=" +
                            String(response.errorCode) + " expectedSequence=" +
                            String(response.expectedSequence));
        }
        if (response.sequence != expectedAckSequence(Mgmt::Command::ConfigEnd, 0, totalChunks)) {
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ConfigEnd unexpected ack sequence=" +
                            String(response.sequence));
        }
        return true;
    }
    return setError(String("CAN config transfer failed: ") + nodeName + " command=ConfigEnd");
}

bool CanBoot::sendApplyConfigWithResult(Mgmt::Client& mgmt,
                                        const String& nodeName,
                                        uint16_t canID) {
    // ApplyConfig может занять больше обычного ACK: нода парсит JSON и поднимает локальные сервисы.
    for (uint8_t attempt = 1; attempt <= kMaxConfigRetries; ++attempt) {
        if (!mgmt.sendApplyConfig(canID)) {
            if (attempt < kMaxConfigRetries) continue;
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ApplyConfig send failed");
        }

        bool ok = false;
        uint8_t errorCode = 0;
        if (!mgmt.waitForApplyResult(canID, ok, errorCode, kApplyTimeoutMs)) {
            if (attempt < kMaxConfigRetries) continue;
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ApplyConfig result timeout");
        }
        if (!ok) {
            return setError(String("CAN config transfer failed: ") +
                            nodeName + " command=ApplyConfig errorCode=" +
                            String(errorCode));
        }
        return true;
    }
    return setError(String("CAN config transfer failed: ") + nodeName + " command=ApplyConfig");
}

bool CanBoot::setError(const String& message) {
    lastError_ = message;
    Log::E("[CAN] %s", message.c_str());
    return false;
}
