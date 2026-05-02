#include "Device/DeviceTask.h"

#include <cstring>

const char* roleName(Role role) {
    switch (role) {
        case Role::Paper: return "paper";
        case Role::Table: return "table";
        case Role::Guillotine: return "guillotine";
        case Role::Panel: return "panel";
        case Role::Motion: return "motion";
        case Role::Check: return "check";
        case Role::Unknown:
        default: return "unknown";
    }
}

Role roleFromName(const char* name) {
    if (name == nullptr) return Role::Unknown;
    if (strcmp(name, "paper") == 0) return Role::Paper;
    if (strcmp(name, "table") == 0) return Role::Table;
    if (strcmp(name, "guillotine") == 0) return Role::Guillotine;
    if (strcmp(name, "panel") == 0) return Role::Panel;
    if (strcmp(name, "motion") == 0) return Role::Motion;
    if (strcmp(name, "check") == 0) return Role::Check;
    return Role::Unknown;
}

const char* commandName(DeviceCommand command) {
    switch (command) {
        case DeviceCommand::Configure: return "Configure";
        case DeviceCommand::SelfTest: return "SelfTest";
        case DeviceCommand::Check: return "Check";
        case DeviceCommand::Stop: return "Stop";
        case DeviceCommand::ResetError: return "ResetError";
        case DeviceCommand::PaperFeed: return "PaperFeed";
        case DeviceCommand::PaperFeedUntilMark: return "PaperFeedUntilMark";
        case DeviceCommand::TableUp: return "TableUp";
        case DeviceCommand::TableDown: return "TableDown";
        case DeviceCommand::GuillotineCut: return "GuillotineCut";
        case DeviceCommand::ProfileRun: return "ProfileRun";
        default: return "Unknown";
    }
}
