#include "Machine/Machine.h"

#if !defined(X32_TARGET_HEAD_UNIT)
#include "Machine/Context/MachineContextA.h"
#include "Machine/Context/MachineContextB.h"
#endif

Machine::Machine() = default;

Machine& Machine::getInstance() {
    static Machine instance;
    return instance;
}

bool Machine::select(Catalog::MachineType type, String* errorMessage) {
    type_ = type;
    spec_ = MachineSpec::get(type);
#if !defined(X32_TARGET_HEAD_UNIT)
    runtimeContext_.clearBindings();
#endif
    contextBound_ = false;
    lastConfigReport_ = MachineSpec::Report{};
    lastRegistryReport_ = MachineSpec::Report{};

    if (spec_.type() == Catalog::MachineType::UNKNOWN
#if !defined(X32_TARGET_HEAD_UNIT)
        || buildContext(type) == nullptr
#endif
    ) {
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

#if !defined(X32_TARGET_HEAD_UNIT)
bool Machine::bindRegistry(Registry& registry, String* errorMessage) {
    IMachineContext* activeContext = buildContext(type_);
    if (!activeContext) {
        if (errorMessage) *errorMessage = "[Machine] Context is not selected.";
        return false;
    }
    activeContext->bind(registry);
    runtimeContext_.copyBindingsFrom(*activeContext);
    contextBound_ = true;
    return true;
}

IMachineContext& Machine::context() {
    return runtimeContext_;
}

const IMachineContext& Machine::context() const {
    return runtimeContext_;
}

MachineSpec::Report Machine::validateConfig(JsonObjectConst device) {
    lastConfigReport_ = spec_.validateDeviceConfig(device);
    return lastConfigReport_;
}

MachineSpec::Report Machine::validateRegistry(const Registry& registry) {
    lastRegistryReport_ = spec_.validateRegistry(registry);
    return lastRegistryReport_;
}
#endif

bool Machine::readyForMotion() const {
#if defined(X32_TARGET_HEAD_UNIT)
    return true;
#else
    IMachineContext* activeContext = buildContext(type_);
    if (!activeContext) return false;
    if (!contextBound_) return false;
    if (!lastRegistryReport_.allowMotion) return false;
    return activeContext->readyForMotion();
#endif
}

bool Machine::shouldContinueBoot(bool allowMissingHardware) const {
    if (!lastRegistryReport_.hasErrors()) return true;
    if (!allowMissingHardware) return false;

    for (const String& err : lastRegistryReport_.errors) {
        if (!isMissingObjectError(err)) return false;
    }
    return true;
}

void Machine::collectContextIssues(std::vector<String>& issues) const {
#if defined(X32_TARGET_HEAD_UNIT)
    (void)issues;
#else
    IMachineContext* activeContext = buildContext(type_);
    if (!activeContext) {
        issues.push_back("[Machine] Context is not selected.");
        return;
    }
    if (!contextBound_) {
        issues.push_back("[Machine] Context is not bound to Registry.");
        return;
    }
    activeContext->collectIssues(issues);
#endif
}

#if !defined(X32_TARGET_HEAD_UNIT)
IMachineContext* Machine::buildContext(Catalog::MachineType type) const {
    switch (type) {
        case Catalog::MachineType::A:
            return &MachineContextA::instance();
        case Catalog::MachineType::B:
            return &MachineContextB::instance();
        case Catalog::MachineType::C:
        case Catalog::MachineType::D:
        case Catalog::MachineType::E:
        case Catalog::MachineType::F:
        case Catalog::MachineType::UNKNOWN:
        default:
            return nullptr;
    }
}
#endif

bool Machine::isMissingObjectError(const String& error) {
    return error.indexOf("Не создан объект") >= 0 || error.indexOf("Missing required object") >= 0;
}
