#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Guillotine : public Page {
public:
    static Guillotine& instance();

protected:
    void onPrepare() override;

private:
    Guillotine() : Page(SCREEN_ID_GUILLOTINE) {}
    static void popBack(lv_event_t* e);
};

}  // namespace Screen
