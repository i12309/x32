#include "Service/CAN/CanHelper.h"

#include "Core.h"
#include "core/Helper.h"
#include "Service/Log.h"

CanHelper& CanHelper::instance() {
    static CanHelper helper;
    return helper;
}

bool CanHelper::nodeAddress(const char* nodeName, uint16_t& out) {
    if (nodeName == nullptr || nodeName[0] == '\0') {
        return setError("Empty CAN node name");
    }

    if (!Core::config.nodeAddress(nodeName, out) || !canfw::isNonZeroCanId(out)) {
        return setError(String("CAN node is not configured: ") + nodeName);
    }
    return true;
}

bool CanHelper::nodeGroup(const char* nodeName, uint16_t& out) {
    if (nodeName == nullptr || nodeName[0] == '\0') {
        return setError("Empty CAN node name");
    }

    if (!Core::config.nodeGroup(nodeName, out) || !canfw::isNonZeroCanId(out)) {
        return setError(String(nodeName) + " group is not configured");
    }
    return true;
}

bool CanHelper::feedGroup(uint16_t& out) {
    uint16_t paperGroup = 0;
    uint16_t throwGroup = 0;
    if (!nodeGroup("PAPER", paperGroup)) return false;
    if (!nodeGroup("THROW", throwGroup)) return false;
    if (paperGroup != throwGroup) {
        return setError(String("PAPER and THROW are in different CAN groups: 0x") +
                        String(paperGroup, HEX) + " vs 0x" +
                        String(throwGroup, HEX));
    }

    out = paperGroup;
    return true;
}

String CanHelper::macToString(const Mgmt::MacAddress& mac) const {
    char buffer[18];
    snprintf(buffer,
             sizeof(buffer),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac.bytes[0],
             mac.bytes[1],
             mac.bytes[2],
             mac.bytes[3],
             mac.bytes[4],
             mac.bytes[5]);
    return String(buffer);
}

bool CanHelper::allBootNodesAssigned(const std::vector<bool>& assigned) const {
    for (bool value : assigned) {
        if (!value) return false;
    }
    return true;
}

String CanHelper::missingBootNodes(const std::vector<bool>& assigned) const {
    String result;
    for (size_t i = 0; i < assigned.size() && i < Core::config.nodes.size(); ++i) {
        if (assigned[i]) continue;
        if (result.length() > 0) result += ", ";
        result += Core::config.nodes[i].name;
        result += "(MAC ";
        result += Core::config.nodes[i].mac;
        result += ")";
    }
    return result;
}

bool CanHelper::setError(const String& message) {
    lastError_ = message;
    Log::E("[CAN] %s", message.c_str());
    return false;
}
