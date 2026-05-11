#pragma once

#include <Arduino.h>
#include <vector>

#include "protocols/mgmt/Mgmt.h"

class CanHelper {
public:
    static CanHelper& instance();

    bool nodeAddress(const char* nodeName, uint16_t& out);
    bool nodeGroup(const char* nodeName, uint16_t& out);
    bool feedGroup(uint16_t& out);
    String macToString(const Mgmt::MacAddress& mac) const;
    bool allBootNodesAssigned(const std::vector<bool>& assigned) const;
    String missingBootNodes(const std::vector<bool>& assigned) const;
    String lastError() const { return lastError_; }

private:
    bool setError(const String& message);

    String lastError_;
};
