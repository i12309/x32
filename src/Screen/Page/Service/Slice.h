#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Slice : public Page {
public:
    static Slice& instance();

protected:
    void onPrepare() override;
    void onShow() override;

private:
    Slice() : Page(SCREEN_ID_SLICE) {}

    static void popBack(lv_event_t* e);
    static void popProfileList(lv_event_t* e);
    static void popMinus(lv_event_t* e);
    static void popPlus(lv_event_t* e);
    static void popGo(lv_event_t* e);
};

}  // namespace Screen
