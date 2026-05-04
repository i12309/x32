#pragma once

#include "Catalog.h"
#include "Screen/Page/Page.h"
#include "Screen/Page/Task/List.h"

namespace Screen {

class TaskList : public Page {
public:
    static TaskList& instance();

    void setBackPage(Catalog::PageMode mode) { backPageStatus_ = mode; }
    void show();

private:
    TaskList() : Page(SCREEN_ID_LIST) {}

    void display();

    static void popBack(lv_event_t* e);
    static void popNext(lv_event_t* e);
    static void popAdd(lv_event_t* e);
    static void popRow(lv_event_t* e);
    static void popEdit(lv_event_t* e);

    Catalog::PageMode backPageStatus_ = Catalog::PageMode::pTaskRun;
};

}  // namespace Screen
