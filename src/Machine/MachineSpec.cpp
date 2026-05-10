#include "Machine/MachineSpec.h"

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
    // Пустой каркас. Конкретное наполнение делает ConfigDefaults.
    root["nodes"].to<JsonArray>();
    root["groups"].to<JsonObject>();
}

void MachineSpec::fillDeviceDefaults(JsonObject device) const {
    // Переходный мост для старого ConfigDefaults, который пока пишет config.device.
    // Новая модель строится от корня и использует fillControllerDefaults().
    (void)device;
}

MachineSpec::Report MachineSpec::validateControllerConfig(JsonObjectConst root) const {
    Report report;

    if (type_ == Catalog::MachineType::UNKNOWN) {
        report.errors.push_back("[MachineSpec] Неизвестный MachineType, спецификация не выбрана.");
        report.allowMotion = false;
        return report;
    }

    JsonArrayConst nodes = root["nodes"];
    if (nodes.isNull()) {
        report.errors.push_back("[MachineSpec] В config отсутствует секция nodes.");
        report.allowMotion = false;
        return report;
    }

    for (const NodeRequirement& req : requiredNodes_) {
        if (arrayHasNode(nodes, req.name)) continue;

        const String msg = String("[MachineSpec] Отсутствует обязательная нода в config.nodes: ") + req.name;
        if (req.criticalForMotion) {
            report.errors.push_back(msg);
            report.allowMotion = false;
        } else {
            report.warnings.push_back(msg);
        }
    }

    JsonObjectConst groups = root["groups"];
    if (groups.isNull() && !requiredGroups_.empty()) {
        report.errors.push_back("[MachineSpec] В config отсутствует секция groups.");
        report.allowMotion = false;
        return report;
    }

    for (const GroupRequirement& req : requiredGroups_) {
        if (!arrayHasNode(nodes, req.nodeA) || !arrayHasNode(nodes, req.nodeB)) {
            const String msg = String("[MachineSpec] Группа ") + req.name +
                               " не может быть проверена: отсутствуют ноды " + req.nodeA + " и/или " + req.nodeB;
            if (req.criticalForMotion) {
                report.errors.push_back(msg);
                report.allowMotion = false;
            } else {
                report.warnings.push_back(msg);
            }
            continue;
        }

        uint16_t groupA = 0;
        uint16_t groupB = 0;
        String groupNameA;
        String groupNameB;
        const bool hasGroupA = collectNodeGroup(groups, req.nodeA, groupA, groupNameA);
        const bool hasGroupB = collectNodeGroup(groups, req.nodeB, groupB, groupNameB);

        if (!hasGroupA || groupA == 0 || !hasGroupB || groupB == 0 || groupA != groupB) {
            const String msg = String("[MachineSpec] Обязательная группа ") + req.name +
                               " требует, чтобы ноды " + req.nodeA + " и " + req.nodeB +
                               " были в одной и той же ненулевой группе.";
            if (req.criticalForMotion) {
                report.errors.push_back(msg);
                report.allowMotion = false;
            } else {
                report.warnings.push_back(msg);
            }
            continue;
        }

        // collectNodeGroup() возвращает значения только из config.groups, но проверку оставляем явной.
        bool groupIdDeclared = false;
        for (JsonPairConst item : groups) {
            JsonObjectConst groupObj = item.value().as<JsonObjectConst>();
            if (groupObj.isNull()) continue;
            if (parseCanID(groupObj["canID"]) == groupA) {
                groupIdDeclared = true;
                break;
            }
        }
        if (!groupIdDeclared) {
            const String msg = String("[MachineSpec] Group ID 0x") + String(groupA, HEX) +
                               " для связи " + req.name + " не объявлен в config.groups.";
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
    return validateControllerConfig(device);
}

uint16_t MachineSpec::parseCanID(JsonVariantConst value) {
    if (value.isNull()) return 0;

    if (value.is<int>()) {
        const int raw = value.as<int>();
        if (raw <= 0 || raw > 0x7FF) return 0;
        return static_cast<uint16_t>(raw);
    }

    const char* text = value | "";
    if (text == nullptr || text[0] == '\0') return 0;

    char* end = nullptr;
    const unsigned long raw = strtoul(text, &end, 0);
    if (end == text || *end != '\0' || raw == 0 || raw > 0x7FFUL) return 0;
    return static_cast<uint16_t>(raw);
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

bool MachineSpec::collectNodeGroup(
    JsonObjectConst groupsObj,
    const String& nodeName,
    uint16_t& groupID,
    String& groupName
) {
    groupID = 0;
    groupName = "";
    if (groupsObj.isNull()) return false;

    for (JsonPairConst item : groupsObj) {
        JsonObjectConst groupObj = item.value().as<JsonObjectConst>();
        if (groupObj.isNull()) continue;

        JsonArrayConst groupNodes = groupObj["nodes"];
        if (!arrayHasNode(groupNodes, nodeName)) continue;

        const uint16_t parsedID = parseCanID(groupObj["canID"]);
        if (parsedID == 0) continue;

        groupID = parsedID;
        groupName = String(item.key().c_str());
        return true;
    }

    return false;
}
