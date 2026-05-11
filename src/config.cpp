#include "config.h"
#include "Machine/MachineSpec.h"

namespace ConfigDefaults {

namespace detail {

void fillSettings(JsonObject settings, bool accessPoint) {
    settings["AUTO_UPDATE"] = 0;
    settings["UPDATE"] = 1;
    settings["UPDATE_URL"] = "http://45.9.43.227/update/";
    settings["MQTT_SERVER"] = "45.9.43.227";
    settings["CONNECT_WIFI"] = 1;
    settings["ACCESS_POINT"] = accessPoint ? 1 : 0;
    settings["HTTP_SERVER"] = 1;
    settings["CHECK_SYSTEM"] = 1;
    settings["log"] = 1;
    settings["metrics"] = 1;
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

} // namespace detail

bool build(JsonDocument& doc, const Options& options) {
    doc.clear();

    JsonObject root = doc.to<JsonObject>();
    root["config_version"] = options.configVersion;
    root["machine"] = Catalog::machineName(options.machine);
    root["name"] = options.name;
    root["group"] = options.group;
    root["mac"] = "00:00:00:00";
    root["canID"] = "0x200";

    JsonObject settings = root["settings"].to<JsonObject>();
    detail::fillSettings(settings, options.accessPoint);

    JsonObject tuning = root["tuning"].to<JsonObject>();
    detail::fillTuning(tuning);

    JsonArray nodes = root["nodes"].to<JsonArray>();
    if (options.machine == Catalog::MachineType::A) {
        nodes.add("TABLE");
        nodes.add("GUILLOTINE");
        nodes.add("PAPER");
        nodes.add("THROW");
    }

    JsonObject groups = root["groups"].to<JsonObject>();
    if (options.machine == Catalog::MachineType::A) {
        JsonObject feed = groups["FEED"].to<JsonObject>();
        feed["canID"] = "0x220";
        JsonArray feedNodes = feed["nodes"].to<JsonArray>();
        feedNodes.add("PAPER");
        feedNodes.add("THROW");
    }

    return true;
}

bool buildNode(JsonDocument& doc, Catalog::MachineType machine, const String& nodeName) {
    doc.clear();

    if (machine != Catalog::MachineType::A) {
        return false;
    }

    uint16_t canID = 0;
    const char* group = "0x000";
    if (nodeName == "TABLE") {
        canID = 0x201;
    } else if (nodeName == "GUILLOTINE") {
        canID = 0x202;
    } else if (nodeName == "PAPER") {
        canID = 0x203;
        group = "0x220";
    } else if (nodeName == "THROW") {
        canID = 0x204;
        group = "0x220";
    } else {
        return false;
    }

    JsonObject root = doc.to<JsonObject>();
    root["config_version"] = 0;
    root["machine"] = "NODE";
    root["name"] = nodeName;
    root["group"] = group;
    root["mac"] = "00:00:00:00";
    root["canID"] = String("0x") + String(canID, HEX);
    root["settings"].to<JsonObject>();

    root["device"].to<JsonObject>();
    return true;
}


} // namespace ConfigDefaults
