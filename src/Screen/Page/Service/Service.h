#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Service : public Page {
public:
    static Service& instance();

protected:
    void onPrepare() override;
    void onShow() override;

private:
    Service() : Page(SCREEN_ID_SERVICE) {}

    static void popBack(lv_event_t* e);
    static void popNext(lv_event_t* e);
    static void popTable(lv_event_t* e);
    static void popPaper(lv_event_t* e);
    static void popGuillotine(lv_event_t* e);
    static void popSlice(lv_event_t* e);
    static void popCalibration(lv_event_t* e);
    static void popProba(lv_event_t* e);
};

}  // namespace Screen
