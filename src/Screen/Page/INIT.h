#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>

namespace Screen {

class INIT : public Page {
public:
    static INIT& instance();

protected:
    void onPrepare() override;
    void onShow() override;

private:
    INIT() : Page(SCREEN_ID_INIT) {}

    void prefill();
    void refreshMachineList();
    uint32_t getSelectedMachineIndex();
    void setSelectedMachineIndex(uint32_t index);

    static void popSave(lv_event_t* e);
    static void popHttp(lv_event_t* e);
    static void popAccessPoint(lv_event_t* e);
    static void popTest(lv_event_t* e);

    bool prefilled_ = false;
    uint32_t lastMachineIndex_ = 0;
};

}  // namespace Screen
