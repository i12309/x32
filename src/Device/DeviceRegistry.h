#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Device/DeviceNode.h"

class CanRouter;

// DeviceRegistry - реестр соседей на CAN для головного ESP32.
// Он не создает локальные моторы/датчики и не знает пины оконечных устройств:
// только имя, адрес, протокол, required-флаг, payload для configure и статусы.
class DeviceRegistry {
public:
    static constexpr uint8_t MaxDevices = 12;
    static constexpr uint8_t MaxRoles = 12;

    struct CanSettings {
        int txPin = 17;
        int rxPin = 18;
        uint32_t bitrate = 500000;
        uint32_t heartbeatPeriodMs = 200;
        uint32_t heartbeatTimeoutMs = 1000;
        uint32_t taskTimeoutMs = 5000;
    };

    static DeviceRegistry& getInstance();

    // Читает can/devices из корневого config. Секция roles поддерживается как
    // необязательный override, но по умолчанию роль берется из имени device.
    bool loadFromConfig(JsonObjectConst root);

    // Привязывает роутер, через который registry сможет отправлять задания.
    void bindRouter(CanRouter* router);

    // Отправляет configure всем required device. На первом этапе это soft-call:
    // если шина еще не поднята, задания получают статус Rejected без падения boot.
    bool configureRequiredDevices();

    // Основной путь для Scene: роль преобразуется в device по имени или через optional config.roles.
    DeviceTaskId sendTask(Role role, DeviceCommand cmd, const DeviceParams& params, uint32_t timeoutMs = 0);

    // Диагностический и boot-путь: явное имя device из config.devices.
    DeviceTaskId sendTask(DeviceName name, DeviceCommand cmd, const DeviceParams& params, uint32_t timeoutMs = 0);

    // Низкоуровневый путь для диагностики: явный CAN-адрес device.
    DeviceTaskId sendTask(uint8_t address, DeviceCommand cmd, const DeviceParams& params, uint32_t timeoutMs = 0);

    // Отдельная отправка configure, потому что payload хранится ссылкой на JSON.
    DeviceTaskId sendConfigure(DeviceName name, uint32_t timeoutMs = 0);

    DeviceTaskStatus taskStatus(DeviceTaskId id) const;
    DeviceStatus status(DeviceName name) const;
    DeviceStatus status(Role role) const;
    bool allRequiredReady() const;

    DeviceNode* findByName(DeviceName name);
    DeviceNode* findByAddress(uint8_t address);
    DeviceNode* findByRole(Role role);
    const DeviceNode* findByName(DeviceName name) const;
    const DeviceNode* findByAddress(uint8_t address) const;
    const DeviceNode* findByRole(Role role) const;

    void markHeartbeat(uint8_t address, uint8_t protocolMajor, uint8_t protocolMinor, uint8_t state, uint8_t errorCode, DeviceTaskId activeTaskId);
    void markTaskStatus(DeviceTaskId taskId, DeviceTaskStatus status);
    void process(uint32_t nowMs);

    uint8_t count() const { return count_; }
    const DeviceNode* deviceAt(uint8_t index) const;
    const CanSettings& canSettings() const { return can_; }

private:
    struct RoleBinding {
        Role role = Role::Unknown;
        DeviceName deviceName = nullptr;
    };

    DeviceRegistry() = default;

    DeviceTaskId nextTaskId();
    DeviceTaskId queueRejected(DeviceNode* node);
    bool validateRequiredCapabilities(const DeviceNode& node) const;
    void addDefaultRoleBindings();
    DeviceName resolveRole(Role role) const;
    uint32_t timeoutOrDefault(uint32_t timeoutMs) const;

    DeviceNode devices_[MaxDevices];
    RoleBinding roles_[MaxRoles];
    uint8_t count_ = 0;
    uint8_t roleCount_ = 0;
    DeviceTaskId nextTaskId_ = 1;
    CanSettings can_;
    CanRouter* router_ = nullptr;
};
