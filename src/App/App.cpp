#include "App.h"

#include "Core.h"
#include "Machine/Machine.h"
#include "Service/DeviceError.h"
#include "Service/Log.h"
#include "Service/Service.h"
#include "Screen/Page/Page.h"
#include "Screen/Panel/Panel.h"
#include "State/PlanManager.h"
#include "State/State.h"

namespace {
App::Context buildAppContext() {
    App::Context ctx;

    auto& config = Core::config;
    auto& settings = Core::settings;
    auto& data = Core::data;

    ctx.config.machineName = &config.machine;
    ctx.config.checkSystem = &settings.CHECK_SYSTEM;
    ctx.config.connectWifi = &settings.CONNECT_WIFI;
    ctx.config.httpServer = &settings.HTTP_SERVER;
    ctx.config.autoUpdate = &settings.AUTO_UPDATE;
    ctx.config.updateEsp = &settings.UPDATE;

    ctx.storage.dataDoc = &data.doc;

    Machine& machine = Machine::getInstance();
    ctx.machine.machine = &machine;

    ctx.plan.manager = &PlanManager::instance();
    ctx.diagnostics.deviceError = &DeviceError::getInstance();

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
    Panel::init();
    State::init();
}

void App::process() {
    Panel::process();
    State::process();
    Screen::Page::process();
    Service::process();
}
