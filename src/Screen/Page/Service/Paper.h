#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Paper : public Page {
public:
    static Paper& instance();

protected:
    void onPrepare() override;

private:
    Paper() : Page(SCREEN_ID_PAPER) {}
    static void popBack(lv_event_t* e);
};

}  // namespace Screen
