#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <string>
#include <vector>

#include "config.h"
#include "Machine/MachineSpec.h"
#include "Service/Log.h"

class T{
    public:
    typedef std::function<void(void)> THandlerFunction;
    typedef std::function<void(void* ptr)> THandlerPageFunction;

    // String conversion helpers (canonical type: std::string)
    static std::string toStd(const String& s) {
        return std::string(s.c_str());
    }
    static std::string toStd(const char* s) {
        return s ? std::string(s) : std::string();
    }
    static String toString(const std::string& s) {
        return String(s.c_str());
    }

    static bool isStringValidFloat(const char* str) {
        if (str == nullptr || *str == '\0') return false; // Проверка на пустую строку

        char* endPtr;
        float value = strtof(str, &endPtr); // Преобразование строки в float

        // Если endPtr указывает на начало строки, значит преобразование не удалось
        if (endPtr == str) {
            return false; // Строка не содержит допустимых чисел
        }

        // Проверка: если остались непреобразованные символы после числа
        while (*endPtr != '\0') {
            if (!isspace(*endPtr)) { // Пропускаем пробельные символы
                return false; // Недопустимые символы после числа
            }
            endPtr++;
        }

        return true; // Преобразование успешное
    }
    static bool isStringValidInteger(const String& str) {
        if (str.isEmpty()) return false; // Проверка на пустую строку

        bool hasDigit = false; // Флаг наличия хотя бы одной цифры
        int index = 0;

        // Проверка на знак (+ или -)
        if (str[index] == '+' || str[index] == '-') {
            index++;
        }

        // Проверка оставшейся части строки
        for (; index < str.length(); index++) {
            char c = str[index];

            // Разрешаем только цифры
            if (isdigit(c)) {
                hasDigit = true;
                continue;
            }

            // Все остальные символы недопустимы
            return false;
        }

        return hasDigit; // Должна быть хотя бы одна цифра
    }
};

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

        // Настройки CAN-шины головного устройства из секции config.CAN.
        // Здесь только параметры транспорта; логика BOOT/передачи конфига будет отдельным слоем.
        struct CanBusConfig {
            bool enabled = false;
            int tx = 27;
            int rx = 26;
            int bitrate = 500;
            uint32_t ackTimeoutMs = 150;
            uint32_t checkTimeoutMs = 300;
        };

        // Загруженный конфиг одной ноды из файла /node_<NAME>.json.
        // payload хранит весь JSON без форматирования, чтобы позже отправить его ноде по CAN.
        struct NodeConfig {
            String name;
            String path;
            String payload;
            String mac;
            uint16_t canID = 0;
            // Групповой CAN ID из верхнего поля node.group.
            // Нода слушает и свой индивидуальный CAN.canID, и этот групповой адрес.
            uint16_t groupID = 0;
            bool hasMac = false;
            bool hasCanID = false;
            bool hasGroupID = false;
        };

        CanBusConfig can;
        std::vector<NodeConfig> nodes;

        // true означает, что config.json описывает головное устройство CAN-сети,
        // а не старый монолит с полным локальным device.
        bool isCanCoordinator() const {
            return can.enabled || doc["nodes"].is<JsonArrayConst>();
        }

        // Поиск загруженной ноды по логическому имени из config.nodes.
        const NodeConfig* findNode(const String& nodeName) const {
            for (const NodeConfig& node : nodes) {
                if (node.name == nodeName) return &node;
            }
            return nullptr;
        }

        // Поиск ноды по MAC из BootHello. Сейчас реальные MAC могут отсутствовать,
        // поэтому заглушки 00:00... намеренно не участвуют в сопоставлении.
        const NodeConfig* findNodeByMac(const String& mac) const {
            for (const NodeConfig& node : nodes) {
                if (node.hasMac && node.mac == mac) return &node;
            }
            return nullptr;
        }

        // Разбирает CAN ID из JSON: поддерживает число или строку вида "0x201".
        // Ограничение 0x7FF соответствует стандартному 11-битному CAN identifier.
        static uint16_t parseCanID(JsonVariantConst value, bool& ok) {
            ok = false;
            if (value.isNull()) return 0;

            if (value.is<int>()) {
                int raw = value.as<int>();
                if (raw < 0 || raw > 0x7FF) return 0;
                ok = true;
                return static_cast<uint16_t>(raw);
            }

            const char* text = value | "";
            if (text == nullptr || text[0] == '\0') return 0;

            char* end = nullptr;
            unsigned long raw = strtoul(text, &end, 0);
            if (end == text || *end != '\0' || raw > 0x7FFUL) return 0;

            ok = true;
            return static_cast<uint16_t>(raw);
        }

        // Временные MAC-заглушки не считаются реальными идентификаторами железа.
        static bool isPlaceholderMac(const String& mac) {
            return mac.length() == 0 ||
                   mac == "00:00:00:00" ||
                   mac == "00:00:00:00:00:00";
        }

        // Единое правило имени файла ноды: /node_TABLE.json, /node_PAPER.json и т.д.
        static String nodePath(const String& nodeName) {
            return String("/node_") + nodeName + ".json";
        }

        // Загружает один файл ноды, проверяет минимально обязательные поля
        // и готовит payload для будущей передачи в эту ноду.
        bool loadNodeConfig(const String& nodeName, NodeConfig& out) {
            out = NodeConfig();
            out.name = nodeName;
            out.path = nodePath(nodeName);

            if (!LittleFS.exists(out.path)) {
                Log::E("[Config] Node '%s' listed but file '%s' is missing", nodeName.c_str(), out.path.c_str());
                return false;
            }

            File file = LittleFS.open(out.path, "r");
            if (!file) {
                Log::E("[Config] Failed to open node config '%s'", out.path.c_str());
                return false;
            }
            if (file.size() == 0) {
                file.close();
                Log::E("[Config] Node config '%s' is empty", out.path.c_str());
                return false;
            }

            JsonDocument nodeDoc;
            DeserializationError error = deserializeJson(nodeDoc, file);
            file.close();
            if (error) {
                Log::E("[Config] Failed to parse node config '%s': %s", out.path.c_str(), error.c_str());
                return false;
            }

            const String declaredName = nodeDoc["name"] | "";
            if (declaredName.length() > 0 && declaredName != nodeName) {
                Log::D("[Config] Node file '%s' name='%s' differs from listed node '%s'",
                       out.path.c_str(), declaredName.c_str(), nodeName.c_str());
            }

            bool groupOk = false;
            out.groupID = parseCanID(nodeDoc["group"], groupOk);
            out.hasGroupID = groupOk;
            if (!out.hasGroupID) {
                Log::E("[Config] Node '%s' has invalid or missing group", nodeName.c_str());
                return false;
            }

            JsonObjectConst canObj = nodeDoc["CAN"];
            if (canObj.isNull()) {
                Log::E("[Config] Node '%s' has no CAN section", nodeName.c_str());
                return false;
            }

            out.mac = canObj["mac"] | "";
            out.hasMac = !isPlaceholderMac(out.mac);
            if (!out.hasMac) {
                Log::D("[Config] Node '%s' has no real CAN.mac; BOOT cannot map hello MAC to this node yet",
                       nodeName.c_str());
            }

            bool canIDOk = false;
            out.canID = parseCanID(canObj["canID"], canIDOk);
            out.hasCanID = canIDOk;
            if (!out.hasCanID) {
                Log::E("[Config] Node '%s' has invalid or missing CAN.canID", nodeName.c_str());
                return false;
            }

            if (!nodeDoc["device"].is<JsonObjectConst>()) {
                Log::E("[Config] Node '%s' has no device section", nodeName.c_str());
                return false;
            }

            out.payload = "";
            serializeJson(nodeDoc, out.payload);
            return true;
        }

        // Загружает CAN-секцию головного конфига и все файлы нод из nodes[].
        // На этом этапе проверяется только структура и адресация, без создания устройств.
        bool loadCanConfig() {
            can = CanBusConfig();
            nodes.clear();

            JsonObjectConst canObj = doc["CAN"];
            can.enabled = !canObj.isNull() && ((canObj["enabled"] | 0) == 1);
            if (!canObj.isNull()) {
                can.tx = canObj["tx"] | can.tx;
                can.rx = canObj["rx"] | can.rx;
                can.bitrate = canObj["bitrate"] | can.bitrate;
                JsonObjectConst timeouts = canObj["timeouts_ms"];
                if (!timeouts.isNull()) {
                    can.ackTimeoutMs = timeouts["ack"] | can.ackTimeoutMs;
                    can.checkTimeoutMs = timeouts["check"] | can.checkTimeoutMs;
                }
            }

            JsonArrayConst nodeArray = doc["nodes"];
            if (nodeArray.isNull()) {
                return true;
            }

            bool ok = true;
            for (JsonVariantConst item : nodeArray) {
                const char* nodeNameRaw = item | "";
                String nodeName = nodeNameRaw ? String(nodeNameRaw) : String();
                nodeName.trim();
                if (nodeName.length() == 0) {
                    Log::E("[Config] Empty node name in nodes array");
                    ok = false;
                    continue;
                }
                if (findNode(nodeName) != nullptr) {
                    Log::E("[Config] Duplicate node '%s' in nodes array", nodeName.c_str());
                    ok = false;
                    continue;
                }

                NodeConfig node;
                if (!loadNodeConfig(nodeName, node)) {
                    ok = false;
                    continue;
                }

                for (const NodeConfig& existing : nodes) {
                    if (existing.canID == node.canID) {
                        Log::E("[Config] Duplicate CAN.canID 0x%s for nodes '%s' and '%s'",
                               String(node.canID, HEX).c_str(), existing.name.c_str(), node.name.c_str());
                        ok = false;
                    }
                    if (existing.hasMac && node.hasMac && existing.mac == node.mac) {
                        Log::E("[Config] Duplicate CAN.mac '%s' for nodes '%s' and '%s'",
                               node.mac.c_str(), existing.name.c_str(), node.name.c_str());
                        ok = false;
                    }
                }
                nodes.push_back(node);
            }

            Log::D("[Config] CAN: enabled=%d tx=%d rx=%d bitrate=%d nodes=%d",
                   can.enabled ? 1 : 0, can.tx, can.rx, can.bitrate, static_cast<int>(nodes.size()));
            return ok;
        }

        // записать дефолтный config.json
        bool writeDefault() {
            ConfigDefaults::Options options;
            if (!ConfigDefaults::build(doc, options)) {
                Log::D("Failed to build default config from MachineSpec");
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
                Log::D("Ошибка создания файла");
                return false;
            }
            serializeJsonPretty(doc, file);
            file.close();
            return true;
        }

        // открыть, если его нет, то создать по умолчанию (если разрешено)
        bool open(File& file, bool createDefaultIfMissing){
            if (!LittleFS.exists(CONFIG_PATH)) {
                if (!createDefaultIfMissing) {
                    Log::D("Файл '%s' не найден", CONFIG_PATH.c_str());
                    return false;
                }
                Log::D("Файл не найден, создаем из дефолтного");
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
                Log::D("Файл пустой, создаем из дефолтного");
                if (!writeDefault()) return false;
                file = LittleFS.open(CONFIG_PATH, "r");
                if (!file || file.size() == 0) {
                    Log::D("Ошибка открытия после создания");
                    return false;
                }
            }
            return true;
        }

        // Единый метод загрузки конфигурации
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

            // Загрузка переменной machine
            machine = doc["machine"] | "UNKNOWN";
            configVersion = doc["config_version"] | 0;
            name = doc["name"] | "ESP32";
            group = doc["group"] | "";

            // Загрузка основных параметров
            if (!settings.load(doc)) {
                Log::D("Ошибка загрузки секции settings");
                return false;
            }
            if (!loadCanConfig()) {
                Log::E("[Config] CAN config validation failed");
                return false;
            }
            if (isCanCoordinator()) {
                return true;
            }
            Catalog::MachineType machineType = Catalog::getMachine(machine);
            MachineSpec spec = MachineSpec::get(machineType);
            JsonObjectConst deviceObj = doc["device"];
            MachineSpec::Report report = spec.validateDeviceConfig(deviceObj);

            for (const String& warning : report.warnings) {
                Log::D("%s", warning.c_str());
            }
            if (report.hasErrors()) {
                for (const String& err : report.errors) {
                    Log::E("%s", err.c_str());
                }
                Log::E("[Config] MachineSpec validation failed for machine='%s'", machine.c_str());
                return false;
            }

            return true;
        }

        // сохранение
        bool save() {
            // Сохраняем переменные из корня config
            doc["machine"] = machine;
            doc["config_version"] = configVersion;
            doc["name"] = name;
            doc["group"] = group;

            JsonObject settingsObj = doc["settings"];
            settings.serialize(settingsObj);

            File file = LittleFS.open(CONFIG_PATH, "w");
            if (!file) {
                Log::D("Ошибка записи файла");
                return false;
            }
            serializeJsonPretty(doc, file);
            file.close();
            return true;
        }

        // Отладка: вывод конфига в терминал
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
        int AUTO_UPDATE; // автообновление
        String SERVER;
        String VERSION;
        String FIRMWARE;
        String HASH;
        int UPDATE;
        String UPDATE_URL;

        String TFT_VERSION;
        String TFT_FIRMWARE;
        int TFT_UPDATE;

        int CONNECT_WIFI;// 1
        int ACCESS_POINT;// 0
        int HTTP_SERVER;// 1
        int WEB;// 0
        int log;// 1

        int LOG_BUFFER;// 0

        int CHECK_SYSTEM;// 0
        int ALLOW_MISSING_HARDWARE;// 0
        int LICENCE_OFF;// 1

        int metrics;// 0 - статистика



        String buildLevel(const String& fileName, int level) const {
            switch (level) {
                case 1: return "http://" + SERVER + "/" + Core::config.machine + "/" + fileName;
                case 2: return "http://" + SERVER + "/" + Core::config.machine + "/" + Core::config.group + "/" + fileName;
                case 3: return "http://" + SERVER + "/" + Core::config.machine + "/" + Core::config.group + "/" + Core::config.name + "/" + fileName;
                default: return "";
            }
        }

        // Функции для сборки URL
        String getVersionURL(int level = 3) const { return buildLevel(VERSION, level); }
        String getFirmwareURL(int level = 3) const { return buildLevel(FIRMWARE, level); }
        String getHashURL(int level = 3) const { return buildLevel(HASH, level); }
        String getTFTVersionURL(int level = 3) const { return buildLevel(TFT_VERSION, level); }
        String getTFTFirmwareURL(int level = 3) const { return buildLevel(TFT_FIRMWARE, level); }

        // Загрузка секции settings
        bool load(const JsonDocument& doc) {

            JsonObjectConst obj = doc["settings"];
            if (obj.isNull()) {
                Log::D("Объект settings не найден");
                return false;
            }

            settings.log = obj["log"] | 1;
            Log::level = settings.log;

            settings.AUTO_UPDATE = obj["AUTO_UPDATE"] | 0;

            settings.SERVER = obj["SERVER"] | "";
            settings.VERSION = obj["VERSION"] | "";
            settings.FIRMWARE = obj["FIRMWARE"] | "";
            settings.HASH = obj["HASH"] | "";
            settings.UPDATE = obj["UPDATE"] | 0;
            settings.UPDATE_URL = obj["UPDATE_URL"] | "";

            settings.TFT_VERSION = obj["TFT_VERSION"] | "";
            settings.TFT_FIRMWARE = obj["TFT_FIRMWARE"] | "";
            settings.TFT_UPDATE = obj["TFT_UPDATE"] | 0;

            settings.CONNECT_WIFI = obj["CONNECT_WIFI"] | 0;
            settings.ACCESS_POINT = obj["ACCESS_POINT"] | 0;
            settings.HTTP_SERVER = obj["HTTP_SERVER"] | 0;
            // WEB строго бинарный флаг: 0 (выключен) или 1 (включен).
            // По умолчанию (если параметр отсутствует) - 0.
            settings.WEB = ((obj["WEB"] | 0) == 1) ? 1 : 0;

            settings.LOG_BUFFER = obj["LOG_BUFFER"] | 0;
            settings.CHECK_SYSTEM = obj["CHECK_SYSTEM"] | 0;
            settings.ALLOW_MISSING_HARDWARE = obj["ALLOW_MISSING_HARDWARE"] | 0;
            settings.LICENCE_OFF = obj["licence_off"] | 1;
            settings.metrics = obj["metrics"] | 0;
            // Если параметр отсутствует, используем тот же дефолт, что и в config defaults.

            return true;
        }

        void serialize(JsonObject& obj) {
            obj["AUTO_UPDATE"] = settings.AUTO_UPDATE;
            obj["SERVER"] = settings.SERVER;
            obj["VERSION"] = settings.VERSION;
            obj["FIRMWARE"] = settings.FIRMWARE;
            obj["HASH"] = settings.HASH;
            obj["UPDATE"] = settings.UPDATE;
            obj["UPDATE_URL"] = settings.UPDATE_URL;
            obj["TFT_VERSION"] = settings.TFT_VERSION;
            obj["TFT_FIRMWARE"] = settings.TFT_FIRMWARE;
            obj["TFT_UPDATE"] = settings.TFT_UPDATE;
            obj["CONNECT_WIFI"] = settings.CONNECT_WIFI;
            obj["ACCESS_POINT"] = settings.ACCESS_POINT;
            obj["HTTP_SERVER"] = settings.HTTP_SERVER;
            obj["WEB"] = settings.WEB;
            obj["log"] = settings.log;
            obj["LOG_BUFFER"] = settings.LOG_BUFFER;
            obj["CHECK_SYSTEM"] = settings.CHECK_SYSTEM;
            obj["ALLOW_MISSING_HARDWARE"] = settings.ALLOW_MISSING_HARDWARE;
            obj["licence_off"] = settings.LICENCE_OFF;
            obj["metrics"] = settings.metrics;
        }
    };
public: static Settings settings;
};
