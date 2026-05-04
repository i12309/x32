#include "Device.h"

#include "Core.h"
#include "Screen/Page/Help/Help.h"
#include "Screen/Page/Help/Page.h"
#include "Screen/Page/Main/Info.h"

namespace Screen {

Device& Device::instance() {
    static Device page;
    return page;
}

void Device::show() {
    const PageMenu::MenuItem items[] = {
        {"Параметры", Device::popParams},
        {"Тест пинов", Device::popPinTest},
    };
    PageMenu::instance().showMenu("Устройство", items, 2, Device::popBack);
}

void Device::popBack(lv_event_t* e) {
    (void)e;
    Help::instance().show();
}

void Device::popParams(lv_event_t* e) {
    (void)e;
    Info::showInfo("Параметры", String("Машина: ") + Core::config.machine,
                   String("Группа: ") + Core::config.group + " / " + Core::config.name);
}

void Device::popPinTest(lv_event_t* e) {
    (void)e;
    Info::showInfo("Тест пинов", "TODO(ui-lvgl)", "В EEZ нет отдельного LVGL screen для PinTest");
}

}  // namespace Screen
