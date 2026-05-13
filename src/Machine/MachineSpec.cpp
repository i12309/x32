#include "Machine/MachineSpec.h"

#include "core/Helper.h"

MachineSpec MachineSpec::get(Catalog::MachineType type) {
    switch (type) {
        case Catalog::MachineType::A:
            return MachineSpec::makeA();

        case Catalog::MachineType::B:
        case Catalog::MachineType::C:
        case Catalog::MachineType::D:
        case Catalog::MachineType::E:
        case Catalog::MachineType::F:
        case Catalog::MachineType::UNKNOWN:
        default:
            return MachineSpec::makeUnknown();
    }
}

MachineSpec MachineSpec::makeUnknown() {
    MachineSpec spec;
    spec.type_ = Catalog::MachineType::UNKNOWN;
    return spec;
}

void MachineSpec::fillControllerDefaults(JsonObject root) const {
    root["nodes"].to<JsonArray>();
    root["groups"].to<JsonObject>();
}

void MachineSpec::fillDeviceDefaults(JsonObject device) const {
    (void)device;
}

MachineSpec::Report MachineSpec::validateControllerConfig(
    JsonObjectConst root,
    const std::vector<NodeInfo>& nodes
) const {
    Report report;

    if (type_ == Catalog::MachineType::UNKNOWN) {
        report.errors.push_back("[MachineSpec] unknown MachineType, spec was not selected");
        report.allowMotion = false;
        return report;
    }

    for (const NodeRequirement& req : requiredNodes_) {
        if (findNode(nodes, req.name) != nullptr) continue;

        const String msg = String("[MachineSpec] required node is missing for machine ") +
                           Catalog::machineName(type_) + ": " + req.name;
        if (req.criticalForMotion) {
            report.errors.push_back(msg);
            report.allowMotion = false;
        } else {
            report.warnings.push_back(msg);
        }
    }

    JsonObjectConst groups = root["groups"];
    if (groups.isNull() && !requiredGroups_.empty()) {
        report.errors.push_back("[MachineSpec] config.groups is missing");
        report.allowMotion = false;
        return report;
    }

    for (const GroupRequirement& req : requiredGroups_) {
        uint16_t declaredGroupId = 0;
        bool hasDeclaredGroup = false;
        JsonObjectConst groupObj = groups[req.name].as<JsonObjectConst>();
        if (!groupObj.isNull()) {
            hasDeclaredGroup = canfw::parseCanId(groupObj["canID"] | "", declaredGroupId) &&
                               canfw::isNonZeroCanId(declaredGroupId);
        }
        if (!hasDeclaredGroup) {
            const String msg = String("[MachineSpec] required group is missing or invalid: ") + req.name;
            if (req.criticalForMotion) {
                report.errors.push_back(msg);
                report.allowMotion = false;
            } else {
                report.warnings.push_back(msg);
            }
            continue;
        }

        const NodeInfo* nodeA = findNode(nodes, req.nodeA);
        const NodeInfo* nodeB = findNode(nodes, req.nodeB);
        if (nodeA == nullptr || nodeB == nullptr) {
            const String msg = String("[MachineSpec] required group ") + req.name +
                               " needs nodes " + req.nodeA + " and " + req.nodeB;
            if (req.criticalForMotion) {
                report.errors.push_back(msg);
                report.allowMotion = false;
            } else {
                report.warnings.push_back(msg);
            }
            continue;
        }

        if (!canfw::isNonZeroCanId(nodeA->groupID) ||
            !canfw::isNonZeroCanId(nodeB->groupID) ||
            nodeA->groupID != nodeB->groupID) {
            const String msg = String("[MachineSpec] required group ") + req.name +
                               " needs " + req.nodeA + " and " + req.nodeB +
                               " in the same non-zero group";
            if (req.criticalForMotion) {
                report.errors.push_back(msg);
                report.allowMotion = false;
            } else {
                report.warnings.push_back(msg);
            }
            continue;
        }

        JsonArrayConst groupNodes = groupObj["nodes"];
        if (!arrayHasNode(groupNodes, req.nodeA) || !arrayHasNode(groupNodes, req.nodeB)) {
            const String msg = String("[MachineSpec] group ") + req.name +
                               " must list nodes " + req.nodeA + " and " + req.nodeB;
            if (req.criticalForMotion) {
                report.errors.push_back(msg);
                report.allowMotion = false;
            } else {
                report.warnings.push_back(msg);
            }
            continue;
        }

        if (nodeA->groupID != declaredGroupId) {
            const String msg = String("[MachineSpec] required group ") + req.name +
                               " mismatch: declared=0x" + String(declaredGroupId, HEX) +
                               ", nodes=0x" + String(nodeA->groupID, HEX);
            if (req.criticalForMotion) {
                report.errors.push_back(msg);
                report.allowMotion = false;
            } else {
                report.warnings.push_back(msg);
            }
        }
    }

    return report;
}

MachineSpec::Report MachineSpec::validateDeviceConfig(JsonObjectConst device) const {
    JsonArrayConst nodeArray = device["nodes"];
    if (nodeArray.isNull()) {
        return Report{};
    }

    std::vector<NodeInfo> nodeInfos;
    for (JsonVariantConst item : nodeArray) {
        const char* raw = item.as<const char*>();
        if (raw == nullptr || raw[0] == '\0') continue;
        NodeInfo info;
        info.name = raw;
        nodeInfos.push_back(info);
    }
    return validateControllerConfig(device, nodeInfos);
}

bool MachineSpec::arrayHasNode(JsonArrayConst nodes, const String& nodeName) {
    if (nodes.isNull()) return false;
    for (JsonVariantConst item : nodes) {
        const char* raw = item.as<const char*>();
        if (raw == nullptr) continue;
        if (nodeName == raw) return true;
    }
    return false;
}

const MachineSpec::NodeInfo* MachineSpec::findNode(const std::vector<NodeInfo>& nodes, const String& nodeName) {
    for (const NodeInfo& node : nodes) {
        if (node.name == nodeName) {
            return &node;
        }
    }
    return nullptr;
}
