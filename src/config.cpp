#include "config.h"
#include "Machine/MachineSpec.h"

namespace ConfigDefaults {

namespace detail {

void fillSettings(JsonObject settings, bool accessPoint) {
    settings["AUTO_UPDATE"] = 0;
    settings["UPDATE"] = 1;
    settings["SERVER"] = "45.9.43.227";
    settings["VERSION"] = "firmware.txt";
    settings["FIRMWARE"] = "firmware.bin";
    settings["HASH"] = "firmware.md5";

    settings["TFT_UPDATE"] = 1;
    settings["TFT_VERSION"] = "ui.txt";
    settings["TFT_FIRMWARE"] = "ui.tft";

    settings["CONNECT_WIFI"] = 1;
    settings["ACCESS_POINT"] = accessPoint ? 1 : 0;
    settings["HTTP_SERVER"] = 1;
    settings["WEB"] = 0;

    settings["LOG_BUFFER"] = 0;
    settings["CHECK_SYSTEM"] = 1;
    settings["ALLOW_MISSING_HARDWARE"] = 0;
    settings["licence_off"] = 1;
    settings["MAX_PROFILES"] = 50;
    settings["MAX_TASKS"] = 50;
    settings["log"] = 0;
    settings["metrics"] = 1;
    // Отдельный таймаут для McpTrigger: позволяет переживать короткую занятость I2C без ложного отказа arm/read snapshot.
    settings["MCP_TRIGGER_I2C_WAIT_MS"] = 150;
}

void fillCan(JsonObject can) {
    can["driver"] = "twai";
    can["tx_pin"] = 17;
    can["rx_pin"] = 18;
    can["bitrate"] = 500000;
    can["mode"] = "normal";
    can["heartbeat_period_ms"] = 200;
    can["heartbeat_timeout_ms"] = 1000;
    can["task_timeout_ms"] = 5000;
}

bool ensureSupportedMachine(Catalog::MachineType type) {
    MachineSpec spec = MachineSpec::get(type);
    if (spec.type() == Catalog::MachineType::UNKNOWN) {
        Log::E("[ConfigDefaults] MachineSpec is not implemented for type '%s'", Catalog::machineName(type).c_str());
        return false;
    }
    return true;
}

void fillTableDevice(JsonObject device) {
    JsonObject table = device["TABLE"].to<JsonObject>();
    table["driver"] = "RMT";
    JsonObject pin = table["pin"].to<JsonObject>();
    pin["step"] = 23;
    pin["dir"] = 0;
    pin["ena"] = 1;
    table["DelayToEnable"] = 2000;
    table["DelayToDisable"] = 6000;
    table["MicroStep"] = 3200;
    JsonArray speed = table["Speed"].to<JsonArray>();
    JsonObject normal = speed.add<JsonObject>();
    normal["Mode"] = "Normal";
    normal["Speed"] = 16000;
    normal["Acceleration"] = 32000;
    JsonObject slow = speed.add<JsonObject>();
    slow["Mode"] = "Slow";
    slow["Speed"] = 9600;
    slow["Acceleration"] = 9600;
    table["WorkMove"] = 9500;
    JsonArray check = table["Check"].to<JsonArray>();
    JsonObject notDown = check.add<JsonObject>();
    notDown["Mode"] = "TABLE_NOT_DOWN";
    notDown["step"] = 19000;
    notDown["ms"] = -1;
    JsonObject notUp = check.add<JsonObject>();
    notUp["Mode"] = "TABLE_NOT_UP";
    notUp["step"] = 19000;
    notUp["ms"] = -1;

    JsonObject sensors = device["sensors"].to<JsonObject>();
    JsonObject tableUp = sensors["TABLE_UP"].to<JsonObject>();
    tableUp["pin"] = 0;
    JsonObject tableDown = sensors["TABLE_DOWN"].to<JsonObject>();
    tableDown["pin"] = 1;
}

void fillGuillotineDevice(JsonObject device) {
    JsonObject guillotine = device["GUILLOTINE"].to<JsonObject>();
    guillotine["driver"] = "RMT";
    JsonObject pin = guillotine["pin"].to<JsonObject>();
    pin["step"] = 18;
    pin["dir"] = 4;
    pin["ena"] = 5;
    guillotine["DelayToEnable"] = 2000;
    guillotine["DelayToDisable"] = 6000;
    guillotine["MicroStep"] = 3200;
    JsonArray speed = guillotine["Speed"].to<JsonArray>();
    JsonObject normal = speed.add<JsonObject>();
    normal["Mode"] = "Normal";
    normal["Speed"] = 16000;
    normal["Acceleration"] = 32000;
    JsonObject slow = speed.add<JsonObject>();
    slow["Mode"] = "Slow";
    slow["Speed"] = 9600;
    slow["Acceleration"] = 9600;
    guillotine["WorkMove"] = 3200;
    JsonArray check = guillotine["Check"].to<JsonArray>();
    JsonObject notIn = check.add<JsonObject>();
    notIn["Mode"] = "GUILLOTINE_NOT_IN";
    notIn["step"] = 4000;
    notIn["ms"] = 1500;

    JsonObject sensors = device["sensors"].to<JsonObject>();
    JsonObject guillotineSensor = sensors["GUILLOTINE"].to<JsonObject>();
    guillotineSensor["pin"] = 2;
}

bool fillHeadDevices(JsonObject devices, Catalog::MachineType type) {
    if (!ensureSupportedMachine(type)) {
        return false;
    }

    JsonObject paper = devices["paper"].to<JsonObject>();
    paper["address"] = 33;
    paper["protocol"] = "mks";
    paper["required"] = true;

    JsonObject table = devices["table"].to<JsonObject>();
    table["address"] = 16;
    table["protocol"] = "stm.v1";
    table["required"] = true;
    fillTableDevice(table["device"].to<JsonObject>());

    JsonObject guillotine = devices["GUILLOTINE"].to<JsonObject>();
    guillotine["address"] = 17;
    guillotine["protocol"] = "stm.v1";
    guillotine["required"] = true;
    fillGuillotineDevice(guillotine["device"].to<JsonObject>());

    return true;
}

bool fillDevice(JsonObject device, Catalog::MachineType type) {
    MachineSpec spec = MachineSpec::get(type);
    if (spec.type() == Catalog::MachineType::UNKNOWN) {
        Log::E("[ConfigDefaults] MachineSpec is not implemented for type '%s'", Catalog::machineName(type).c_str());
        return false;
    }

    spec.fillDeviceDefaults(device);
    return true;
}

void fillTuning(JsonObject tuning) {
    tuning["EDGE_DISTANCE_mm"] = 34.2;
    tuning["SENSOR_DISTANCE_mm"] = 34.2;
    tuning["DELTA_mm"] = 0;
    tuning["MARK_LENGHT_mm"] = 3;
    tuning["OVER_mm"] = 1.5;
    tuning["DISTANCE_BETWEEN_MARKS_mm"] = 40;
    tuning["CUT_count"] = 12;
    tuning["PROFILE_WIDTH_step"] = 2000;
    tuning["PROFILE_COUNT_CUT"] = 5;
}

void fillTestProfiles(JsonArray profiles) {
    JsonObject profile = profiles.add<JsonObject>();
    profile["ID"] = 0;
    profile["NAME"] = "Профиль";
    profile["RATIO_mm"] = 50.6;
    profile["DESC"] = "Описание профиля";
    profile["LENGHT_mm"] = 450;
}

void fillTestTasks(JsonArray tasks) {
    JsonObject task = tasks.add<JsonObject>();
    task["ID"] = 0;
    task["NAME"] = "Задание";
    task["PROFILE_ID"] = 0;
    task["OVER_mm"] = 2;
    task["PRODUCT_mm"] = 49.5;
    task["FIRST_CUT_mm"] = 8;
    task["MARK_mm"] = 3;
    task["LASTCUT_mm"] = 0;
    task["MARK"] = 1;
}

} // namespace detail

bool build(JsonDocument& doc, const Options& options) {
    doc.clear();

    JsonObject root = doc.to<JsonObject>();
    root["config_version"] = options.configVersion;
    root["machine"] = Catalog::machineName(options.machine);
    root["name"] = options.name;
    root["group"] = options.group;

    JsonObject settings = root["settings"].to<JsonObject>();
    detail::fillSettings(settings, options.accessPoint);

    JsonObject can = root["can"].to<JsonObject>();
    detail::fillCan(can);

    JsonObject devices = root["devices"].to<JsonObject>();
    if (!detail::fillHeadDevices(devices, options.machine)) {
        return false;
    }

    JsonObject tuning = root["tuning"].to<JsonObject>();
    detail::fillTuning(tuning);

    return true;
}

bool buildData(JsonDocument& doc, bool withTestData) {
    doc.clear();

    JsonObject root = doc.to<JsonObject>();

    JsonArray profiles = root["profiles"].to<JsonArray>();
    JsonArray tasks = root["tasks"].to<JsonArray>();

    if (withTestData) {
        detail::fillTestProfiles(profiles);
        detail::fillTestTasks(tasks);
    }

    return true;
}

} // namespace ConfigDefaults
