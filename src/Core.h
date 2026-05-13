#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <string>
#include <vector>

#include "core/Helper.h"
#include "config.h"
#include "Machine/MachineSpec.h"
#include "Service/Log.h"

class FileSystem{ public:
    static bool init(bool formatOnFail = true) {
        if (!LittleFS.begin(true, "/spiffs")) {
            if (formatOnFail) {
                if (LittleFS.format()) {
                    Log::D("LittleFS отформатирована и инициализирована успешно");
                    return LittleFS.begin();
                } else {
                    Log::D("Ошибка форматирования LittleFS");
                    return false;
                }
            }
            Log::D("Ошибка инициализации LittleFS");
            return false;
        }
        Log::D("LittleFS инициализирован успешно");
        return true;
    }

    // Чтение JSON-файла и десериализация в структуру
    static bool readConfig(String& formattedJson) {
        return readJsonFile(CONFIG_PATH, formattedJson);
    }

    static bool readData(String& formattedJson) {
        return readJsonFile(DATA_PATH, formattedJson);
    }

    // Сохранение JSON-файла, сериализованного из структуры
    static bool saveConfig(String& data) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data);
        if (error) {
            Log::D("Ошибка парсинга config JSON: %s", error.c_str());
            return false;
        }

        if (!writeJsonDocument(CONFIG_PATH, doc)) {
            return false;
        }

        Log::D("JSON сохранен в файл '%s'", CONFIG_PATH.c_str());
        return true;
    }

    static bool saveData(String& data) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data);
        if (error) {
            Log::D("Ошибка парсинга data JSON: %s", error.c_str());
            return false;
        }

        if (!writeJsonDocument(DATA_PATH, doc)) {
            return false;
        }

        Log::D("JSON сохранен в файл '%s'", DATA_PATH.c_str());
        return true;
    }

    // Удаление файла
    static bool deleteFile(const char* filename) {
        if (!LittleFS.exists(filename)) {
            Serial.printf("Файл '%s' не существует\n", filename);
            return false;
        }

        if (LittleFS.remove(filename)) {
            Serial.printf("Файл '%s' удален\n", filename);
            return true;
        } else {
            Serial.printf("Ошибка удаления файла '%s'\n", filename);
            return false;
        }
    }

private:
    static bool readJsonDocument(const String& path, JsonDocument& doc) {
        if (!LittleFS.exists(path)) {
            return false;
        }

        File file = LittleFS.open(path, "r");
        if (!file) {
            return false;
        }

        doc.clear();
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        if (error) {
            return false;
        }
        return true;
    }

    static bool readJsonFile(const String& path, String& formattedJson) {
        JsonDocument doc;
        if (!readJsonDocument(path, doc)) {
            Log::D("Файл '%s' не найден или поврежден", path.c_str());
            return false;
        }

        serializeJsonPretty(doc, formattedJson);
        Log::D("JSON из файла '%s' успешно загружен", path.c_str());
        return true;
    }

    static bool writeJsonDocument(const String& path, const JsonDocument& doc) {
        File file = LittleFS.open(path, "w");
        if (!file) {
            Log::D("Ошибка открытия файла '%s' для записи", path.c_str());
            return false;
        }

        serializeJsonPretty(doc, file);
        file.close();
        return true;
    }
};
class Core {
private:
    struct DataJson {
        JsonDocument doc;

        bool load() {
            Log::D(__func__);

            doc.clear();
            if (!LittleFS.exists(DATA_PATH)) {
                return true;
            }

            File file = LittleFS.open(DATA_PATH, "r");
            if (!file) {
                Log::D("Ошибка открытия файла '%s'", DATA_PATH.c_str());
                return false;
            }

            if (file.size() == 0) {
                file.close();
                return true;
            }

            DeserializationError error = deserializeJson(doc, file);
            file.close();

            if (error) {
                Log::D("Ошибка парсинга data.json: %s", error.c_str());
                return false;
            }
            if (!doc["profiles"].is<JsonArray>()) {
                doc["profiles"].to<JsonArray>();
            }
            if (!doc["tasks"].is<JsonArray>()) {
                doc["tasks"].to<JsonArray>();
            }

            return true;
        }

