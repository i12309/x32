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
    settings["log"] = 1;
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

void fillHeadDevices(JsonObject devices, JsonObject roles) {
    JsonObject paper = devices["paper"].to<JsonObject>();
    paper["address"] = 33;
    paper["protocol"] = "mks";
    paper["required"] = true;
    JsonObject paperDevice = paper["device"].to<JsonObject>();
    paperDevice["role"] = "paper";
    paperDevice["note"] = "MKS configure payload is not finalized yet.";

    JsonObject motion = devices["motion"].to<JsonObject>();
    motion["address"] = 16;
    motion["protocol"] = "stm.v1";
    motion["required"] = true;
    JsonObject motionDevice = motion["device"].to<JsonObject>();
    motionDevice["role"] = "motion";
    motionDevice["note"] = "Temporary payload. Detailed realtime config still lives in root device during migration.";

    JsonObject panel = devices["panel"].to<JsonObject>();
    panel["address"] = 48;
    panel["protocol"] = "stm.v1";
    panel["required"] = false;
    JsonObject panelDevice = panel["device"].to<JsonObject>();
    panelDevice["role"] = "panel";

    roles["paper"] = "paper";
    roles["table"] = "motion";
    roles["guillotine"] = "motion";
    roles["motion"] = "motion";
    roles["panel"] = "panel";
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
    tuning["EDGE_DISTANCE_mm"] = 17;
    tuning["SENSOR_DISTANCE_mm"] = 17;
    tuning["DELTA_mm"] = 0;
    tuning["MARK_LENGHT_mm"] = 3;
    tuning["MARK_CENTER_DISTANCE_mm"] = 9.5;
    tuning["OFFSET_FIRSTCUT_MM"] = 8.0;
    tuning["OFFSET_TOL_MM"] = 0.15;
    tuning["OFFSET_MAX_ITER"] = 2;
    tuning["OVER_mm"] = 2;
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
    JsonObject roles = root["roles"].to<JsonObject>();
    detail::fillHeadDevices(devices, roles);

    JsonObject device = root["device"].to<JsonObject>();
    if (!detail::fillDevice(device, options.machine)) {
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
