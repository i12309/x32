#include "Main.h"

#include "App/App.h"
#include "Data.h"
#include "Screen/Page/Info.h"
#include "Screen/Panel/LvglHelpers.h"
#include "Service/Licence.h"
#include "Service/WiFiConfig.h"
#include "State/State.h"

#include <ui/screens.h>

namespace Screen {

Main& Main::instance() {
    static Main page;
    return page;
}

void Main::onPrepare() {
    Ui::onPop(objects.main_task, Main::popTask);
    Ui::onPop(objects.main_profile, Main::popProfile);
    Ui::onPop(objects.main_net, Main::popSettings);
    Ui::onPop(objects.main_service, Main::popService);
    Ui::onPop(objects.main_stats, Main::popStats);
    Ui::onPop(objects.main_support, Main::popHelp);
}

void Main::onShow() {
    Data::work.clear();
    licenseChecked_ = false;
    updateNetworkInfo();
}

void Main::onTick() {
    if (App::state() == nullptr || App::state()->type() != State::Type::IDLE) return;

    checkLicenseOnce();
    checkStartButton();

    const unsigned long now = millis();
    if (now - lastNetworkUpdateMs_ >= 5000) {
        lastNetworkUpdateMs_ = now;
        updateNetworkInfo();
    }
}

void Main::updateNetworkInfo() {
    // TODO(ui-lvgl): добавить имена объектам Wi-Fi info/RSSI в EEZ и выводить:
    // WiFiConfig::getInstance().getNetInfo()
    // WiFiConfig::getInstance().getRSSI()
}

void Main::checkLicenseOnce() {
    if (licenseChecked_) return;
    licenseChecked_ = true;

    if (!Licence::getInstance().isValid()) {
        Info::showInfo("Лицензия не верная", "Работа не возможна!");
    }
}

void Main::checkStartButton() {
    if (App::ctx().reg.getButton("START")->isTrigger()) {
        popTask(nullptr);
    }
}

void Main::popTask(lv_event_t* e) {
    (void)e;

    if (Data::work.task.valid()) {
        Data::profiles.getByID(Data::work.task.PROFILE_ID, Data::work.profile);
    }

    Data::param.frame = 0;
    // TODO(ui-lvgl): после переноса TaskRun вызвать Screen::TaskRun::instance().show().
}

void Main::popProfile(lv_event_t* e) {
    (void)e;
    // TODO(ui-lvgl): после переноса ProfileList вызвать Screen::ProfileList::instance().show().
}

void Main::popSettings(lv_event_t* e) {
    (void)e;
    // TODO(ui-lvgl): после переноса Wifi вызвать Screen::Wifi::instance().show().
}

void Main::popService(lv_event_t* e) {
    (void)e;
    // TODO(ui-lvgl): после переноса Service вызвать Screen::Service::instance().show().
}

void Main::popStats(lv_event_t* e) {
    (void)e;
    // TODO(ui-lvgl): после переноса Stats вызвать Screen::Stats::instance().show().
}

void Main::popHelp(lv_event_t* e) {
    (void)e;
    // TODO(ui-lvgl): после переноса Help/Update вызвать Screen::Help::instance().show().
}

}  // namespace Screen
