#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Device : public Page {
public:
    static Device& instance();
    void show();

private:
    Device() : Page(SCREEN_ID_PAGE) {}

    static void popBack(lv_event_t* e);
    static void popParams(lv_event_t* e);
    static void popPinTest(lv_event_t* e);
};

}  // namespace Screen
