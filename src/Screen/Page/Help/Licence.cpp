#include "Licence.h"

#include "Core.h"
#include "Screen/Page/Help/Help.h"
#include "Screen/Page/Main/Info.h"
#include "Screen/Page/Main/Wait.h"
#include "Service/Licence.h"
#include "Service/MQTTc.h"
#include "Service/WiFiConfig.h"

namespace Screen {

LicencePage& LicencePage::instance() {
    static LicencePage page;
    return page;
}

void LicencePage::show() {
    ::Licence& licence = ::Licence::getInstance();
    String text = licence.isLicenceLoaded() ? licence.getSummary() : "Лицензия не загружена";
    if (text.isEmpty()) text = "Лицензия загружена";

    Info::showInfo("Лицензия", text, "",
                   []() { LicencePage::instance().startLicenceRequest(); },
                   []() { Help::instance().show(); },
                   true,
                   "Запросить",
                   "Назад");
}

void LicencePage::startLicenceRequest() {
    if (!Core::settings.CONNECT_WIFI || !WiFiConfig::getInstance().isConnect()) {
        Info::showInfo("Нет соединения", "Проверьте Wi-Fi и MQTT", "");
        return;
    }

    ::Licence& licence = ::Licence::getInstance();
    const bool hadLicence = licence.isLicenceLoaded();
    const String licenceBefore = licence.toJson();

    Wait::wait("", "Запрашиваем лицензию...", "", 200,
               [hadLicence, licenceBefore]() {
                   MQTTc::getInstance().licence_request();

                   const unsigned long timeout = 8000;
                   unsigned long start = millis();
                   bool received = false;

                   while (millis() - start < timeout) {
                       MQTTc::process();
                       delay(100);

                       if (::Licence::getInstance().isLicenceLoaded()) {
                           String after = ::Licence::getInstance().toJson();
                           if (!after.isEmpty()) {
                               received = !hadLicence || after != licenceBefore || hadLicence;
                               break;
                           }
                       }
                   }

                   if (received) {
                       LicencePage::instance().show();
                   } else {
                       Info::showInfo("Лицензия", "Лицензия не получена", "",
                                      []() { Help::instance().show(); });
                   }
               },
               false);
}

}  // namespace Screen
