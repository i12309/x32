#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Table : public Page {
public:
    static Table& instance();

protected:
    void onPrepare() override;

private:
    Table() : Page(SCREEN_ID_TABLE) {}
    static void popBack(lv_event_t* e);
};

}  // namespace Screen
