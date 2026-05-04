#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Service2 : public Page {
public:
    static Service2& instance();

protected:
    void onPrepare() override;

private:
    Service2() : Page(SCREEN_ID_SERVICE2) {}

    static void popBack(lv_event_t* e);
    static void popThrow(lv_event_t* e);
    static void popBigel(lv_event_t* e);
    static void popTest1(lv_event_t* e);
    static void popTest2(lv_event_t* e);
    static void popTest3(lv_event_t* e);
};

}  // namespace Screen
