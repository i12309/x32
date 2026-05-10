#include "Machine/MachineSpec.h"

MachineSpec MachineSpec::makeA() {
    MachineSpec spec;
    spec.type_ = Catalog::MachineType::A;

    spec.requiredNodes_.push_back({"TABLE", true});
    spec.requiredNodes_.push_back({"GUILLOTINE", true});
    spec.requiredNodes_.push_back({"PAPER", true});
    spec.requiredNodes_.push_back({"THROW", true});

    spec.requiredGroups_.push_back({"FEED", "PAPER", "THROW", true});

    return spec;
}
