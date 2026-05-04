#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Help : public Page {
public:
    static Help& instance();
    void show();

private:
    Help() : Page(SCREEN_ID_PAGE) {}

    static void popBack(lv_event_t* e);
    static void popDevice(lv_event_t* e);
    static void popUpdate(lv_event_t* e);
    static void popLicence(lv_event_t* e);
};

}  // namespace Screen
