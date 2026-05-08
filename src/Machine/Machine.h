#pragma once

#include <Arduino.h>

#include "Catalog.h"
#include "Machine/MachineSpec.h"

class Machine {
public:
    static Machine& getInstance();

    bool select(Catalog::MachineType type, String* errorMessage = nullptr);
    bool selectByName(const String& machineName, String* errorMessage = nullptr);

    Catalog::MachineType type() const { return type_; }
    const MachineSpec& spec() const { return spec_; }

    MachineSpec::Report validateConfig(JsonObjectConst device);
    const MachineSpec::Report& lastConfigReport() const { return lastConfigReport_; }

private:
    Machine();

    Catalog::MachineType type_ = Catalog::MachineType::UNKNOWN;
    MachineSpec spec_ = MachineSpec::get(Catalog::MachineType::UNKNOWN);
    MachineSpec::Report lastConfigReport_;
};
