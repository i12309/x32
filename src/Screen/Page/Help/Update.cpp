#include "Update.h"

#include "Core.h"
#include "Screen/Page/Help/Help.h"
#include "Screen/Page/Main/Info.h"
#include "Screen/Page/Main/Wait.h"
#include "Screen/Panel/LvglHelpers.h"
#include "Service/ESPUpdate.h"
#include "Service/WiFiConfig.h"
#include "version.h"

#include <ui/screens.h>

namespace Screen {

Update& Update::instance() {
    static Update page;
    return page;
}

void Update::onPrepare() {
    Ui::onPop(objects.update_back, Update::popBack);
    Ui::onPop(objects.update_dev, Update::popUpdateEsp);
    Ui::onPop(objects.update_scr, Update::popUpdateTft);
    Ui::onPop(objects.update_auto, Update::popAutoUpdate);
}

void Update::onShow() {
    setAutoUpdateUi();

    String versionEsp = String(APP_VERSION);
    if (ESPUpdate::getInstance().checkForUpdate() > 0) {
        versionEsp = ESPUpdate::getInstance().getNewAppVersion() + " новая";
    }

    Ui::setText(objects.update_dev_ver, versionEsp);
    Ui::setText(objects.update_scr_ver, "LVGL");
    Ui::setText(objects.update_version, String("Версия: ") + Version::makeDeviceVersion(0));
}

void Update::setAutoUpdateUi() {
    Ui::setText(objects.update_auto, Core::settings.AUTO_UPDATE ? "Авто: Вкл" : "Авто: Выкл");
    Ui::setBgColor(objects.update_auto, Core::settings.AUTO_UPDATE ? lv_color_hex(0xf6be00) : lv_color_hex(0xcdcecd));
}

void Update::popBack(lv_event_t* e) {
    (void)e;
    Core::config.save();
    Help::instance().show();
}

void Update::popUpdateEsp(lv_event_t* e) {
    (void)e;
    if (!Core::settings.CONNECT_WIFI || !WiFiConfig::getInstance().isConnect()) {
        Info::showInfo("Обновление", "Нет соединения Wi-Fi", "");
        return;
    }
    if (!Core::settings.UPDATE) return;

    int level = ESPUpdate::getInstance().checkForUpdate();
    if (level <= 0) {
        Info::showInfo("Обновление", "Новая версия не найдена", "");
        return;
    }

    Wait::wait("", "Обновление", "", 0,
               [level]() {
                   ESPUpdate::getInstance().FirmwareUpdate(level);
               },
               false);
}

void Update::popUpdateTft(lv_event_t* e) {
    (void)e;
    Info::showInfo("Обновление экрана", "TODO(ui-lvgl)", "Nextion UART не используется в LVGL Update");
}

void Update::popAutoUpdate(lv_event_t* e) {
    (void)e;
    Core::settings.AUTO_UPDATE = !Core::settings.AUTO_UPDATE;
    instance().setAutoUpdateUi();
}

}  // namespace Screen
