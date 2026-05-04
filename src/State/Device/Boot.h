#pragma once
#include "Core.h"
#include "Catalog.h"
#include "Machine/Machine.h"
#include "Service/HServer.h"
#include "Service/NVS.h"
#include "Service/ESPUpdate.h"
#include "Service/NexUpdate.h"
#include "Service/Licence.h"
#include "Service/Stats.h"
#include "version.h"

#include "Screen/Page/Main/INIT.h"
#include "Screen/Page/Main/Load.h"
#include "State/State.h"
#include "Controller/McpTrigger.h"
#include "App/App.h"

class Boot : public State {
private:
    struct BootContext {
        bool wifiConnected = false;
        bool abortRequested = false;
        bool haltRequested = false;
        State::Type abortState = State::Type::NULL_STATE;
        int label_num = 0;
    };

    static BootContext& context() {
        static BootContext ctx;
        return ctx;
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
public:
    Boot() : State(State::Type::BOOT) {}

    void oneRun() override {
        State::oneRun();
        resetContext();

        PlanManager& plan = App::plan();
        plan.beginPlan(this->type());

        plan.addAction(State::Type::ACTION, &Boot::LogStart, "LogStart");
        plan.addAction(State::Type::ACTION, &Boot::ShowLoadScreen, "ShowLoadScreen");
        plan.addAction(State::Type::ACTION, &Boot::InitFileSystem, "InitFileSystem");
        plan.addAction(State::Type::ACTION, &Boot::InitNVS, "InitNVS");
        plan.addAction(State::Type::ACTION, &Boot::LoadConfig, "LoadConfig");
        plan.addAction(State::Type::ACTION, &Boot::ConnectWiFi, "ConnectWiFi");
        plan.addAction(State::Type::ACTION, &Boot::UpdateESP, "UpdateESP");
        plan.addAction(State::Type::ACTION, &Boot::StartHttp, "StartHTTP");
        plan.addAction(State::Type::ACTION, &Boot::ConnectMQTT, "ConnectMQTT");
        plan.addAction(State::Type::ACTION, &Boot::LoadData, "LoadData");
        plan.addAction(State::Type::ACTION, &Boot::InitRegistry, "InitRegistry");
        plan.addAction(State::Type::ACTION, &Boot::RegisterTriggers, "RegisterTriggers");

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

        if (!plan.hasPending()) {
            Log::L(" === END LOAD v.%s", Version::makeDeviceVersion(NexUpdate::getInstance().getCurrentVersion()).c_str());
            Data::param.reset();
            App::ctx().reg.reset();
            App::mode() = Mode::NORMAL;
            App::diag().clearErrors();
            ESPUpdate::getInstance().markCurrentFirmwareValid(); // Boot completed; CHECK_SYSTEM validates hardware, not OTA viability.
            if (*App::cfg().checkSystem == 1) {
                Log::D("BOOT: %s", "Проверка устройства");
                return Factory(State::Type::CHECK);
            }
            return Factory(State::Type::IDLE);
        }

        return Factory(plan.nextType(this->type()));
    }

private:
    static bool LogStart() {
        Log::L(" === START v.%s", Version::makeDeviceVersion(NexUpdate::getInstance().getCurrentVersion()).c_str());
        Log::D("ESP32 Chip: %s Rev %d", ESP.getChipModel(), ESP.getChipRevision());
        Log::D("Flash size: %d", ESP.getFlashChipSize());
        return true;
    }

    static bool ShowLoadScreen() {
        Screen::Load::instance().show();
        Log::D("BOOT: %s", "LVGL UI");
        return true;
    }

    static bool InitFileSystem() {
        Log::D("BOOT: %s", "Инициализация файловой системы");
        if (FileSystem::init(true)) {
            return true;
        }

        Log::D("BOOT: %s", "Ошибка файловой системы");
        Log::E(" === ERROR FileSystem Init ");
        requestAbort(State::Type::NULL_STATE);
        return true;
    }

    static bool InitNVS() {
        Log::D("BOOT: %s", "Инициализация NVS");
        NVS& nvs = NVS::getInstance();
        nvs.init();

        int bootCount = nvs.getInt("boot_count", 0, "boot");
        bootCount++;
        if (bootCount > 3) {
            if (nvs.getInt("ota_pending", 0, "boot") == 1) {
                Log::D("BOOT: %s", "OTA boot failed, rollback");
                if (ESPUpdate::getInstance().rollbackToPreviousPartition("boot_count exceeded")) {
                    requestHalt();
                    return true;
                }
            }
            nvs.setInt("boot_count", 0, "boot");
            nvs.setInt("ota_pending", 0, "boot");
            // TODO(LVGL): вывести ошибку boot на новом экране диагностики.
            // pINFO::showInfo("", "Что-то пошло не так!", "Проверьте параметры", [](){pINIT::getInstance().show();});
            requestAbort(State::Type::NULL_STATE);
            return true;
        }
        nvs.setInt("boot_count", bootCount, "boot");
        return true;
    }

    static bool LoadConfig() {
        Log::D("BOOT: %s", "Загрузка конфигурации");
        if (Core::config.load(false)) {
            String machineError;
            String selectedMachine = *App::cfg().machineName;
            if (!App::machine().selectByName(selectedMachine, &machineError)) {
                Log::D("BOOT: %s", machineError.c_str());
                Log::E(" === ERROR Machine Select: %s", machineError.c_str());
                Screen::INIT::instance().show();
                requestAbort(State::Type::NULL_STATE);
                return true;
            }
            Screen::Load::instance().setModel(selectedMachine);
            return true;
        }

        Log::D("BOOT: %s", "Ошибка загруки config"); //TODO надо сделать что бы при этой ошибке http был доступен и можно было исправить config
        Log::E(" === ERROR Core::config.load");
        Screen::INIT::instance().show();
        //Core::config.print_config();  // отладка - выведем что в config при ошибке
        requestAbort(State::Type::NULL_STATE);
        return true;
    }

    static bool ConnectWiFi() {
        Log::D("BOOT: %s", "Подключение к Wi-Fi");
        BootContext& boot = context();
        boot.wifiConnected = (*App::cfg().connectWifi == 1) && WiFiConfig::getInstance().begin();
        return true;
    }

    static bool UpdateESP() {
        Log::D("BOOT: %s", "Проверка обновлений прошивки");
        if (!context().wifiConnected) return true;

        if ((*App::cfg().autoUpdate == 1) && (*App::cfg().updateEsp == 1)) {
            int level = ESPUpdate::getInstance().checkForUpdate();
            if (level > 0) {
                ESPUpdate::getInstance().FirmwareUpdate(level, [](int percent) {
                    Log::D("OTA update: %d%%", percent);
                });
                // TODO(LVGL): показывать процент OTA на новом экране загрузки.
                // ESPUpdate::getInstance().FirmwareUpdate(level, [](int percent) {
                //     String text = "Обновление: " + String(percent) + "%";
                //     pLoad::getInstance().text(text);
                // });
            }
        }
        return true;
    }

    static bool StartHttp() {
        Log::D("BOOT: %s", "Запуск HTTP сервера");
        if (context().wifiConnected && (*App::cfg().httpServer == 1)) {
            HServer::getInstance().begin();
        }
        return true;
    }

    static bool ConnectMQTT() {
        Log::D("BOOT: %s", "Подключение к серверу");
        if (context().wifiConnected) {
            MQTTc::getInstance().connect(); // подключится
        }
        return true;
    }

    static bool LoadData() {
        Log::D("BOOT: %s", "Загрузка данных");
        // проверка лицензии
        /*
        if (!Licence::getInstance().isValid()) {
            pLoad::getInstance().text("Лицензия не верная. Работа не возможна!");
            Log::L(" === Лицензия не верная");
            requestAbort(State::Type::NULL_STATE);
            return true;
        }*/

        Core::data.load();
        Data::tuning.load();
        Data::profiles.load();
        Data::tasks.load();
        Stats::getInstance().init();
        return true;
    }
    static bool InitRegistry() {
        Log::D("BOOT: %s", "Инициализация устройств");
        String registry_error_message;
        if (!App::reg().init(&registry_error_message)) { // Инициализация устройства
            Log::D("BOOT: %s", registry_error_message.c_str());
            Log::E(" === ERROR Registry Init: %s", registry_error_message.c_str());
            requestAbort(State::Type::NULL_STATE);
            return true;
        }

        // А теперь проверяем подходит ли то что загрузили к этому типу машины 
        Machine& machine = App::machine();
        String machineError;
        if (!machine.bindRegistry(App::reg(), &machineError)) {
            Log::D("BOOT: %s", machineError.c_str());
            Log::E(" === ERROR Machine Bind: %s", machineError.c_str());
            requestAbort(State::Type::NULL_STATE);
            return true;
        }

        MachineSpec::Report specReport = machine.validateRegistry(App::reg());
        for (const String& warning : specReport.warnings) {
            Log::D("%s", warning.c_str());
            App::diag().addWarning(State::ErrorCode::CONFIG_ERROR, "Конфигурация устройства неверная", warning);
        }
        if (!machine.shouldContinueBoot(*App::cfg().allowMissingHardware == 1)) {
            String failText = "Machine registry validation failed";
            if (!specReport.errors.empty()) {
                failText = specReport.errors.front();
            }
            Log::D("BOOT: %s", failText.c_str());
            for (const String& err : specReport.errors) {
                Log::E("%s", err.c_str());
            }
            requestAbort(State::Type::NULL_STATE);
            return true;
        }

        if (specReport.hasErrors()) {
            Log::E("[Machine] Registry validation has blocking errors, but ALLOW_MISSING_HARDWARE=1. Continue boot.");
            for (const String& err : specReport.errors) {
                Log::D("[Machine][warn-only] %s", err.c_str());
                App::diag().addWarning(State::ErrorCode::CONFIG_ERROR, "Работа с ограничениями", err);
            }
        }
        if (!machine.readyForMotion()) {
            Log::D("[Machine] Motion is not ready after registry binding.");
        }

        // ----
        Data::param.reset();
        App::mode() = Mode::NORMAL;
        App::diag().clearErrors();
        return true;
    }

    static bool RegisterTriggers() {
        Log::D("BOOT: %s", "Регистрация триггеров");
        Trigger::getInstance().registerTrigger();
        McpTrigger::getInstance().init();
        return true;
    }

};

