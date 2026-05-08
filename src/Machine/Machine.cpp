#include "Machine/Machine.h"

Machine::Machine() = default;

Machine& Machine::getInstance() {
    static Machine instance;
    return instance;
}

bool Machine::select(Catalog::MachineType type, String* errorMessage) {
    type_ = type;
    spec_ = MachineSpec::get(type);
    lastConfigReport_ = MachineSpec::Report{};

    if (spec_.type() == Catalog::MachineType::UNKNOWN) {
        if (errorMessage) {
            *errorMessage = String("[Machine] Unsupported machine type: ") + Catalog::machineName(type);
        }
        return false;
    }
    return true;
}

bool Machine::selectByName(const String& machineName, String* errorMessage) {
    return select(Catalog::getMachine(machineName), errorMessage);
}

MachineSpec::Report Machine::validateConfig(JsonObjectConst device) {
    lastConfigReport_ = spec_.validateDeviceConfig(device);
    return lastConfigReport_;
}
