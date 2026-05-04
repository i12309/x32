#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Update : public Page {
public:
    static Update& instance();

protected:
    void onPrepare() override;
    void onShow() override;

private:
    Update() : Page(SCREEN_ID_UPDATE) {}

    void setAutoUpdateUi();

    static void popBack(lv_event_t* e);
    static void popUpdateEsp(lv_event_t* e);
    static void popUpdateTft(lv_event_t* e);
    static void popAutoUpdate(lv_event_t* e);
};

}  // namespace Screen
