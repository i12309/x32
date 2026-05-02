#include "Can/StmCan.h"

#include "Device/DeviceRegistry.h"
#include "Service/Log.h"

namespace {

uint8_t commandCode(DeviceCommand command) {
    return static_cast<uint8_t>(command);
}

uint32_t commandCanId(uint8_t address) {
    return 0x300 + address;
}

bool isHeartbeat(uint32_t id) {
    return id >= 0x700 && id <= 0x77F;
}

} // namespace

bool StmCan::supports(DeviceCommand command) const {
    switch (command) {
        case DeviceCommand::Configure:
        case DeviceCommand::SelfTest:
        case DeviceCommand::Check:
        case DeviceCommand::Stop:
        case DeviceCommand::ResetError:
        case DeviceCommand::PaperFeed:
        case DeviceCommand::PaperFeedUntilMark:
        case DeviceCommand::TableUp:
        case DeviceCommand::TableDown:
        case DeviceCommand::GuillotineCut:
        case DeviceCommand::ProfileRun:
            return true;
        default:
            return false;
    }
}

bool StmCan::sendTask(const DeviceNode& node, const DeviceTask& task) {
    if (!supports(task.command)) return false;

    CanFrame frame;
    frame.id = commandCanId(node.address);
    frame.size = 8;
    frame.data[0] = commandCode(task.command);
    frame.data[1] = static_cast<uint8_t>(task.id & 0xFF);
    frame.data[2] = static_cast<uint8_t>((task.id >> 8) & 0xFF);

    switch (task.command) {
        case DeviceCommand::PaperFeed:
        case DeviceCommand::PaperFeedUntilMark: {
            int16_t mm10 = static_cast<int16_t>(task.params.u.paperFeed.mm * 10.0f);
            frame.data[3] = static_cast<uint8_t>(mm10 & 0xFF);
            frame.data[4] = static_cast<uint8_t>((mm10 >> 8) & 0xFF);
            frame.data[5] = static_cast<uint8_t>(task.params.u.paperFeed.speedMmS & 0xFF);
            frame.data[6] = static_cast<uint8_t>((task.params.u.paperFeed.speedMmS >> 8) & 0xFF);
            break;
        }
        case DeviceCommand::GuillotineCut:
            frame.data[3] = static_cast<uint8_t>(task.params.u.guillotineCut.cutSpeedMmS & 0xFF);
            frame.data[4] = static_cast<uint8_t>((task.params.u.guillotineCut.cutSpeedMmS >> 8) & 0xFF);
            break;
        case DeviceCommand::ProfileRun:
            frame.data[3] = task.params.u.profileRun.profileId;
            break;
        default:
            break;
    }

    return bus_.send(frame);
}

bool StmCan::sendConfigure(const DeviceNode& node, const ConfigureTask& task) {
    // TODO(CAN): полный configure для STM32 может занимать несколько кадров.
    // Сейчас отправляется короткий стартовый кадр, чтобы связать boot flow,
    // task-id и будущий протокол конфигурации без копирования JSON payload.
    (void)task;
    CanFrame frame;
    frame.id = commandCanId(node.address);
    frame.size = 4;
    frame.data[0] = commandCode(DeviceCommand::Configure);
    frame.data[1] = static_cast<uint8_t>(task.id & 0xFF);
    frame.data[2] = static_cast<uint8_t>((task.id >> 8) & 0xFF);
    frame.data[3] = task.payload.isNull() ? 0 : 1;
    return bus_.send(frame);
}

bool StmCan::handleFrame(const CanFrame& frame, DeviceRegistry& registry) {
    if (!isHeartbeat(frame.id) || frame.size < 8) return false;

    uint8_t address = static_cast<uint8_t>(frame.id - 0x700);
    uint16_t activeTask = static_cast<uint16_t>(frame.data[4]) |
                          (static_cast<uint16_t>(frame.data[5]) << 8);

    registry.markHeartbeat(
        address,
        frame.data[0],
        frame.data[1],
        frame.data[2],
        frame.data[3],
        activeTask
    );
    return true;
}

void StmCan::process(DeviceRegistry& registry, uint32_t nowMs) {
    (void)registry;
    (void)nowMs;
}
