#pragma once

#include "App/App.h"
#include "Can/CanRouter.h"
#include "Catalog.h"
#include "Core.h"
#include "Data.h"
#include "Device/DeviceRegistry.h"
#include "Service/ESPUpdate.h"
#include "Service/HServer.h"
#include "Service/MQTTC.h"
#include "Service/NVS.h"
#include "Service/Stats.h"
#include "Service/WiFiConfig.h"
#include "State/State.h"
#include "Screen/Page/Main/load.h"
#include "version.h"

class Boot : public State {
private:
    struct BootContext {
        bool wifiConnected = false;
        bool displayReady = false;
        bool abortRequested = false;
        bool haltRequested = false;
        bool waitingRequiredDevices = false;
        uint32_t waitStartedMs = 0;
        State::Type abortState = State::Type::NULL_STATE;
        int labelNum = 0;
    };

    static BootContext& context() {
        static BootContext ctx;
        return ctx;
    }

    static Screen::Main::load& loadPage() {
        static Screen::Main::load page(App::panel());
        return page;
    }

    static void resetContext() {
        context() = BootContext();
    }

    static void requestAbort(State::Type state) {
        BootContext& ctx = context();
        ctx.abortRequested = true;
        ctx.abortState = state;
    }

    static void requestHalt() {
        context().haltRequested = true;
    }

    static void setStatus(const String& text) {
        BootContext& ctx = context();
        Log::D("BOOT: %s", text.c_str());

        if (!ctx.displayReady) return;

        ctx.labelNum++;
        loadPage().setStatus(text);
        if (ctx.labelNum - 1 > 0) {
            loadPage().setProgressColor(ctx.labelNum - 1, Catalog::Color::green);
        }
        loadPage().setProgressColor(ctx.labelNum, Catalog::Color::orange);
    }

    static void setStatusFail(const String& text) {
        BootContext& ctx = context();
        Log::E("BOOT: %s", text.c_str());

        if (!ctx.displayReady) return;

        loadPage().setStatus(text);
        loadPage().setProgressColor(ctx.labelNum, Catalog::Color::red);
    }

public:
    Boot() : State(State::Type::BOOT) {}

    void oneRun() override {
        State::oneRun();
        resetContext();

        PlanManager& plan = App::plan();
        plan.beginPlan(this->type());

        plan.addAction(State::Type::ACTION, &Boot::LogStart, "LogStart");
        plan.addAction(State::Type::ACTION, &Boot::InitBoard, "InitBoard");
        plan.addAction(State::Type::ACTION, &Boot::InitDisplay, "InitDisplay");
        plan.addAction(State::Type::ACTION, &Boot::ShowLoad, "ShowLoad");
        plan.addAction(State::Type::ACTION, &Boot::InitFileSystem, "InitFileSystem");
        plan.addAction(State::Type::ACTION, &Boot::InitNVS, "InitNVS");
        plan.addAction(State::Type::ACTION, &Boot::LoadConfig, "LoadConfig");
        plan.addAction(State::Type::ACTION, &Boot::InitDeviceRegistry, "InitDeviceRegistry");
        plan.addAction(State::Type::ACTION, &Boot::InitCanBus, "InitCanBus");
        plan.addAction(State::Type::ACTION, &Boot::ConfigureDevices, "ConfigureDevices");
        plan.addAction(State::Type::ACTION, &Boot::WaitDevicesReady, "WaitDevicesReady");
        plan.addAction(State::Type::ACTION, &Boot::ConnectWiFi, "ConnectWiFi");
        plan.addAction(State::Type::ACTION, &Boot::UpdateFirmware, "UpdateFirmware");
        plan.addAction(State::Type::ACTION, &Boot::StartHttp, "StartHttp");
        plan.addAction(State::Type::ACTION, &Boot::ConnectMQTT, "ConnectMQTT");
        plan.addAction(State::Type::ACTION, &Boot::LoadData, "LoadData");
        plan.addAction(State::Type::ACTION, &Boot::RegisterRuntimeEvents, "RegisterRuntimeEvents");
    }

