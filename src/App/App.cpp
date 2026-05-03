#include "App.h"

#include "Core.h"
#include "Machine/Machine.h"
#if !defined(X32_TARGET_HEAD_UNIT)
#include "Controller/Registry.h"
#include "Controller/Trigger.h"
#include "Service/Service.h"
#include "UI/Page.h"
#endif
#include "Bus/Esp32TwaiBus.h"
#include "Can/CanRouter.h"
#include "Can/MksCan.h"
#include "Can/StmCan.h"
#include "Device/DeviceRegistry.h"
#include "Service/DeviceError.h"
#include "Service/Log.h"
#include "Scene/SceneManager.h"
#include "Screen/Panel/Panel.h"
#include "State/PlanManager.h"
#include "State/State.h"

namespace {
Esp32TwaiBus canBus;
DeviceRegistry& deviceRegistry = DeviceRegistry::getInstance();
CanRouter canRouter(canBus, deviceRegistry);
MksCan mksCan(canBus);
StmCan stmCan(canBus);
SceneManager sceneManager(deviceRegistry);
Screen::Panel screenPanel;

App::Context buildAppContext() {
    App::Context ctx;

    auto& config = Core::config;
    auto& settings = Core::settings;
    auto& data = Core::data;

    ctx.config.machineName = &config.machine;
    ctx.config.allowMissingHardware = &settings.ALLOW_MISSING_HARDWARE;
    ctx.config.checkSystem = &settings.CHECK_SYSTEM;
    ctx.config.connectWifi = &settings.CONNECT_WIFI;
    ctx.config.httpServer = &settings.HTTP_SERVER;
    ctx.config.webEnabled = &settings.WEB;
    ctx.config.autoUpdate = &settings.AUTO_UPDATE;
    ctx.config.updateEsp = &settings.UPDATE;
    ctx.config.updateTft = &settings.TFT_UPDATE;

    ctx.storage.dataDoc = &data.doc;

    Machine& machine = Machine::getInstance();
    ctx.machine.machine = &machine;
#if !defined(X32_TARGET_HEAD_UNIT)
    ctx.machine.devices = &machine.context();
    ctx.machine.registry = &Registry::getInstance();
#endif

    ctx.plan.manager = &PlanManager::instance();
    ctx.diagnostics.deviceError = &DeviceError::getInstance();
#if !defined(X32_TARGET_HEAD_UNIT)
    ctx.motion.scene = &Scene::getInstance();
#endif
    ctx.motion.sceneManager = &sceneManager;
    ctx.can.devices = &deviceRegistry;
    ctx.can.router = &canRouter;
    ctx.ui.panel = &screenPanel;

#if !defined(X32_TARGET_HEAD_UNIT)
    ctx.ui.activePage = &Page::activePage;
    ctx.ui.previousPage = &Page::previousPage;
#endif

    return ctx;
}
}  // namespace

App* App::instance_ = nullptr;

App::App() : context_(buildAppContext()) {
    instance_ = this;
}

App::Context::ConfigContext& App::cfg() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    return ctx->config;
}

#if !defined(X32_TARGET_HEAD_UNIT)
IMachineContext& App::ctx() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    IMachineContext* deviceContext = ctx->machine.devices;
    if (deviceContext != nullptr) return *deviceContext;
    return Machine::getInstance().context();
}
#endif

Machine& App::machine() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    Machine* m = ctx->machine.machine;
    if (m != nullptr) return *m;
    return Machine::getInstance();
}

#if !defined(X32_TARGET_HEAD_UNIT)
Registry& App::reg() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    Registry* reg = ctx->machine.registry;
    if (reg != nullptr) return *reg;
    return Registry::getInstance();
}

Scene& App::scene() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    Scene* scene = ctx->motion.scene;
    if (scene != nullptr) return *scene;
    return Scene::getInstance();
}
#endif

SceneManager& App::sceneManager() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    SceneManager* manager = ctx->motion.sceneManager;
    if (manager != nullptr) return *manager;

    Log::E("[App] SceneManager is not initialized. Aborting.");
    abort();
}

DeviceRegistry& App::devices() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    DeviceRegistry* devices = ctx->can.devices;
    if (devices != nullptr) return *devices;
    return DeviceRegistry::getInstance();
}

CanRouter& App::can() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    CanRouter* router = ctx->can.router;
    if (router != nullptr) return *router;

    Log::E("[App] CanRouter is not initialized. Aborting.");
    abort();
}

Screen::Panel& App::panel() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    Screen::Panel* panel = ctx->ui.panel;
    if (panel != nullptr) return *panel;

    Log::E("[App] Screen panel is not initialized. Aborting.");
    abort();
}

DeviceError& App::diag() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    DeviceError* diagnostics = ctx->diagnostics.deviceError;
    if (diagnostics != nullptr) return *diagnostics;
    return DeviceError::getInstance();
}

State*& App::state() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    return ctx->runtime.currentState;
}

Catalog::Mode& App::mode() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    return ctx->runtime.mode;
}

PlanManager& App::plan() {
    App::Context* ctx = App::tryContext();
    if (ctx == nullptr) {
        Log::E("[App] Context is not initialized. Aborting.");
        abort();
    }
    PlanManager* manager = ctx->plan.manager;
    if (manager != nullptr) return *manager;

    Log::E("[App] PlanManager is not initialized. Aborting.");
    abort();
}

void App::init() {
    Log::init();
    canRouter.bindProtocol(&mksCan);
    canRouter.bindProtocol(&stmCan);
    State::init();
#if !defined(X32_TARGET_HEAD_UNIT)
    Trigger::init();
#endif
}

bool App::initCanDeviceLayer() {
    return initDeviceRegistry() && initCanBus();
}

bool App::initDeviceRegistry() {
    JsonObjectConst root = Core::config.doc.as<JsonObjectConst>();
    if (!devices().loadFromConfig(root)) {
        Log::E("[App] DeviceRegistry config load failed.");
        return false;
    }
    return true;
}

bool App::initCanBus() {
    CanBusConfig cfg;
    const DeviceRegistry::CanSettings& settings = devices().canSettings();
    cfg.txPin = settings.txPin;
    cfg.rxPin = settings.rxPin;
    cfg.bitrate = settings.bitrate;

    if (!can().begin(cfg)) {
        Log::E("[App] CAN bus was not started.");
        return false;
    }
    return true;
}

void App::process() {
    can().process();
    panel().process();
#if !defined(X32_TARGET_HEAD_UNIT)
    Page::process();
#endif
    State::process();
#if !defined(X32_TARGET_HEAD_UNIT)
    Service::process();
#endif
}
