#include "Service/CAN/CanBoot.h"

#include <vector>

#include "Core.h"
#include "core/Helper.h"
#include "Service/Log.h"
#include "protocols/mgmt/Mgmt.h"

namespace {

constexpr uint32_t kAckTimeoutMs = 150;
constexpr uint32_t kCheckTimeoutMs = 300;
constexpr uint32_t kApplyTimeoutMs = 5000;
constexpr uint8_t kMaxConfigRetries = 3;

// Окно discovery масштабируется от числа нод, но не становится меньше базовых 3 секунд.
uint32_t bootDiscoveryWindowMs() {
    const uint32_t perNode = kCheckTimeoutMs * 10;
    const uint32_t count = Core::config.nodes.empty()
        ? 1
        : static_cast<uint32_t>(Core::config.nodes.size());
    const uint32_t window = perNode * count;
    return window < 3000 ? 3000 : window;
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
        if (!canfw::isNonZeroCanId(nodeCanID) || !hasPayload) {
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

    Mgmt::Client mgmt(bus.network().bus());
    for (const auto& node : Core::config.nodes) {
        if (!canfw::isNonZeroCanId(node.canID) || node.payload.length() == 0) {
            return setError(String("CAN config transfer failed: invalid node config for ") +
                            node.name + " can=0x" + String(node.canID, HEX));
        }

        if (!sendNodeConfig(mgmt, node.name, node.canID, node.payload)) {
            return false;
        }
    }

    Log::D("[CAN] config transfer complete");
    return true;
}

bool CanBoot::sendNodeConfig(Mgmt::Client& mgmt,
                             const String& nodeName,
                             uint16_t canID,
                             const String& payload) {
    const uint32_t size = static_cast<uint32_t>(payload.length());
    const uint16_t crc = Mgmt::crc16(reinterpret_cast<const uint8_t*>(payload.c_str()), size);
    const uint32_t totalChunks = (size + 4U) / 5U;

    Log::D("[CAN] config transfer start: %s can=0x%s size=%u crc=0x%s chunks=%u",
           nodeName.c_str(),
           String(canID, HEX).c_str(),
           static_cast<unsigned>(size),
           String(crc, HEX).c_str(),
           static_cast<unsigned>(totalChunks));

    Mgmt::ConfigTransferOptions options;
    options.retries = kMaxConfigRetries - 1;
    options.ackTimeoutMs = kAckTimeoutMs;

    Mgmt::ConfigTransferStatus status;
    if (!mgmt.sendJsonConfig(canID, payload.c_str(), options, &status)) {
        return setError(String("CAN config transfer failed: ") +
                        nodeName + " command=0x" +
                        String(static_cast<uint8_t>(status.failedCommand), HEX) +
                        " errorCode=" + String(static_cast<uint8_t>(status.errorCode)) +
                        " lastSequence=" + String(status.lastSequence) +
                        " expectedSequence=" + String(status.expectedSequence) +
                        " totalChunks=" + String(status.totalChunks));
    }

    if (!sendApplyConfigWithResult(mgmt, nodeName, canID)) {
        return false;
    }

    Log::D("[CAN] config applied: %s can=0x%s",
           nodeName.c_str(),
           String(canID, HEX).c_str());
    return true;
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