    State* run() override {
        PlanManager& plan = App::plan();
        BootContext& boot = context();

        if (boot.haltRequested) {
            plan.clear();
            return this;
        }

        if (boot.abortRequested) {
            plan.clear();
            return Factory(boot.abortState);
        }

        if (boot.waitingRequiredDevices) {
            State* waitResult = waitForRequiredDevices(this);
            if (waitResult != nullptr) return waitResult;
        }

        if (!plan.hasPending()) {
            Log::L(" === END LOAD v.%s", Version::makeDeviceVersion(-1).c_str());
            Data::param.reset();
            App::ctx().reg.reset();
            App::mode() = Mode::NORMAL;
            App::diag().clearErrors();
            ESPUpdate::getInstance().markCurrentFirmwareValid();
            if (*App::cfg().checkSystem == 1) {
                setStatus("Check system");
                return Factory(State::Type::CHECK);
            }
            return Factory(State::Type::IDLE);
        }

        return Factory(plan.nextType(this->type()));
    }

private:
    static State* waitForRequiredDevices(State* self) {
        BootContext& boot = context();
        DeviceRegistry& devices = App::devices();

        if (devices.allRequiredReady()) {
            boot.waitingRequiredDevices = false;
            setStatus("Required devices ready");
            return nullptr;
        }

        const uint32_t now = millis();
        const uint32_t timeoutMs = devices.canSettings().taskTimeoutMs + devices.canSettings().heartbeatTimeoutMs;
        if (now - boot.waitStartedMs < timeoutMs) {
            return self;
        }

        for (uint8_t i = 0; i < devices.count(); ++i) {
            const DeviceNode* node = devices.deviceAt(i);
            if (node == nullptr || !node->required) continue;
            const DeviceStatus& status = node->status;
            if (status.online && status.ready && !status.error && !status.incompatible) continue;

            String message = String("Required device not ready: ") + String(node->name ? node->name : "unknown");
            if (status.errorText.length() > 0) message += String(" (") + status.errorText + ")";
            setStatusFail(message);
            Log::E("[BOOT] device=%s online=%d ready=%d error=%d incompatible=%d",
                   node->name ? node->name : "unknown",
                   status.online ? 1 : 0,
                   status.ready ? 1 : 0,
                   status.error ? 1 : 0,
                   status.incompatible ? 1 : 0);
        }

        boot.waitingRequiredDevices = false;
        App::diag().addError(State::ErrorCode::CONFIG_ERROR, "Required CAN device is not ready", "", false);
        return self->Factory(State::Type::NULL_STATE);
    }

    static bool LogStart() {
        Log::L(" === START v.%s", Version::makeDeviceVersion(-1).c_str());
        Log::D("ESP32 Chip: %s Rev %d", ESP.getChipModel(), ESP.getChipRevision());
        Log::D("Flash size: %d", ESP.getFlashChipSize());
        return true;
    }

    static bool InitBoard() {
        setStatus("Init board");
        return true;
    }

    static bool InitDisplay() {
        setStatus("Init display");
        BootContext& boot = context();
        boot.displayReady = App::panel().init();
        if (!boot.displayReady) {
            Log::E("[BOOT] Screen panel init failed. Continue without display.");
        }
        return true;
    }

    static bool ShowLoad() {
        if (!context().displayReady) return true;
        loadPage().show();
        setStatus("Load screen");
        return true;
    }

    static bool InitFileSystem() {
        setStatus("Init file system");
        if (FileSystem::init(true)) return true;

        setStatusFail("File system init failed");
        requestAbort(State::Type::NULL_STATE);
        return true;
    }

