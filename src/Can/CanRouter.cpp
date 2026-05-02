#include "Can/CanRouter.h"

#include <cstring>

#include "Service/Log.h"

CanRouter::CanRouter(ICanBus& bus, DeviceRegistry& registry)
    : bus_(bus), registry_(registry) {
    registry_.bindRouter(this);
}

bool CanRouter::bindProtocol(CanProtocol* protocol) {
    if (protocol == nullptr || protocolCount_ >= MaxProtocols) return false;
    if (protocolFor(protocol->name()) != nullptr) return true;
    protocols_[protocolCount_++] = protocol;
    Log::D("[CAN] protocol bound: %s", protocol->name());
    return true;
}

bool CanRouter::begin(const CanBusConfig& config) {
    return bus_.begin(config);
}

bool CanRouter::sendTask(const DeviceNode& node, const DeviceTask& task) {
    CanProtocol* protocol = protocolFor(node.protocol);
    if (protocol == nullptr) return false;
    if (!protocol->supports(task.command)) return false;

    bool ok = protocol->sendTask(node, task);
    registry_.markTaskStatus(task.id, ok ? DeviceTaskStatus::Sent : DeviceTaskStatus::Rejected);
    return ok;
}

bool CanRouter::sendConfigure(const DeviceNode& node, const ConfigureTask& task) {
    CanProtocol* protocol = protocolFor(node.protocol);
    if (protocol == nullptr) return false;
    if (!protocol->supports(DeviceCommand::Configure)) return false;

    bool ok = protocol->sendConfigure(node, task);
    registry_.markTaskStatus(task.id, ok ? DeviceTaskStatus::Sent : DeviceTaskStatus::Rejected);
    return ok;
}

bool CanRouter::supports(const char* protocolName, DeviceCommand command) {
    CanProtocol* protocol = protocolFor(protocolName);
    return protocol != nullptr && protocol->supports(command);
}

void CanRouter::process() {
    bus_.process();

    CanFrame frame;
    while (bus_.receive(frame)) {
        for (uint8_t i = 0; i < protocolCount_; ++i) {
            if (protocols_[i] != nullptr && protocols_[i]->handleFrame(frame, registry_)) break;
        }
    }

    uint32_t now = millis();
    for (uint8_t i = 0; i < protocolCount_; ++i) {
        if (protocols_[i] != nullptr) protocols_[i]->process(registry_, now);
    }
    registry_.process(now);
}

CanProtocol* CanRouter::protocolFor(const char* name) {
    if (name == nullptr) return nullptr;
    for (uint8_t i = 0; i < protocolCount_; ++i) {
        if (protocols_[i] != nullptr && strcmp(protocols_[i]->name(), name) == 0) {
            return protocols_[i];
        }
    }
    return nullptr;
}