        bool save() {
            File file = LittleFS.open(DATA_PATH, "w");
            if (!file) {
                Log::D("Ошибка записи data.json");
                return false;
            }
            serializeJsonPretty(doc, file);
            file.close();
            return true;
        }
    };
public: static DataJson data;

private:
    struct ConfigJson {
        JsonDocument doc;
        String machine;
        int configVersion;
        String name;
        String group;
        String mac;
        uint16_t canID = 0;
        struct GroupConfig {
            // Логическая CAN-группа из config.groups.<NAME>.
            String name;
            uint16_t id = 0;
            std::vector<String> nodes;
        };
        struct NodeConfig {
            // Кэш envelope-данных ноды после валидации /node_<NAME>.json.
            String name;
            String path;
            String mac;
            uint16_t canID = 0;
            uint16_t groupID = 0;
            String payload;
        };
        std::vector<GroupConfig> groups;
        std::vector<NodeConfig> nodes;
        static bool isDefaultMac(const String& value) {
            return value == "00:00:00:00";
        }
        static String nodePath(const String& nodeName) {
            return String("/node_") + nodeName + ".json";
        }
        bool hasGroupID(uint16_t id) const {
            if (id == 0) return false;
            for (const GroupConfig& groupCfg : groups) {
                if (groupCfg.id == id) return true;
            }
            return false;
        }
        bool groupHasNode(uint16_t id, const String& nodeName) const {
            if (id == 0) return false;
            for (const GroupConfig& groupCfg : groups) {
                if (groupCfg.id != id) continue;
                for (const String& member : groupCfg.nodes) {
                    if (member == nodeName) return true;
                }
                return false;
            }
            return false;
        }
        bool findNode(const String& nodeName, NodeConfig& out) const {
            for (const NodeConfig& node : nodes) {
                if (node.name == nodeName) {
                    out = node;
                    return true;
                }
            }
            return false;
        }
        static String normalizeMac(String value) {
            value.trim();
            value.toUpperCase();
            return value;
        }
        bool findNodeByMac(const String& macAddress, NodeConfig& out) const {
            const String target = normalizeMac(macAddress);
            if (target.length() == 0 || isDefaultMac(target)) return false;
            for (const NodeConfig& node : nodes) {
                if (!isDefaultMac(normalizeMac(node.mac)) &&
                    normalizeMac(node.mac) == target) {
                    out = node;
                    return true;
                }
            }
            return false;
        }
        bool nodeBootInfoByMac(const String& macAddress,
                               String& outName,
                               uint16_t& outCanID,
                               bool& outHasPayload) const {
            NodeConfig node;
            if (!findNodeByMac(macAddress, node)) return false;
            outName = node.name;
            outCanID = node.canID;
            outHasPayload = node.payload.length() > 0;
            return true;
        }
        bool nodeAddress(const char* nodeName, uint16_t& out) const {
            out = 0;
            if (nodeName == nullptr || nodeName[0] == '\0') return false;
            for (const NodeConfig& node : nodes) {
                if (node.name == nodeName) {
                    out = node.canID;
                    return canfw::isNonZeroCanId(out);
                }
            }
            return false;
        }
        bool nodeGroup(const char* nodeName, uint16_t& out) const {
            out = 0;
            if (nodeName == nullptr || nodeName[0] == '\0') return false;
            for (const NodeConfig& node : nodes) {
                if (node.name == nodeName) {
                    out = node.groupID;
                    return canfw::isNonZeroCanId(out);
                }
            }
            return false;
        }
        bool loadGroups(JsonObjectConst groupsObj) {
            // Формат ожидается строго как:
            // "groups": { "FEED": { "canID": "0x220", "nodes": ["PAPER","THROW"] } }
            groups.clear();
            if (groupsObj.isNull()) return true;
            for (JsonPairConst item : groupsObj) {
                String groupName = item.key().c_str();
                groupName.trim();
                if (groupName.length() == 0) {
                    Log::E("[Config] groups contains empty name");
                    return false;
                }

                JsonObjectConst groupObj = item.value().as<JsonObjectConst>();
                if (groupObj.isNull()) {
                    Log::E("[Config] group '%s' must be an object", groupName.c_str());
                    return false;
                }

                uint16_t groupId = 0;
                if (!canfw::parseCanId(groupObj["canID"] | "", groupId) ||
                    !canfw::isNonZeroCanId(groupId)) {
                    Log::E("[Config] group '%s' has invalid CAN ID", groupName.c_str());
                    return false;
                }

                JsonArrayConst groupNodes = groupObj["nodes"];
                if (groupNodes.isNull() || groupNodes.size() == 0) {
                    Log::E("[Config] group '%s' must contain non-empty nodes array", groupName.c_str());
                    return false;
                }
                for (const GroupConfig& existed : groups) {
                    if (existed.name == groupName) {
                        Log::E("[Config] duplicate group name in config.groups: %s", groupName.c_str());
                        return false;
                    }
                    if (existed.id == groupId) {
                        Log::E("[Config] duplicate group CAN ID 0x%s: %s and %s",
                               String(groupId, HEX).c_str(),
                               existed.name.c_str(),
                               groupName.c_str());
                        return false;
                    }
                }
                GroupConfig groupCfg;
                groupCfg.name = groupName;
                groupCfg.id = groupId;
                for (JsonVariantConst nodeItem : groupNodes) {
                    if (!nodeItem.is<const char*>()) {
                        Log::E("[Config] group '%s' has non-string node entry", groupName.c_str());
                        return false;
                    }
                    String nodeName = nodeItem.as<String>();
                    nodeName.trim();
                    if (nodeName.length() == 0) {
                        Log::E("[Config] group '%s' has empty node name", groupName.c_str());
                        return false;
                    }
                    groupCfg.nodes.push_back(nodeName);
                }
                groups.push_back(groupCfg);
            }
            return true;
        }
        bool loadNodeFile(const String& expectedName, NodeConfig& out) {
            const String path = nodePath(expectedName);
            if (!LittleFS.exists(path)) {
                Log::E("[Config] node file is missing: %s", path.c_str());
                return false;
            }
            File file = LittleFS.open(path, "r");
            if (!file) {
                Log::E("[Config] failed to open node file: %s", path.c_str());
                return false;
            }
            if (file.size() == 0) {
                file.close();
                Log::E("[Config] node file is empty: %s", path.c_str());
                return false;
            }
            JsonDocument nodeDoc;
            DeserializationError error = deserializeJson(nodeDoc, file);
            file.close();
            if (error) {
                Log::E("[Config] failed to parse node file '%s': %s", path.c_str(), error.c_str());
                return false;
            }
            const String machineName = nodeDoc["machine"] | "";
            if (machineName != "NODE") {
                Log::E("[Config] node %s has machine='%s', expected 'NODE'",
                       path.c_str(),
                       machineName.c_str());
                return false;
            }
            String nodeName = nodeDoc["name"] | "";
            nodeName.trim();
            if (nodeName.length() == 0) {
                Log::E("[Config] node %s has empty name", path.c_str());
                return false;
            }
            if (nodeName != expectedName) {
                Log::E("[Config] node name mismatch: list='%s', file='%s'",
                       expectedName.c_str(),
                       nodeName.c_str());
                return false;
            }
            if (!nodeDoc["mac"].is<const char*>()) {
                Log::E("[Config] node %s has invalid mac field", path.c_str());
                return false;
            }
            String nodeMac = nodeDoc["mac"].as<String>();
            nodeMac.trim();
            uint16_t nodeCanID = 0;
            if (!canfw::parseCanId(nodeDoc["canID"] | "", nodeCanID) ||
                !canfw::isNonZeroCanId(nodeCanID)) {
                Log::E("[Config] node %s has invalid canID", path.c_str());
                return false;
            }
            uint16_t nodeGroupID = 0;
            if (!canfw::parseCanId(nodeDoc["group"] | "", nodeGroupID)) {
                Log::E("[Config] node %s has invalid group", path.c_str());
                return false;
            }
            if (nodeGroupID != 0 && !hasGroupID(nodeGroupID)) {
                Log::E("[Config] node %s uses group 0x%s but config.groups does not declare it",
                       expectedName.c_str(),
                       String(nodeGroupID, HEX).c_str());
                return false;
            }
            if (nodeGroupID != 0 && !groupHasNode(nodeGroupID, nodeName)) {
                Log::E("[Config] node %s uses group 0x%s but is not listed in group nodes[]",
                       expectedName.c_str(),
                       String(nodeGroupID, HEX).c_str());
                return false;
            }
            out.name = nodeName;
            out.path = path;
            out.mac = normalizeMac(nodeMac);
            out.canID = nodeCanID;
            out.groupID = nodeGroupID;
            serializeJson(nodeDoc, out.payload);
            return true;
        }
        bool loadNodes(JsonArrayConst nodeArray) {
            nodes.clear();
            if (nodeArray.isNull() || nodeArray.size() == 0) {
                Log::E("[Config] nodes must be a non-empty array");
                return false;
            }
            std::vector<String> expectedNames;
            for (JsonVariantConst item : nodeArray) {
                if (!item.is<const char*>()) {
                    Log::E("[Config] nodes array must contain only strings");
                    return false;
                }
                String nodeName = item.as<String>();
                nodeName.trim();
                if (nodeName.length() == 0) {
                    Log::E("[Config] nodes array contains empty name");
                    return false;
                }
                for (const String& existed : expectedNames) {
                    if (existed == nodeName) {
                        Log::E("[Config] duplicate node name in config.nodes: %s", nodeName.c_str());
                        return false;
                    }
                }
                expectedNames.push_back(nodeName);
            }
            for (const String& nodeName : expectedNames) {
                NodeConfig nodeCfg;
                if (!loadNodeFile(nodeName, nodeCfg)) return false;
                nodes.push_back(nodeCfg);
            }
            for (size_t i = 0; i < nodes.size(); ++i) {
                const NodeConfig& current = nodes[i];
                for (size_t j = i + 1; j < nodes.size(); ++j) {
                    const NodeConfig& next = nodes[j];
                    if (current.canID == next.canID) {
                        Log::E("[Config] duplicate CAN ID 0x%s: %s and %s",
                               String(current.canID, HEX).c_str(),
                               current.name.c_str(),
                               next.name.c_str());
                        return false;
                    }
                    const String currentMac = normalizeMac(current.mac);
                    const String nextMac = normalizeMac(next.mac);
                    if (!isDefaultMac(currentMac) &&
                        !isDefaultMac(nextMac) &&
                        currentMac == nextMac) {
                        Log::E("[Config] duplicate node MAC '%s': %s and %s",
                               currentMac.c_str(),
                               current.name.c_str(),
                               next.name.c_str());
                        return false;
                    }
                }
                if (hasGroupID(current.canID)) {
                    Log::E("[Config] node %s CAN ID 0x%s conflicts with group ID",
                           current.name.c_str(),
                           String(current.canID, HEX).c_str());
                    return false;
                }
            }
            return true;
        }
        bool loadCanTopologyConfig() {
            groups.clear();
            nodes.clear();
            if (!loadGroups(doc["groups"].as<JsonObjectConst>())) return false;
            if (!loadNodes(doc["nodes"].as<JsonArrayConst>())) return false;
            std::vector<MachineSpec::NodeInfo> nodeInfos;
            nodeInfos.reserve(nodes.size());
            for (const NodeConfig& node : nodes) {
                MachineSpec::NodeInfo info;
                info.name = node.name;
                info.canID = node.canID;
                info.groupID = node.groupID;
                nodeInfos.push_back(info);
            }
            MachineSpec spec = MachineSpec::get(Catalog::getMachine(machine));
            MachineSpec::Report report =
                spec.validateControllerConfig(doc.as<JsonObjectConst>(), nodeInfos);
            for (const String& warning : report.warnings) {
                Log::D("%s", warning.c_str());
            }
            for (const String& error : report.errors) {
                Log::E("%s", error.c_str());
            }
            return !report.hasErrors();
        }
        bool ensureNodeFiles(Catalog::MachineType machineType, bool overwriteExisting) {
            JsonArrayConst nodeArray = doc["nodes"];
            for (JsonVariantConst item : nodeArray) {
                if (!item.is<const char*>()) continue;
                String nodeName = item.as<String>();
                nodeName.trim();
                if (nodeName.length() == 0) continue;

                const String path = nodePath(nodeName);
                if (!overwriteExisting && LittleFS.exists(path)) continue;

                JsonDocument nodeDoc;
                if (!ConfigDefaults::buildNode(nodeDoc, machineType, nodeName)) {
                    Log::E("[Config] failed to build default node config: %s", nodeName.c_str());
                    return false;
                }

                File nodeFile = LittleFS.open(path, "w");
                if (!nodeFile) {
                    Log::E("[Config] failed to create node config file: %s", path.c_str());
                    return false;
                }
                serializeJsonPretty(nodeDoc, nodeFile);
                nodeFile.close();
            }
            return true;
        }
        bool writeDefault() {
            ConfigDefaults::Options options;
            if (!ConfigDefaults::build(doc, options)) {
                Log::D("Ошибка сборки дефолтного config из MachineSpec");
                return false;
            }
            machine = Catalog::machineName(options.machine);
            configVersion = options.configVersion;
            name = options.name;
            group = options.group;
            if (!settings.load(doc)) {
                Log::D("Ошибка загрузки settings из дефолтного config");
                return false;
            }
            File file = LittleFS.open(CONFIG_PATH, "w");
            if (!file) {
                Log::D("Ошибка создания config файла");
                return false;
            }
            serializeJsonPretty(doc, file);
            file.close();
            return ensureNodeFiles(options.machine, true);
        }
        bool open(File& file, bool createDefaultIfMissing){
            if (!LittleFS.exists(CONFIG_PATH)) {
                if (!createDefaultIfMissing) {
                    Log::D("Файл '%s' не найден", CONFIG_PATH.c_str());
                    return false;
                }
                Log::D("Файл config отсутствует, создаем дефолтный");
                if (!writeDefault()) return false;
            }
            file = LittleFS.open(CONFIG_PATH, "r");
            if (!file) {
                Log::D("Ошибка открытия файла '%s'", CONFIG_PATH.c_str());
                return false;
            }
            if (file.size() == 0) {
                file.close();
                if (!createDefaultIfMissing) {
                    Log::D("Файл '%s' пустой", CONFIG_PATH.c_str());
                    return false;
                }
                Log::D("Файл config пустой, создаем дефолтный");
                if (!writeDefault()) return false;
                file = LittleFS.open(CONFIG_PATH, "r");
                if (!file || file.size() == 0) {
                    Log::D("Ошибка открытия дефолтного config после создания");
                    return false;
                }
            }
            return true;
        }
        bool load(bool createDefaultIfMissing = true) {
            Log::D(__func__);
            File file;
            if (!open(file, createDefaultIfMissing)) return false;
            DeserializationError error = deserializeJson(doc, file);
            file.close();
            if (error) {
                Log::D("Ошибка парсинга config.json: %s", error.c_str());
                return false;
            }
            machine = doc["machine"] | "UNKNOWN";
            configVersion = doc["config_version"] | 0;
            name = doc["name"] | "ESP32";
            group = doc["group"] | "";
            mac = doc["mac"] | "";
            canID = 0;
            canfw::parseCanId(doc["canID"] | "", canID);
            if (!settings.load(doc)) {
                Log::D("Ошибка загрузки секции settings");
                return false;
            }
            if (!loadCanTopologyConfig()) {
                Log::E("[Config] CAN topology validation failed");
                return false;
            }
            return true;
        }
        bool save() {
            doc["machine"] = machine;
            doc["config_version"] = configVersion;
            doc["name"] = name;
            doc["group"] = group;
            doc["mac"] = mac;
            doc["canID"] = String("0x") + String(canID, HEX);
            JsonObject settingsObj = doc["settings"];
            settings.serialize(settingsObj);
            File file = LittleFS.open(CONFIG_PATH, "w");
            if (!file) {
                Log::D("Ошибка записи config файла");
                return false;
            }
            serializeJsonPretty(doc, file);
            file.close();
            return ensureNodeFiles(Catalog::getMachine(machine), false);
        }
        void print_config() {
            if (Log::level == 1){
                Log::L("========== CONFIG DEBUG ==========");
                serializeJsonPretty(doc, Serial);
                Serial.println();
                Log::L("==================================");
            }
        }
    };
public: static ConfigJson config;

private: struct Settings {
        int AUTO_UPDATE;
        int UPDATE;
        String UPDATE_URL;
        String MQTT_SERVER;
        int CONNECT_WIFI;
        int ACCESS_POINT;
        int HTTP_SERVER;
        int CHECK_SYSTEM;
        int log;
        int metrics;

        // Загружает секцию settings из config.json.
        bool load(const JsonDocument& doc) {
            JsonObjectConst obj = doc["settings"];
            if (obj.isNull()) {
                Log::D("Объект settings не найден");
                return false;
            }

            settings.AUTO_UPDATE = obj["AUTO_UPDATE"] | 0;
            settings.UPDATE = obj["UPDATE"] | 0;
            settings.UPDATE_URL = obj["UPDATE_URL"] | "";
            settings.MQTT_SERVER = obj["MQTT_SERVER"] | "";
            settings.CONNECT_WIFI = obj["CONNECT_WIFI"] | 0;
            settings.ACCESS_POINT = obj["ACCESS_POINT"] | 0;
            settings.HTTP_SERVER = obj["HTTP_SERVER"] | 0;
            settings.CHECK_SYSTEM = obj["CHECK_SYSTEM"] | 0;
            settings.log = obj["log"] | 1;
            settings.metrics = obj["metrics"] | 1;
            Log::level = settings.log;
            return true;
        }

        // Сохраняет только поля новой структуры settings.
        void serialize(JsonObject& obj) {
            obj["AUTO_UPDATE"] = settings.AUTO_UPDATE;
            obj["UPDATE"] = settings.UPDATE;
            obj["UPDATE_URL"] = settings.UPDATE_URL;
            obj["MQTT_SERVER"] = settings.MQTT_SERVER;
            obj["CONNECT_WIFI"] = settings.CONNECT_WIFI;
            obj["ACCESS_POINT"] = settings.ACCESS_POINT;
            obj["HTTP_SERVER"] = settings.HTTP_SERVER;
            obj["CHECK_SYSTEM"] = settings.CHECK_SYSTEM;
            obj["log"] = settings.log;
            obj["metrics"] = settings.metrics;
        }
    };
public: static Settings settings;
};


