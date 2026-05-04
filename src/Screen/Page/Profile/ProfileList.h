#pragma once

#include "Catalog.h"
#include "Screen/Page/Page.h"
#include "Screen/Page/Task/List.h"

namespace Screen {

class ProfileList : public Page {
public:
    static ProfileList& instance();

    void setBackPage(Catalog::PageMode mode) { backPageStatus_ = mode; }
    void show();

private:
    ProfileList() : Page(SCREEN_ID_LIST) {}

    void display();

    static void popBack(lv_event_t* e);
    static void popNext(lv_event_t* e);
    static void popAdd(lv_event_t* e);
    static void popRow(lv_event_t* e);

    Catalog::PageMode backPageStatus_ = Catalog::PageMode::pMain;
};

}  // namespace Screen
