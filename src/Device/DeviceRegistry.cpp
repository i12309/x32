#include "Device/DeviceRegistry.h"

#include <cstring>

#include "Can/CanRouter.h"
#include "Service/Log.h"

namespace {

bool isTerminalTaskStatus(DeviceTaskStatus status) {
    return status == DeviceTaskStatus::Done ||
           status == DeviceTaskStatus::Failed ||
           status == DeviceTaskStatus::Timeout ||
           status == DeviceTaskStatus::Rejected;
}

struct RoleCommandRequirement {
    Role role;
    DeviceCommand command;
};

constexpr RoleCommandRequirement RequiredRoleCommands[] = {
    {Role::Paper, DeviceCommand::PaperFeed},
    {Role::Paper, DeviceCommand::Stop},
    {Role::Table, DeviceCommand::TableUp},
    {Role::Table, DeviceCommand::TableDown},
    {Role::Table, DeviceCommand::Stop},
    {Role::Guillotine, DeviceCommand::GuillotineCut},
    {Role::Guillotine, DeviceCommand::Stop},
    {Role::Motion, DeviceCommand::Check},
    {Role::Motion, DeviceCommand::Stop},
    {Role::Panel, DeviceCommand::Check},
};

const char* protocolHint(DeviceCommand command) {
    switch (command) {
        case DeviceCommand::PaperFeedUntilMark:
        case DeviceCommand::TableUp:
        case DeviceCommand::TableDown:
        case DeviceCommand::GuillotineCut:
        case DeviceCommand::ProfileRun:
        case DeviceCommand::SelfTest:
        case DeviceCommand::Configure:
            return "stm.v1";
        default:
            return "compatible protocol";
    }
}

} // namespace

DeviceRegistry& DeviceRegistry::getInstance() {
    static DeviceRegistry instance;
    return instance;
}

bool DeviceRegistry::loadFromConfig(JsonObjectConst root) {
    count_ = 0;
    roleCount_ = 0;
    nextTaskId_ = 1;
    can_ = CanSettings();

    JsonObjectConst canObj = root["can"];
    if (!canObj.isNull()) {
        can_.txPin = canObj["tx_pin"] | can_.txPin;
        can_.rxPin = canObj["rx_pin"] | can_.rxPin;
        can_.bitrate = canObj["bitrate"] | can_.bitrate;
        can_.heartbeatPeriodMs = canObj["heartbeat_period_ms"] | can_.heartbeatPeriodMs;
        can_.heartbeatTimeoutMs = canObj["heartbeat_timeout_ms"] | can_.heartbeatTimeoutMs;
        can_.taskTimeoutMs = canObj["task_timeout_ms"] | can_.taskTimeoutMs;
    }

    JsonObjectConst devicesObj = root["devices"];
    if (devicesObj.isNull()) {
        Log::D("[DeviceRegistry] config.devices is missing; CAN head-device layer is empty.");
        return true;
    }

    for (JsonPairConst pair : devicesObj) {
        if (count_ >= MaxDevices) {
            Log::E("[DeviceRegistry] Too many devices, max=%u", MaxDevices);
            return false;
        }

        JsonObjectConst cfg = pair.value().as<JsonObjectConst>();
        DeviceNode& node = devices_[count_++];
        node = DeviceNode();
        node.name = pair.key().c_str();
        node.address = cfg["address"] | 0;
        node.protocol = cfg["protocol"] | "";
        node.required = cfg["required"] | false;
        node.configPayload = cfg["device"];
        node.status.deviceName = node.name ? node.name : "";
        node.expectedProtocolMajor = cfg["protocol_major"] | 1;
        node.expectedProtocolMinor = cfg["protocol_minor"] | 0;

        Log::L("[DeviceRegistry] device name=%s address=%u protocol=%s required=%d",
               node.name,
               node.address,
               node.protocol,
               node.required ? 1 : 0);
    }

    JsonObjectConst rolesObj = root["roles"];
    if (!rolesObj.isNull()) {
        for (JsonPairConst pair : rolesObj) {
            if (roleCount_ >= MaxRoles) break;
            roles_[roleCount_].role = roleFromName(pair.key().c_str());
            roles_[roleCount_].deviceName = pair.value() | nullptr;
            if (roles_[roleCount_].role != Role::Unknown && roles_[roleCount_].deviceName != nullptr) {
                roleCount_++;
            }
        }
    }
    addDefaultRoleBindings();
    return true;
}

