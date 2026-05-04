#include "Main.h"

#include "App/App.h"
#include "Catalog.h"
#include "Data.h"
#include "Screen/Page/Help/Help.h"
#include "Screen/Page/Main/Info.h"
#include "Screen/Page/Profile/ProfileList.h"
#include "Screen/Page/Service/Service.h"
#include "Screen/Page/Statistics/Statistics.h"
#include "Screen/Page/Task/TaskRun.h"
#include "Screen/Page/Wifi/Wifi.h"
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
    //if (App::ctx().reg.getButton("START")->isTrigger()) {popTask(nullptr);}
}

void Main::popTask(lv_event_t* e) {
    (void)e;

    if (Data::work.task.valid()) {
        Data::profiles.getByID(Data::work.task.PROFILE_ID, Data::work.profile);
    }

    Data::param.frame = 0;
    TaskRun::instance().setBackPage(Catalog::PageMode::pMain);
    TaskRun::instance().show();
}

void Main::popProfile(lv_event_t* e) {
    (void)e;
    ProfileList::instance().setBackPage(Catalog::PageMode::pMain);
    ProfileList::instance().show();
}

void Main::popSettings(lv_event_t* e) {
    (void)e;
    Wifi::instance().show();
}

void Main::popService(lv_event_t* e) {
    (void)e;
    Service::instance().show();
}

void Main::popStats(lv_event_t* e) {
    (void)e;
    Statistics::instance().show();
}

void Main::popHelp(lv_event_t* e) {
    (void)e;
    Help::instance().show();
}

}  // namespace Screen
