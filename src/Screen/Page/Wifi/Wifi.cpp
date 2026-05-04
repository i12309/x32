#include "Wifi.h"

#include "Core.h"
#include "Screen/Page/Main/Info.h"
#include "Screen/Page/Main/Input.h"
#include "Screen/Page/Main/Main.h"
#include "Screen/Page/Main/Wait.h"
#include "Screen/Panel/LvglHelpers.h"
#include "Service/HServer.h"
#include "Service/WiFiConfig.h"

#include <WiFi.h>
#include <ui/screens.h>

namespace Screen {

Wifi& Wifi::instance() {
    static Wifi page;
    return page;
}

void Wifi::onPrepare() {
    Ui::onPop(objects.wifi_back, Wifi::popBack);
    Ui::onPop(objects.wifi_save, Wifi::popSave);
    Ui::onPop(objects.wifi_connect, Wifi::popConnect);
    Ui::onPop(objects.wifi_add, Wifi::popAdd);
    Ui::onPop(objects.wifi_del, Wifi::popDelete);
    Ui::onPop(objects.wifi_auto_connect, Wifi::popAutoConnect);
}

void Wifi::onShow() {
    refreshList();
    updateAutoConnect();
    updateConnectionInfo();
    updateConnectButton();
}

void Wifi::onTick() {
    unsigned long now = millis();
    if (now - lastUpdateMs_ < 5000) return;
    lastUpdateMs_ = now;
    updateConnectionInfo();
    updateConnectButton();
}

void Wifi::refreshList() {
    WiFiConfig& wifi = WiFiConfig::getInstance();
    uint8_t count = wifi.getCount();

    if (count == 0) {
        Ui::dropdownSetOptions(objects.wifi_ssid, "");
        Ui::setText(objects.wifi_rssi, "");
        Ui::setText(objects.wifi_ip, "");
        Ui::setHidden(objects.wifi_connect, true);
        Ui::setHidden(objects.wifi_del, true);
        return;
    }

    String options;
    int defaultIndex = wifi.getDefaultIndex();
    for (int i = 0; i < count; ++i) {
        if (i > 0) options += "\n";
        if (i == defaultIndex) options += "* ";
        options += wifi.getSSIDByIndex(i);
    }

    Ui::dropdownSetOptions(objects.wifi_ssid, options);
    int selected = wifi.getConnectedIndex();
    if (selected < 0) selected = (defaultIndex >= 0 && defaultIndex < count) ? defaultIndex : 0;
    Ui::dropdownSetSelected(objects.wifi_ssid, static_cast<uint32_t>(selected));
    Ui::setHidden(objects.wifi_connect, false);
    Ui::setHidden(objects.wifi_del, false);
}

void Wifi::updateConnectionInfo() {
    WiFiConfig& wifi = WiFiConfig::getInstance();
    if (wifi.isConnect()) {
        int rssi = WiFi.RSSI();
        Ui::setText(objects.wifi_rssi, String(rssi == 0 ? 0 : 100 + rssi));
        Ui::setText(objects.wifi_ip, wifi.getIP().toString());
    } else {
        Ui::setText(objects.wifi_rssi, "");
        Ui::setText(objects.wifi_ip, "");
    }
}

void Wifi::updateAutoConnect() {
    bool enabled = Core::settings.CONNECT_WIFI != 0;
    Ui::setChecked(Ui::firstChild(objects.wifi_auto_connect), enabled);
    Ui::setBgColor(objects.wifi_auto_connect, enabled ? lv_color_hex(0xf6be00) : lv_color_hex(0xcdcecd));
}

void Wifi::updateConnectButton() {
    WiFiConfig& wifi = WiFiConfig::getInstance();
    if (wifi.getCount() == 0) return;
    String ssid = selectedSSID();
    if (ssid.isEmpty()) return;
    setConnectButtonState(wifi.isConnectedTo(ssid.c_str()));
}

uint32_t Wifi::selectedIndex() const {
    return Ui::dropdownSelected(objects.wifi_ssid);
}

String Wifi::selectedSSID() const {
    WiFiConfig& wifi = WiFiConfig::getInstance();
    uint32_t index = selectedIndex();
    if (index >= wifi.getCount()) return "";
    return String(wifi.getSSIDByIndex(index));
}

void Wifi::setConnectButtonState(bool connected) {
    Ui::setText(objects.wifi_connect, connected ? "Отключиться" : "Подключиться");
    Ui::setBgColor(objects.wifi_connect, connected ? lv_color_hex(0xee9120) : lv_color_hex(0x007129));
}

void Wifi::popBack(lv_event_t* e) {
    (void)e;
    Main::instance().show();
}

void Wifi::popSave(lv_event_t* e) {
    (void)e;
    Wifi& page = instance();
    WiFiConfig& wifi = WiFiConfig::getInstance();

    if (wifi.getCount() == 0) {
        Core::settings.CONNECT_WIFI = 0;
        Core::config.save();
        Main::instance().show();
        return;
    }

    uint32_t index = page.selectedIndex();
    if (index >= wifi.getCount()) return;

    wifi.setDefaultIndex(static_cast<int>(index));
    wifi.save();
    Core::config.save();
    Main::instance().show();
}

void Wifi::popConnect(lv_event_t* e) {
    (void)e;
    Wifi& page = instance();
    WiFiConfig& wifi = WiFiConfig::getInstance();
    if (wifi.getCount() == 0) return;

    String ssid = page.selectedSSID();
    if (ssid.isEmpty()) return;

    if (!wifi.isConnectedTo(ssid.c_str())) {
        Wait::wait("", "Соединение ...", "", 1000,
                   [ssid]() {
                       WiFiConfig& wifi = WiFiConfig::getInstance();
                       bool ok = wifi.connectTo(ssid.c_str());
                       Wifi::instance().show();
                       if (ok) {
                           if (Core::settings.HTTP_SERVER) HServer::getInstance().begin();
                       } else {
                           Info::showInfo("WiFi", "Ошибка подключения", wifi.getLastError());
                       }
                   },
                   false);
    } else {
        wifi.disconnect();
        page.show();
    }
}

void Wifi::popAdd(lv_event_t* e) {
    (void)e;
    Input::showInput("Новая WiFi сеть", "", "Введите название сети (SSID):", "",
                     [](const String& ssid) {
                         if (ssid.isEmpty()) {
                             Wifi::instance().show();
                             return;
                         }
                         String ssidCopy = ssid;
                         Input::showInput("Новая WiFi сеть", "", "Введите пароль сети:", "",
                                          [ssidCopy](const String& pass) {
                                              WiFiConfig& wifi = WiFiConfig::getInstance();
                                              if (!wifi.addOrUpdate(ssidCopy.c_str(), pass.c_str())) {
                                                  Wifi::instance().show();
                                                  Info::showInfo("WiFi", "Не удалось добавить сеть", wifi.getLastError());
                                                  return;
                                              }
                                              wifi.save();
                                              Wifi::instance().show();
                                          },
                                          []() { Wifi::instance().show(); },
                                          1,
                                          false);
                     },
                     []() { Wifi::instance().show(); },
                     1,
                     false);
}

void Wifi::popDelete(lv_event_t* e) {
    (void)e;
    String ssid = instance().selectedSSID();
    if (ssid.isEmpty()) return;

    Info::showInfo("Удаление сети", "Подтвердите удаление", ssid,
                   [ssid]() {
                       WiFiConfig& wifi = WiFiConfig::getInstance();
                       if (wifi.isConnectedTo(ssid.c_str())) wifi.disconnect();
                       wifi.remove(ssid.c_str());
                       wifi.save();
                       Wifi::instance().show();
                   },
                   nullptr,
                   true);
}

void Wifi::popAutoConnect(lv_event_t* e) {
    (void)e;
    Core::settings.CONNECT_WIFI = Core::settings.CONNECT_WIFI ? 0 : 1;
    Core::config.save();
    instance().updateAutoConnect();
}

}  // namespace Screen
