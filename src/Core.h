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
        String mac;
        uint16_t canID = 0;

        // Настройки CAN-шины головного устройства из секции config.CAN.
        // Здесь только параметры транспорта; логика BOOT/передачи конфига будет отдельным слоем.
        struct CanBusConfig {
            int tx = 17;
            int rx = 18;
            int bitrate = 500;
            uint32_t ackTimeoutMs = 150;
            uint32_t checkTimeoutMs = 300;
        };

        // Загруженный конфиг одной ноды из файла /node_<NAME>.json.
        // payload хранит весь JSON без форматирования, чтобы позже отправить его ноде по CAN.
        CanBusConfig can;
        std::vector<String> nodes;
        std::vector<String> nodePayloads;

        // Поиск загруженной ноды по логическому имени из config.nodes.
        // Разбирает CAN ID из JSON: поддерживает число или строку вида "0x201".
        // Ограничение 0x7FF соответствует стандартному 11-битному CAN identifier.
        static uint16_t parseCanID(JsonVariantConst value) {

            if (value.isNull()) return 0;

            if (value.is<int>()) {
                int raw = value.as<int>();
                if (raw < 0 || raw > 0x7FF) return 0;
                return static_cast<uint16_t>(raw);
            }

            const char* text = value | "";
            if (text == nullptr || text[0] == '\0') return 0;

            char* end = nullptr;
            unsigned long raw = strtoul(text, &end, 0);
            if (end == text || *end != '\0' || raw > 0x7FFUL) return 0;

            return static_cast<uint16_t>(raw);
        }

        // Единое правило имени файла ноды: /node_TABLE.json, /node_PAPER.json и т.д.
        static String nodePath(const String& nodeName) {
            return String("/node_") + nodeName + ".json";
        }

        // Загружает один файл ноды и готовит payload для передачи дальше.
        bool loadNodePayload(const String& nodeName, String& payload) {
            String path = nodePath(nodeName);

            if (!LittleFS.exists(path)) {
                Log::E("[Config] Node '%s' listed but file '%s' is missing", nodeName.c_str(), path.c_str());
                return false;
            }

            File file = LittleFS.open(path, "r");
            if (!file) {
                Log::E("[Config] Failed to open node config '%s'", path.c_str());
                return false;
            }
            if (file.size() == 0) {
                file.close();
                Log::E("[Config] Node config '%s' is empty", path.c_str());
                return false;
            }

            JsonDocument nodeDoc;
            DeserializationError error = deserializeJson(nodeDoc, file);
            file.close();
            if (error) {
                Log::E("[Config] Failed to parse node config '%s': %s", path.c_str(), error.c_str());
                return false;
            }

            payload = "";
            serializeJson(nodeDoc, payload);
            return true;
            }

        // Загружает CAN-секцию головного конфига и все файлы нод из nodes[].
        bool loadCanConfig() {
            can = CanBusConfig();
            nodes.clear();
            nodePayloads.clear();

            JsonObjectConst canObj = doc["CAN"];
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
                String payload;
                if (!loadNodePayload(nodeName, payload)) {
                    ok = false;
                    continue;
                }

                nodes.push_back(nodeName);
                nodePayloads.push_back(payload);
            }

            Log::D("[Config] CAN: tx=%d rx=%d bitrate=%d nodes=%d",
                   can.tx, can.rx, can.bitrate, static_cast<int>(nodes.size()));
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
            mac = doc["mac"] | "";
            canID = parseCanID(doc["canID"]);

            // Загрузка основных параметров
            if (!settings.load(doc)) {
                Log::D("Ошибка загрузки секции settings");
                return false;
            }
            if (!loadCanConfig()) {
                Log::E("[Config] CAN config validation failed");
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
            doc["mac"] = mac;
            doc["canID"] = String("0x") + String(canID, HEX);

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