void DeviceRegistry::bindRouter(CanRouter* router) {
    router_ = router;
}

bool DeviceRegistry::configureRequiredDevices() {
    bool ok = true;
    for (uint8_t i = 0; i < count_; ++i) {
        if (!devices_[i].required) continue;
        if (!validateRequiredCapabilities(devices_[i])) {
            ok = false;
            continue;
        }
        DeviceTaskId id = sendConfigure(devices_[i].name, can_.taskTimeoutMs);
        if (id == 0 || devices_[i].activeTaskStatus == DeviceTaskStatus::Rejected) {
            ok = false;
        }
    }
    return ok;
}

DeviceTaskId DeviceRegistry::sendTask(Role role, DeviceCommand cmd, const DeviceParams& params, uint32_t timeoutMs) {
    DeviceNode* node = findByRole(role);
    if (node == nullptr) {
        Log::E("[DeviceRegistry] No device for role=%s", roleName(role));
        return 0;
    }
    return sendTask(node->name, cmd, params, timeoutMs);
}

DeviceTaskId DeviceRegistry::sendTask(uint8_t address, DeviceCommand cmd, const DeviceParams& params, uint32_t timeoutMs) {
    DeviceNode* node = findByAddress(address);
    if (node == nullptr) {
        Log::E("[DeviceRegistry] No device for address=%u", address);
        return 0;
    }
    return sendTask(node->name, cmd, params, timeoutMs);
}

DeviceTaskId DeviceRegistry::sendTask(DeviceName name, DeviceCommand cmd, const DeviceParams& params, uint32_t timeoutMs) {
    DeviceNode* node = findByName(name);
    if (node == nullptr) return 0;

    if (node->status.incompatible) {
        Log::E("[DeviceRegistry] Task rejected device=%s command=%s: incompatible protocol",
               node->name,
               commandName(cmd));
        return queueRejected(node);
    }

    DeviceTask task;
    task.id = nextTaskId();
    task.deviceAddress = node->address;
    task.command = cmd;
    task.params = params;
    task.timeoutMs = timeoutOrDefault(timeoutMs);

    node->activeTaskId = task.id;
    node->activeTaskStatus = DeviceTaskStatus::Queued;
    node->activeTaskDeadlineMs = millis() + task.timeoutMs;

    if (router_ == nullptr || !router_->sendTask(*node, task)) {
        node->activeTaskStatus = DeviceTaskStatus::Rejected;
        Log::E("[DeviceRegistry] Task rejected device=%s command=%s", node->name, commandName(cmd));
    }
    return task.id;
}

DeviceTaskId DeviceRegistry::sendConfigure(DeviceName name, uint32_t timeoutMs) {
    DeviceNode* node = findByName(name);
    if (node == nullptr) return 0;

    ConfigureTask task;
    task.id = nextTaskId();
    task.deviceAddress = node->address;
    task.payload = node->configPayload;
    task.timeoutMs = timeoutOrDefault(timeoutMs);

    node->activeTaskId = task.id;
    node->activeTaskStatus = DeviceTaskStatus::Queued;
    node->activeTaskDeadlineMs = millis() + task.timeoutMs;

    if (router_ == nullptr || !router_->sendConfigure(*node, task)) {
        node->activeTaskStatus = DeviceTaskStatus::Rejected;
        Log::E("[DeviceRegistry] Configure rejected device=%s", node->name);
    }
    return task.id;
}

DeviceTaskStatus DeviceRegistry::taskStatus(DeviceTaskId id) const {
    for (uint8_t i = 0; i < count_; ++i) {
        if (devices_[i].activeTaskId == id) return devices_[i].activeTaskStatus;
    }
    return DeviceTaskStatus::Unknown;
}

DeviceStatus DeviceRegistry::status(DeviceName name) const {
    const DeviceNode* node = findByName(name);
    return node ? node->status : DeviceStatus();
}

