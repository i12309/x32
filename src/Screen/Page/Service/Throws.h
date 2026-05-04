#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Throws : public Page {
public:
    static Throws& instance();

protected:
    void onPrepare() override;

private:
    Throws() : Page(SCREEN_ID_THROWS) {}
    static void popBack(lv_event_t* e);
};

}  // namespace Screen
