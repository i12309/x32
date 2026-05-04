#include "Statistics.h"

#include "Screen/Page/Help/Page.h"
#include "Screen/Page/Main/Main.h"
#include "Screen/Page/Statistics/ShowStat.h"

namespace Screen {

Statistics& Statistics::instance() {
    static Statistics page;
    return page;
}

void Statistics::show() {
    const PageMenu::MenuItem items[] = {
        {"Устройство", Statistics::popDevice},
        {"Задания", Statistics::popTask},
        {"Профили", Statistics::popProfile},
    };
    PageMenu::instance().showMenu("Статистика", items, 3, Statistics::popBack);
}

void Statistics::popBack(lv_event_t* e) {
    (void)e;
    Main::instance().show();
}

void Statistics::popDevice(lv_event_t* e) {
    (void)e;
    ShowStat::instance().showDeviceMotors();
}

void Statistics::popTask(lv_event_t* e) {
    (void)e;
    ShowStat::instance().showTasks();
}

void Statistics::popProfile(lv_event_t* e) {
    (void)e;
    ShowStat::instance().showProfiles();
}

}  // namespace Screen
