#include "Load.h"

#include "App/App.h"
#include "Screen/Page/Main/Main.h"
#include "Screen/Panel/LvglHelpers.h"
#include "State/State.h"
#include "version.h"

#include <WiFi.h>
#include <ui/screens.h>

namespace {
String firmwareVersion() {
    return String(PRODUCT_VERSION) + "." + String(APP_VERSION) + "." + String(APP_BUILD_NUMBER);
}
}  // namespace

namespace Screen {

Load& Load::instance() {
    static Load page;
    return page;
}

void Load::onShow() {
    checkVersion();
}

void Load::onTick() {
    if (App::state() == nullptr || App::state()->type() != State::Type::IDLE) return;
    Main::instance().show();
}

bool Load::checkVersion() {
    Ui::setText(objects.load_version, firmwareVersion());
    Ui::setText(objects.load_ma_caddress, WiFi.macAddress());
    return true;
}

void Load::setModel(const String& text) {
    Ui::setText(objects.load_model, text);
}

}  // namespace Screen
