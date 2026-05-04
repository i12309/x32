#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Statistics : public Page {
public:
    static Statistics& instance();
    void show();

private:
    Statistics() : Page(SCREEN_ID_PAGE) {}

    static void popBack(lv_event_t* e);
    static void popDevice(lv_event_t* e);
    static void popTask(lv_event_t* e);
    static void popProfile(lv_event_t* e);
};

}  // namespace Screen
