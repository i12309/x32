#pragma once

#include "Catalog.h"
#include "Screen/Page/Page.h"

namespace Screen {

class TaskRun : public Page {
public:
    static TaskRun& instance();

    void setBackPage(Catalog::PageMode mode) { backPageStatus_ = mode; }

protected:
    void onPrepare() override;
    void onShow() override;
    void onTick() override;

private:
    TaskRun() : Page(SCREEN_ID_TASK_RUN) {}

    static void popBack(lv_event_t* e);
    static void popStart(lv_event_t* e);
    static void popListTask(lv_event_t* e);
    static void popListProfile(lv_event_t* e);
    static void popMinus(lv_event_t* e);
    static void popPlus(lv_event_t* e);

    Catalog::PageMode backPageStatus_ = Catalog::PageMode::pMain;
};

}  // namespace Screen
