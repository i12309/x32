#include "INIT.h"

#include "Catalog.h"
#include "Core.h"
#include "Machine/MachineSpec.h"
#include "Screen/Page/Info.h"
#include "Screen/Page/Input.h"
#include "Screen/Page/Wait.h"
#include "Screen/Panel/LvglHelpers.h"
#include "Service/HServer.h"
#include "Service/WiFiConfig.h"
#include "version.h"

#include <WiFi.h>
#include <esp_system.h>
#include <ui/screens.h>

namespace {

Catalog::MachineType machineByIndex(uint32_t index) {
    size_t count = 0;
    const Catalog::MachineType* types = Catalog::machineTypes(count);
    Catalog::MachineType firstSupported = Catalog::MachineType::UNKNOWN;
    uint32_t currentIndex = 0;

    for (size_t i = 0; i < count; ++i) {
        MachineSpec spec = MachineSpec::get(types[i]);
        if (spec.type() == Catalog::MachineType::UNKNOWN) continue;

        if (firstSupported == Catalog::MachineType::UNKNOWN) {
            firstSupported = types[i];
        }
        if (currentIndex == index) {
            return types[i];
        }
        ++currentIndex;
    }

    return firstSupported;
}

bool buildConfig(Catalog::MachineType machine, const String& group, const String& name, bool accessPoint, bool withTestData) {
    ConfigDefaults::Options options;
    options.machine = machine;
    options.group = group;
    options.name = name;
    options.accessPoint = accessPoint;
    options.withTestData = withTestData;

    if (!ConfigDefaults::build(Core::config.doc, options)) return false;

    Core::config.machine = Catalog::machineName(machine);
    Core::config.configVersion = options.configVersion;
    Core::config.name = name;
    Core::config.group = group;

    if (!Core::settings.load(Core::config.doc)) return false;
    if (!ConfigDefaults::buildData(Core::data.doc, withTestData)) return false;
    if (!Core::data.save()) return false;

    return Core::config.save();
}

void showHttpIpWait(const String& title, const IPAddress& ip) {
    Screen::Wait& wait = Screen::Wait::instance();
    wait.show();
    Ui::setText(objects.wait_text1, title);
    Ui::setText(objects.wait_text2, "IP: " + ip.toString());
    Ui::setText(objects.wait_text3, "");
}

void showWiFiConnectErrorAndRestart(const char* errorText) {
    Screen::Info::showInfo(
        "WiFi",
        "Ошибка подключения",
        errorText == nullptr ? "" : errorText,
        []() { esp_restart(); }
    );
}

void startHttpServerForConnectedWiFi(const IPAddress& ip) {
    HServer::getInstance().begin();
    showHttpIpWait("HTTP сервер", ip);
}

void askManualWiFiCredentials() {
    Screen::Input::showInput(
        "WiFi", "", "Введите название сети (SSID):", "",
        [](const String& ssid) {
            if (ssid.isEmpty()) {
                Screen::INIT::instance().show();
                return;
            }
            String ssidCopy = ssid;
            Screen::Input::showInput(
                "WiFi", "", "Введите пароль сети:", "",
                [ssidCopy](const String& pass) {
                    Screen::Wait::wait("", "Соединение ...", "", 1000, [ssidCopy, pass]() {
                        WiFiConfig& wifi = WiFiConfig::getInstance();

                        bool ok = wifi.connectWithCreds(ssidCopy.c_str(), pass.c_str(), false);
                        if (!ok) {
                            showWiFiConnectErrorAndRestart(wifi.getLastError());
                            return;
                        }

                        startHttpServerForConnectedWiFi(wifi.getIP());
                    }, false);
                },
                []() { Screen::INIT::instance().show(); },
                1,
                false
            );
        },
        []() { Screen::INIT::instance().show(); },
        1,
        false
    );
}

void connectDefaultWiFiOrAskManual() {
    WiFiConfig& wifi = WiFiConfig::getInstance();
    int defaultIndex = wifi.getDefaultIndex();
    uint8_t count = wifi.getCount();

    if (defaultIndex < 0 || defaultIndex >= count) {
        askManualWiFiCredentials();
        return;
    }

    String defaultSsid = wifi.getSSIDByIndex(static_cast<uint8_t>(defaultIndex));
    if (defaultSsid.isEmpty()) {
        askManualWiFiCredentials();
        return;
    }

    Screen::Info::showInfo(
        "WiFi",
        "Подключиться к сети по умолчанию?",
        defaultSsid,
        [defaultSsid]() {
            Screen::Wait::wait("", "Соединение ...", "", 1000, [defaultSsid]() {
                WiFiConfig& wifi = WiFiConfig::getInstance();
                bool ok = wifi.connectTo(defaultSsid.c_str());
                if (!ok) {
                    showWiFiConnectErrorAndRestart(wifi.getLastError());
                    return;
                }
                startHttpServerForConnectedWiFi(wifi.getIP());
            }, false);
        },
        []() { askManualWiFiCredentials(); },
        true
    );
}

}  // namespace

