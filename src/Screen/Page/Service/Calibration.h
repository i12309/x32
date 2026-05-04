#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Calibration : public Page {
public:
    static Calibration& instance();

protected:
    void onPrepare() override;
    void onShow() override;

private:
    Calibration() : Page(SCREEN_ID_CALIBRATION) {}

    static void popBack(lv_event_t* e);
    static void popProfileList(lv_event_t* e);
    static void popSave(lv_event_t* e);
};

}  // namespace Screen
