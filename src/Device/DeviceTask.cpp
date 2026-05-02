#include "Device/DeviceTask.h"

#include <cctype>
#include <cstring>

namespace {

bool equalsIgnoreCase(const char* lhs, const char* rhs) {
    if (lhs == nullptr || rhs == nullptr) return false;
    while (*lhs != '\0' && *rhs != '\0') {
        unsigned char l = static_cast<unsigned char>(*lhs);
        unsigned char r = static_cast<unsigned char>(*rhs);
        if (tolower(l) != tolower(r)) return false;
        ++lhs;
        ++rhs;
    }
    return *lhs == '\0' && *rhs == '\0';
}

} // namespace

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
    if (equalsIgnoreCase(name, "paper")) return Role::Paper;
    if (equalsIgnoreCase(name, "table")) return Role::Table;
    if (equalsIgnoreCase(name, "guillotine")) return Role::Guillotine;
    if (equalsIgnoreCase(name, "panel")) return Role::Panel;
    if (equalsIgnoreCase(name, "motion")) return Role::Motion;
    if (equalsIgnoreCase(name, "check")) return Role::Check;
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
