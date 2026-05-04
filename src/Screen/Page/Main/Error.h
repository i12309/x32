#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Error : public Page {
public:
    static Error& instance();

protected:
    void onPrepare() override;
    void onShow() override;

private:
    Error() : Page(SCREEN_ID_INFO) {}

    static void popBack(lv_event_t* e);
    static void popNext(lv_event_t* e);
    static void popService(lv_event_t* e);
    static void popReset(lv_event_t* e);

    void renderError();
};

}  // namespace Screen