    static bool InitNVS() {
        setStatus("Init NVS");
        NVS& nvs = NVS::getInstance();
        nvs.init();

        int bootCount = nvs.getInt("boot_count", 0, "boot");
        bootCount++;
        if (bootCount > 3) {
            if (nvs.getInt("ota_pending", 0, "boot") == 1) {
                setStatusFail("OTA boot failed, rollback");
                if (ESPUpdate::getInstance().rollbackToPreviousPartition("boot_count exceeded")) {
                    requestHalt();
                    return true;
                }
            }
            nvs.setInt("boot_count", 0, "boot");
            nvs.setInt("ota_pending", 0, "boot");
            setStatusFail("Boot count exceeded");
            requestAbort(State::Type::NULL_STATE);
            return true;
        }
        nvs.setInt("boot_count", bootCount, "boot");
        return true;
    }

    static bool LoadConfig() {
        setStatus("Load config");
        if (!Core::config.load(!context().displayReady)) {
            setStatusFail("Config load failed");
            Log::E(" === ERROR Core::config.load");
            Core::config.print_config();
            requestAbort(State::Type::NULL_STATE);
            return true;
        }

        String machineError;
        String selectedMachine = *App::cfg().machineName;
        if (!App::machine().selectByName(selectedMachine, &machineError)) {
            setStatusFail(machineError);
            Log::E(" === ERROR Machine Select: %s", machineError.c_str());
            requestAbort(State::Type::NULL_STATE);
        }
        return true;
    }

    static bool InitDeviceRegistry() {
        setStatus("Init device registry");
        if (!App::instance()->initDeviceRegistry()) {
            setStatusFail("Device registry config failed");
            requestAbort(State::Type::NULL_STATE);
        }
        return true;
    }

    static bool InitCanBus() {
        setStatus("Init CAN bus");
        if (!App::instance()->initCanBus()) {
            setStatusFail("CAN bus failed");
            requestAbort(State::Type::NULL_STATE);
        }
        return true;
    }

    static bool ConfigureDevices() {
        setStatus("Configure devices");
        if (!App::devices().configureRequiredDevices()) {
            setStatusFail("Device configure was rejected");
            requestAbort(State::Type::NULL_STATE);
        }
        return true;
    }

    static bool WaitDevicesReady() {
        setStatus("Wait devices ready");
        BootContext& boot = context();
        boot.waitingRequiredDevices = true;
        boot.waitStartedMs = millis();
        return true;
    }

    static bool ConnectWiFi() {
        setStatus("Connect Wi-Fi");
        BootContext& boot = context();
        boot.wifiConnected = (*App::cfg().connectWifi == 1) && WiFiConfig::getInstance().begin();
        return true;
    }

    static bool UpdateFirmware() {
        setStatus("Check firmware update");
        if (!context().wifiConnected) return true;

        if ((*App::cfg().autoUpdate == 1) && (*App::cfg().updateEsp == 1)) {
            int level = ESPUpdate::getInstance().checkForUpdate();
            if (level > 0) {
                ESPUpdate::getInstance().FirmwareUpdate(level, [](int percent) {
                    loadPage().setStatus(String("Firmware update: ") + String(percent) + "%");
                });
            }
        }
        return true;
    }

    static bool StartHttp() {
        setStatus("Start HTTP");
        if (context().wifiConnected && (*App::cfg().httpServer == 1)) {
            HServer::getInstance().begin();
        }
        return true;
    }

    static bool ConnectMQTT() {
        setStatus("Connect MQTT");
        if (context().wifiConnected) {
            MQTTc::getInstance().connect();
        }
        return true;
    }

    static bool LoadData() {
        setStatus("Load data");
        Core::data.load();
        Data::tuning.load();
        Data::profiles.load();
        Data::tasks.load();
        Stats::getInstance().init();
        return true;
    }

    static bool RegisterRuntimeEvents() {
        setStatus("Register runtime events");
        Trigger::getInstance().registerTrigger();
        return true;
    }
};