namespace Screen {

INIT& INIT::instance() {
    static INIT page;
    return page;
}

void INIT::onPrepare() {
    Ui::onPop(objects.init_ok, INIT::popSave);
    Ui::onPop(objects.init_http, INIT::popHttp);
    Ui::onPop(objects.init_access_point, INIT::popAccessPoint);
    Ui::onPop(objects.init_test, INIT::popTest);
}

void INIT::onShow() {
    if (!prefilled_) prefill();
}

void INIT::prefill() {
    refreshMachineList();
    setSelectedMachineIndex(lastMachineIndex_);
    Ui::setChecked(objects.init_r_access_point, false);
    Ui::setChecked(objects.init_r_test, false);
    Ui::setText(objects.init_group, "DEV");
    Ui::setText(objects.init_name, "ESP32");
    prefilled_ = true;
}

void INIT::refreshMachineList() {
    size_t count = 0;
    const Catalog::MachineType* types = Catalog::machineTypes(count);
    String list;
    bool hasAny = false;

    for (size_t i = 0; i < count; ++i) {
        MachineSpec spec = MachineSpec::get(types[i]);
        if (spec.type() == Catalog::MachineType::UNKNOWN) continue;

        if (hasAny) list += "\n";
        list += Catalog::machineName(types[i]);
        hasAny = true;
    }

    Ui::dropdownSetOptions(objects.init_machine, list);
}

uint32_t INIT::getSelectedMachineIndex() {
    return Ui::dropdownSelected(objects.init_machine);
}

void INIT::setSelectedMachineIndex(uint32_t index) {
    Ui::dropdownSetSelected(objects.init_machine, index);
}

void INIT::popSave(lv_event_t* e) {
    (void)e;
    INIT& page = INIT::instance();

    String group = Ui::getText(objects.init_group);
    String name = Ui::getText(objects.init_name);
    group.trim();
    name.trim();

    if (group.isEmpty()) {
        Info::showInfo("Ошибка", "Заполните поле", "Группа");
        return;
    }

    if (name.isEmpty()) {
        Info::showInfo("Ошибка", "Заполните поле", "Имя");
        return;
    }

    page.lastMachineIndex_ = page.getSelectedMachineIndex();
    Catalog::MachineType machine = machineByIndex(page.lastMachineIndex_);

    bool accessPoint = Ui::isChecked(objects.init_r_access_point);
    bool withTestData = Ui::isChecked(objects.init_r_test);

    if (!buildConfig(machine, group, name, accessPoint, withTestData)) {
        Info::showInfo("Ошибка", "Не удалось сохранить", "config.json");
        return;
    }

    esp_restart();
}

void INIT::popHttp(lv_event_t* e) {
    (void)e;
    Info::showInfo(
        "WiFi",
        "Вы хотите подключиться к Wifi сети?",
        "",
        []() {
            connectDefaultWiFiOrAskManual();
        },
        []() {
            WiFiConfig& wifi = WiFiConfig::getInstance();
            if (!WiFi.softAP("SMIT_" + String(APP_VERSION) + "_" + wifi.mac_xx())) {
                Info::showInfo("WiFi", "Ошибка запуска точки доступа", "",
                    []() { INIT::instance().show(); });
                return;
            }

            HServer::getInstance().begin();
            showHttpIpWait("HTTP сервер", WiFi.softAPIP());
        },
        true
    );
}

void INIT::popAccessPoint(lv_event_t* e) {
    (void)e;
    Ui::setChecked(objects.init_r_access_point, !Ui::isChecked(objects.init_r_access_point));
}

void INIT::popTest(lv_event_t* e) {
    (void)e;
    Ui::setChecked(objects.init_r_test, !Ui::isChecked(objects.init_r_test));
}

}  // namespace Screen
