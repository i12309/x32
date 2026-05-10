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

    return true;
}


} // namespace ConfigDefaults
