#include "Help.h"

#include "Core.h"
#include "Screen/Page/Help/Device.h"
#include "Screen/Page/Help/Licence.h"
#include "Screen/Page/Help/Page.h"
#include "Screen/Page/Help/Update.h"
#include "Screen/Page/Main/Input.h"
#include "Screen/Page/Main/Main.h"
#include "Service/WiFiConfig.h"

namespace Screen {

Help& Help::instance() {
    static Help page;
    return page;
}

void Help::show() {
    const PageMenu::MenuItem items[] = {
        {"Устройство", Help::popDevice},
        {"Обновление", Help::popUpdate},
        {"Лицензия", Help::popLicence},
    };
    PageMenu::instance().showMenu("Поддержка", items, 3, Help::popBack);
}

void Help::popBack(lv_event_t* e) {
    (void)e;
    Main::instance().show();
}

void Help::popDevice(lv_event_t* e) {
    (void)e;
    if (Core::config.group == "DEV") {
        Device::instance().show();
        return;
    }

    Input::showInput("Устройство", "Настройка устройства", "", "",
                     [](const String& input) {
                         if (input == WiFiConfig::getInstance().mac_xx()) Device::instance().show();
                     },
                     []() { Help::instance().show(); },
                     1,
                     false);
}

void Help::popUpdate(lv_event_t* e) {
    (void)e;
    Update::instance().show();
}

void Help::popLicence(lv_event_t* e) {
    (void)e;
    LicencePage::instance().show();
}

}  // namespace Screen
