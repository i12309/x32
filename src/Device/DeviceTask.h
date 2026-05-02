#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

using DeviceTaskId = uint32_t;
using DeviceName = const char*;

// Role - логическая роль в сценарии. Она не обязана совпадать с именем device
// в config: роль Guillotine может жить на device motion, а позже переехать на
// отдельный guillotine без переписывания Scene.
enum class Role : uint8_t {
    Unknown,
    Paper,
    Table,
    Guillotine,
    Panel,
    Motion,
    Check
};

// DeviceCommand описывает намерение верхнего уровня. Конечный device сам
// решает, какие моторы, датчики и энкодеры использовать внутри.
enum class DeviceCommand : uint8_t {
    Configure,
    SelfTest,
    Check,
    Stop,
    ResetError,
    PaperFeed,
    PaperFeedUntilMark,
    TableUp,
    TableDown,
    GuillotineCut,
    ProfileRun
};

// DeviceParams хранит маленькие параметры команд без heap-алокаций.
// Большие configure-payload передаются отдельным ConfigureTask.
struct DeviceParams {
    union {
        struct { float mm; uint16_t speedMmS; } paperFeed;
        struct { uint8_t profileId; } profileRun;
        struct { uint16_t cutSpeedMmS; } guillotineCut;
        struct { uint8_t reserved; } empty;
    } u = {};
};

// DeviceTask - частая структура runtime-задания. Здесь нет String и JsonDocument,
// чтобы задания можно было создавать без динамической памяти.
struct DeviceTask {
    DeviceTaskId id = 0;
    uint8_t deviceAddress = 0;
    DeviceCommand command = DeviceCommand::Check;
    DeviceParams params;
    uint32_t timeoutMs = 0;
};

// ConfigureTask держит ссылку на payload внутри Core::config.doc. Этот JSON
// живет весь runtime, поэтому payload не копируется и не аллоцируется заново.
struct ConfigureTask {
    DeviceTaskId id = 0;
    uint8_t deviceAddress = 0;
    JsonObjectConst payload;
    uint32_t timeoutMs = 0;
};

const char* roleName(Role role);
Role roleFromName(const char* name);
const char* commandName(DeviceCommand command);
