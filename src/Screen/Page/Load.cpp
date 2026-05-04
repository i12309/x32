#include "Load.h"

#include "Screen/Panel/LvglHelpers.h"
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

bool Load::checkVersion() {
    Ui::setText(objects.load_version, firmwareVersion());
    Ui::setText(objects.load_ma_caddress, WiFi.macAddress());
    return true;
}

}  // namespace Screen
