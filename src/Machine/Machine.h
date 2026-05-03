#pragma once

#include <Arduino.h>

#include "Catalog.h"
#include "Machine/MachineSpec.h"
#if !defined(X32_TARGET_HEAD_UNIT)
#include "Machine/Context/IMachineContext.h"
#else
class Registry;
#endif

class Machine {
public:
    static Machine& getInstance();

    bool select(Catalog::MachineType type, String* errorMessage = nullptr);
    bool selectByName(const String& machineName, String* errorMessage = nullptr);

    Catalog::MachineType type() const { return type_; }
    const MachineSpec& spec() const { return spec_; }

#if !defined(X32_TARGET_HEAD_UNIT)
    bool bindRegistry(Registry& registry, String* errorMessage = nullptr);
    IMachineContext& context();
    const IMachineContext& context() const;
#endif

    MachineSpec::Report validateConfig(JsonObjectConst device);
#if !defined(X32_TARGET_HEAD_UNIT)
    MachineSpec::Report validateRegistry(const Registry& registry);
#endif

    const MachineSpec::Report& lastConfigReport() const { return lastConfigReport_; }
    const MachineSpec::Report& lastRegistryReport() const { return lastRegistryReport_; }

    bool readyForMotion() const;

    // Решение policy после validateRegistry():
    // - без allowMissingHardware ошибки блокируют boot;
    // - с allowMissingHardware разрешаем только "missing object" ошибки.
    bool shouldContinueBoot(bool allowMissingHardware) const;

    void collectContextIssues(std::vector<String>& issues) const;

private:
    Machine();

#if !defined(X32_TARGET_HEAD_UNIT)
    IMachineContext* buildContext(Catalog::MachineType type) const;
#endif
    static bool isMissingObjectError(const String& error);

    Catalog::MachineType type_ = Catalog::MachineType::UNKNOWN;
    MachineSpec spec_ = MachineSpec::get(Catalog::MachineType::UNKNOWN);
#if !defined(X32_TARGET_HEAD_UNIT)
    // Стабильный runtime-контекст, который получают долгоживущие ссылки (Page/State/Scene).
    IMachineContext runtimeContext_;
#endif
    bool contextBound_ = false;

    MachineSpec::Report lastConfigReport_;
    MachineSpec::Report lastRegistryReport_;
};
