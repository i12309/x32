#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>

namespace Screen {

class Wifi : public Page {
public:
    static Wifi& instance();

protected:
    void onPrepare() override;
    void onShow() override;
    void onTick() override;

private:
    Wifi() : Page(SCREEN_ID_WIFI) {}

    void refreshList();
    void updateConnectionInfo();
    void updateAutoConnect();
    void updateConnectButton();
    uint32_t selectedIndex() const;
    String selectedSSID() const;
    void setConnectButtonState(bool connected);

    static void popBack(lv_event_t* e);
    static void popSave(lv_event_t* e);
    static void popConnect(lv_event_t* e);
    static void popAdd(lv_event_t* e);
    static void popDelete(lv_event_t* e);
    static void popAutoConnect(lv_event_t* e);

    unsigned long lastUpdateMs_ = 0;
};

}  // namespace Screen
