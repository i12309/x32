#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class TaskProcess : public Page {
public:
    static TaskProcess& instance();

protected:
    void onPrepare() override;
    void onShow() override;
    void onTick() override;

private:
    TaskProcess() : Page(SCREEN_ID_TASK_PROCESS) {}

    void renderProgress();
    void renderState();

    static void popProcess(lv_event_t* e);
};

}  // namespace Screen
