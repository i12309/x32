#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Bigel : public Page {
public:
    static Bigel& instance();

protected:
    void onPrepare() override;

private:
    Bigel() : Page(SCREEN_ID_BIGEL) {}
    static void popBack(lv_event_t* e);
};

}  // namespace Screen