DeviceStatus DeviceRegistry::status(Role role) const {
    const DeviceNode* node = findByRole(role);
    return node ? node->status : DeviceStatus();
}

bool DeviceRegistry::allRequiredReady() const {
    for (uint8_t i = 0; i < count_; ++i) {
        const DeviceNode& node = devices_[i];
        if (!node.required) continue;
        if (!node.status.online || !node.status.ready || node.status.error || node.status.incompatible) return false;
    }
    return true;
}

DeviceNode* DeviceRegistry::findByName(DeviceName name) {
    if (name == nullptr) return nullptr;
    for (uint8_t i = 0; i < count_; ++i) {
        if (devices_[i].name != nullptr && strcmp(devices_[i].name, name) == 0) return &devices_[i];
    }
    return nullptr;
}

DeviceNode* DeviceRegistry::findByAddress(uint8_t address) {
    for (uint8_t i = 0; i < count_; ++i) {
        if (devices_[i].address == address) return &devices_[i];
    }
    return nullptr;
}

DeviceNode* DeviceRegistry::findByRole(Role role) {
    DeviceName name = resolveRole(role);
    return findByName(name);
}

const DeviceNode* DeviceRegistry::findByName(DeviceName name) const {
    return const_cast<DeviceRegistry*>(this)->findByName(name);
}

const DeviceNode* DeviceRegistry::findByAddress(uint8_t address) const {
    return const_cast<DeviceRegistry*>(this)->findByAddress(address);
}

const DeviceNode* DeviceRegistry::findByRole(Role role) const {
    return const_cast<DeviceRegistry*>(this)->findByRole(role);
}

void DeviceRegistry::markHeartbeat(uint8_t address, uint8_t protocolMajor, uint8_t protocolMinor, uint8_t state, uint8_t errorCode, DeviceTaskId activeTaskId) {
    DeviceNode* node = findByAddress(address);
    if (node == nullptr) return;

    node->lastHeartbeatMs = millis();
    node->status.online = true;
    node->status.ready = (state & 0x01) != 0;
    node->status.busy = (state & 0x02) != 0;
    node->status.error = (state & 0x04) != 0;
    node->status.incompatible = protocolMajor != node->expectedProtocolMajor;

    if (node->status.incompatible) {
        node->status.online = false;
        node->status.errorText = "protocol major mismatch";
    } else if (errorCode != 0) {
        node->status.errorText = String("error ") + String(errorCode);
    } else {
        node->status.errorText = "";
    }

    if (activeTaskId != 0) {
        if (node->activeTaskId == 0 || static_cast<uint16_t>(node->activeTaskId) == static_cast<uint16_t>(activeTaskId)) {
            if (node->activeTaskId == 0) node->activeTaskId = activeTaskId;
            node->activeTaskStatus = node->status.busy ? DeviceTaskStatus::Running : DeviceTaskStatus::Accepted;
        }
    } else if (node->activeTaskStatus == DeviceTaskStatus::Sent ||
               node->activeTaskStatus == DeviceTaskStatus::Accepted ||
               node->activeTaskStatus == DeviceTaskStatus::Running) {
        node->activeTaskStatus = node->status.error ? DeviceTaskStatus::Failed : DeviceTaskStatus::Done;
        node->activeTaskDeadlineMs = 0;
    }

    if (!node->status.incompatible && protocolMinor != node->expectedProtocolMinor) {
        Log::D("[DeviceRegistry] device=%s protocol minor differs expected=%u got=%u",
               node->name,
               node->expectedProtocolMinor,
               protocolMinor);
    }
}

void DeviceRegistry::markTaskStatus(DeviceTaskId taskId, DeviceTaskStatus status) {
    for (uint8_t i = 0; i < count_; ++i) {
        if (devices_[i].activeTaskId == taskId) {
            devices_[i].activeTaskStatus = status;
            if (isTerminalTaskStatus(status)) devices_[i].activeTaskDeadlineMs = 0;
            return;
        }
    }
}

