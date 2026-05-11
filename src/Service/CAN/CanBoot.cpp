#include "Service/CAN/CanBoot.h"

#include <vector>

#include "Core.h"
#include "Service/Log.h"
#include "protocols/mgmt/Mgmt.h"

namespace {

constexpr uint32_t kAckTimeoutMs = 150;
constexpr uint32_t kCheckTimeoutMs = 300;

uint32_t bootDiscoveryWindowMs() {
    const uint32_t perNode = kCheckTimeoutMs * 10;
    const uint32_t count = Core::config.nodes.empty()
        ? 1
        : static_cast<uint32_t>(Core::config.nodes.size());
    const uint32_t window = perNode * count;
    return window < 3000 ? 3000 : window;
}

} // namespace

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

    while (!helper_.allBootNodesAssigned(assigned)) {
        const uint32_t elapsed = millis() - startedAt;
        if (elapsed >= windowMs) break;

        const uint32_t remaining = windowMs - elapsed;
        const uint32_t waitMs = remaining < kCheckTimeoutMs ? remaining : kCheckTimeoutMs;

        Mgmt::BootInfo boot;
        if (!mgmt.waitForBootHello(boot, waitMs)) {
            continue;
        }

        const String mac = helper_.macToString(boot.mac);
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

    if (!helper_.allBootNodesAssigned(assigned)) {
        return setError(String("CAN boot discovery timeout, missing nodes: ") +
                        helper_.missingBootNodes(assigned));
    }

    Log::D("[CAN] boot discovery complete");
    return true;
}

bool CanBoot::setError(const String& message) {
    lastError_ = message;
    Log::E("[CAN] %s", message.c_str());
    return false;
}
