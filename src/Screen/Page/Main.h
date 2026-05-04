#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Main : public Page {
public:
    static Main& instance();

protected:
    void onPrepare() override;
    void onShow() override;
    void onTick() override;

private:
    Main() : Page(SCREEN_ID_MAIN) {}

    void updateNetworkInfo();
    void checkLicenseOnce();
    void checkStartButton();

    static void popTask(lv_event_t* e);
    static void popProfile(lv_event_t* e);
    static void popSettings(lv_event_t* e);
    static void popService(lv_event_t* e);
    static void popStats(lv_event_t* e);
    static void popHelp(lv_event_t* e);

    unsigned long lastNetworkUpdateMs_ = 0;
    bool licenseChecked_ = false;
};

}  // namespace Screen