void DeviceRegistry::process(uint32_t nowMs) {
    for (uint8_t i = 0; i < count_; ++i) {
        DeviceNode& node = devices_[i];
        if (node.lastHeartbeatMs != 0 && nowMs - node.lastHeartbeatMs > can_.heartbeatTimeoutMs) {
            node.status.online = false;
            node.status.ready = false;
            node.status.busy = false;
            node.status.errorText = "heartbeat timeout";
            if (node.activeTaskStatus == DeviceTaskStatus::Sent ||
                node.activeTaskStatus == DeviceTaskStatus::Accepted ||
                node.activeTaskStatus == DeviceTaskStatus::Running) {
                node.activeTaskStatus = DeviceTaskStatus::Timeout;
                node.activeTaskDeadlineMs = 0;
            }
        }

        if (node.activeTaskDeadlineMs != 0 && nowMs > node.activeTaskDeadlineMs) {
            if (node.activeTaskStatus == DeviceTaskStatus::Queued ||
                node.activeTaskStatus == DeviceTaskStatus::Sent ||
                node.activeTaskStatus == DeviceTaskStatus::Accepted ||
                node.activeTaskStatus == DeviceTaskStatus::Running) {
                node.activeTaskStatus = DeviceTaskStatus::Timeout;
                node.activeTaskDeadlineMs = 0;
            }
        }
    }
}

const DeviceNode* DeviceRegistry::deviceAt(uint8_t index) const {
    if (index >= count_) return nullptr;
    return &devices_[index];
}

DeviceTaskId DeviceRegistry::nextTaskId() {
    if (nextTaskId_ == 0) nextTaskId_ = 1;
    return nextTaskId_++;
}

DeviceTaskId DeviceRegistry::queueRejected(DeviceNode* node) {
    if (node == nullptr) return 0;
    node->activeTaskId = nextTaskId();
    node->activeTaskStatus = DeviceTaskStatus::Rejected;
    node->activeTaskDeadlineMs = 0;
    return node->activeTaskId;
}

bool DeviceRegistry::validateRequiredCapabilities(const DeviceNode& node) const {
    if (router_ == nullptr) {
        Log::E("[DeviceRegistry] Cannot validate device=%s: CAN router is not bound", node.name);
        return false;
    }

    bool ok = true;
    if (!router_->supports(node.protocol, DeviceCommand::Configure)) {
        Log::E("[DeviceRegistry] device '%s' (%s) does not support %s, нужен protocol %s",
               node.name,
               node.protocol,
               commandName(DeviceCommand::Configure),
               protocolHint(DeviceCommand::Configure));
        ok = false;
    }

    for (uint8_t r = 0; r < roleCount_; ++r) {
        if (roles_[r].deviceName == nullptr || strcmp(roles_[r].deviceName, node.name) != 0) continue;

        for (const RoleCommandRequirement& requirement : RequiredRoleCommands) {
            if (requirement.role != roles_[r].role) continue;
            if (router_->supports(node.protocol, requirement.command)) continue;

            Log::E("[DeviceRegistry] device '%s' (%s) does not support %s for role %s, нужен protocol %s",
                   node.name,
                   node.protocol,
                   commandName(requirement.command),
                   roleName(roles_[r].role),
                   protocolHint(requirement.command));
            ok = false;
        }
    }
    return ok;
}

void DeviceRegistry::addDefaultRoleBindings() {
    for (uint8_t i = 0; i < count_ && roleCount_ < MaxRoles; ++i) {
        Role role = roleFromName(devices_[i].name);
        if (role == Role::Unknown) continue;
        bool exists = false;
        for (uint8_t r = 0; r < roleCount_; ++r) {
            if (roles_[r].role == role) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            roles_[roleCount_].role = role;
            roles_[roleCount_].deviceName = devices_[i].name;
            roleCount_++;
        }
    }
}

DeviceName DeviceRegistry::resolveRole(Role role) const {
    for (uint8_t i = 0; i < roleCount_; ++i) {
        if (roles_[i].role == role) return roles_[i].deviceName;
    }
    return roleName(role);
}

uint32_t DeviceRegistry::timeoutOrDefault(uint32_t timeoutMs) const {
    return timeoutMs == 0 ? can_.taskTimeoutMs : timeoutMs;
}
