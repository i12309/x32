#include "Machine/MachineSpec.h"

MachineSpec MachineSpec::get(Catalog::MachineType type) {
    switch (type) {
        case Catalog::MachineType::A:
            return MachineSpec::makeA();

        // Здесь будет расширение на другие типы станков.
        // Логика простая: добавляем makeB()/makeC() и ветки switch.
        case Catalog::MachineType::B:
        case Catalog::MachineType::C:
        case Catalog::MachineType::D:
        case Catalog::MachineType::E:
        case Catalog::MachineType::F:
        case Catalog::MachineType::UNKNOWN:
        default:
            return MachineSpec::makeUnknown();
    }
}

MachineSpec MachineSpec::makeUnknown() {
    MachineSpec spec;
    spec.type_ = Catalog::MachineType::UNKNOWN;
    return spec;
}

void MachineSpec::fillDeviceDefaults(JsonObject device) const {
    if (type_ == Catalog::MachineType::A) {
        fillDefaultsA(device);
        return;
    }

    // Для неизвестного или еще не описанного типа оставляем пустую структуру.
    // Это удобно, чтобы код, который вызывает fillDeviceDefaults, всегда получал валидный JsonObject.
    device["I2C"].to<JsonObject>();
    device["motors"].to<JsonObject>();
    device["sensors"].to<JsonObject>();
    device["optical"].to<JsonObject>();
    device["clutchs"].to<JsonObject>();
    device["switchs"].to<JsonObject>();
    device["buttons"].to<JsonObject>();
    device["encoders"].to<JsonObject>();
}

MachineSpec::Report MachineSpec::validateDeviceConfig(JsonObjectConst device) const {
    Report report;

    if (type_ == Catalog::MachineType::UNKNOWN) {
        report.errors.push_back("[MachineSpec] Неизвестный MachineType, спецификация не выбрана.");
        report.allowMotion = false;
        return report;
    }

    validateSectionConfig("I2C", device["I2C"], i2c_, report);
    validateSectionConfig("motors", device["motors"], motors_, report);
    validateSectionConfig("sensors", device["sensors"], sensors_, report);
    validateSectionConfig("optical", device["optical"], optical_, report);
    validateSectionConfig("clutchs", device["clutchs"], clutchs_, report);
    validateSectionConfig("switchs", device["switchs"], switchs_, report);
    validateSectionConfig("buttons", device["buttons"], buttons_, report);
    validateSectionConfig("encoders", device["encoders"], encoders_, report);

    if (report.hasErrors()) {
        // Если конфиг не соответствует обязательному минимуму,
        // движение разрешать рискованно.
        report.allowMotion = false;
    }

    return report;
}

void MachineSpec::validateSectionConfig(
    const char* sectionName,
    JsonObjectConst sectionObj,
    const std::vector<Requirement>& required,
    Report& report
) {
    if (required.empty()) return;

    if (sectionObj.isNull()) {
        report.errors.push_back(String("[MachineSpec] В config отсутствует секция device.") + sectionName);
        return;
    }

    for (const Requirement& req : required) {
        if (sectionObj[req.name].isNull()) {
            String msg = String("[MachineSpec] Отсутствует device.") + sectionName + "." + req.name;
            if (req.criticalForMotion) {
                report.errors.push_back(msg);
                report.allowMotion = false;
            } else {
                report.warnings.push_back(msg);
            }
        }
    }
}
