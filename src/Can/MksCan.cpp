#include "Can/MksCan.h"

#include <cstring>

#include "Device/DeviceRegistry.h"

namespace {

uint32_t mksCanId(uint8_t address) {
    return 0x140 + address;
}

} // namespace

bool MksCan::supports(DeviceCommand command) const {
    switch (command) {
        case DeviceCommand::Check:
        case DeviceCommand::Stop:
        case DeviceCommand::ResetError:
        case DeviceCommand::PaperFeed:
            return true;
        default:
            return false;
    }
}

bool MksCan::sendTask(const DeviceNode& node, const DeviceTask& task) {
    if (!supports(task.command)) return false;

    CanFrame frame;
    frame.id = mksCanId(node.address);
    frame.size = 8;

    switch (task.command) {
        case DeviceCommand::Stop:
            frame.data[0] = 0xF7;
            break;
        case DeviceCommand::ResetError:
            frame.data[0] = 0x9B;
            break;
        case DeviceCommand::Check:
            frame.data[0] = 0x30;
            break;
        case DeviceCommand::PaperFeed: {
            // TODO(CAN/MKS): уточнить точную команду move для Servo57D_CAN и
            // перевести mm/s в импульсы/скорость конкретной оси. Пока это
            // заготовка, чтобы Scene получала Rejected только для явных
            // unsupported-команд, а не из-за отсутствия класса протокола.
            frame.data[0] = 0xFD;
            int16_t mm10 = static_cast<int16_t>(task.params.u.paperFeed.mm * 10.0f);
            frame.data[1] = static_cast<uint8_t>(mm10 & 0xFF);
            frame.data[2] = static_cast<uint8_t>((mm10 >> 8) & 0xFF);
            break;
        }
        default:
            return false;
    }

    frame.data[6] = static_cast<uint8_t>(task.id & 0xFF);
    frame.data[7] = static_cast<uint8_t>((task.id >> 8) & 0xFF);
    return bus_.send(frame);
}

bool MksCan::sendConfigure(const DeviceNode& node, const ConfigureTask& task) {
    (void)node;
    (void)task;
    return false;
}

bool MksCan::handleFrame(const CanFrame& frame, DeviceRegistry& registry) {
    if (frame.id < 0x140 || frame.id > 0x1BF) return false;

    uint8_t address = static_cast<uint8_t>(frame.id - 0x140);
    uint8_t state = 0x01;
    uint8_t error = 0;
    if (frame.size > 1 && frame.data[1] != 0) {
        state |= 0x04;
        error = frame.data[1];
    }
    registry.markHeartbeat(address, 1, 0, state, error, 0);
    return true;
}

void MksCan::process(DeviceRegistry& registry, uint32_t nowMs) {
    const uint32_t period = registry.canSettings().heartbeatPeriodMs;
    if (nowMs - lastPollMs_ < period) return;
    lastPollMs_ = nowMs;

    for (uint8_t i = 0; i < registry.count(); ++i) {
        const DeviceNode* node = registry.deviceAt(i);
        if (node == nullptr || node->protocol == nullptr) continue;
        if (strcmp(node->protocol, name()) != 0) continue;

        CanFrame frame;
        frame.id = mksCanId(node->address);
        frame.size = 1;
        frame.data[0] = 0x30;
        bus_.send(frame);
    }
}
