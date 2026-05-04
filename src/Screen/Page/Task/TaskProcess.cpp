#include "TaskProcess.h"

#include "App/App.h"
#include "Data.h"
#include "Screen/Page/Task/TaskRun.h"
#include "Screen/Panel/LvglHelpers.h"
#include "Service/Stats.h"
#include "State/State.h"

#include <ui/screens.h>

namespace Screen {

TaskProcess& TaskProcess::instance() {
    static TaskProcess page;
    return page;
}

void TaskProcess::onPrepare() {
    // TODO(ui-lvgl): rename obj34 in EEZ to task_process_button.
    Ui::onPop(objects.obj34, TaskProcess::popProcess);
}

void TaskProcess::onShow() {
    // TODO(ui-lvgl): rename task_process obj27/obj29/obj30/obj31/obj32/obj33 in EEZ.
    Ui::setText(objects.obj27, Data::work.task.valid() ? Data::work.task.NAME : "-");
    Ui::setText(objects.obj29, Data::work.profile.valid() ? Data::work.profile.NAME : "-");

    if (Data::param.frame) {
        Ui::setText(objects.obj30, "0");
        Ui::setText(objects.obj31, "Корректировка изделия");
        Ui::setHidden(objects.obj30, false);
        Ui::setHidden(objects.obj31, false);
    } else {
        Ui::setText(objects.obj30, "");
        Ui::setText(objects.obj31, "");
        Ui::setHidden(objects.obj30, true);
        Ui::setHidden(objects.obj31, true);
    }

    renderProgress();
    renderState();
}

void TaskProcess::onTick() {
    renderProgress();
    renderState();

    if (App::state() == nullptr || App::state()->type() != State::Type::IDLE) return;
    //IButton* start = App::ctx().reg.getButton("START");    if (start != nullptr && start->isTrigger()) popProcess(nullptr);
}

void TaskProcess::renderProgress() {
    Ui::setText(objects.obj32, String("Лист: ") + String(Data::param.cyclesCount));
    Ui::setText(objects.obj33, String("Рез: ") + String(Data::param.cutsCount));
}

void TaskProcess::renderState() {
    if (App::state() == nullptr) return;

    switch (App::state()->type()) {
        case State::Type::FRAME:
            Ui::setText(objects.obj34, "Продолжить");
            Ui::setBgColor(objects.obj34, lv_color_hex(0x0040cd));
            break;
        case State::Type::FINISH:
            Ui::setText(objects.obj34, "ВЫПОЛНЕНО!");
            Ui::setBgColor(objects.obj34, lv_color_hex(0x007129));
            break;
        default:
            Ui::setText(objects.obj34, "СТОП");
            Ui::setBgColor(objects.obj34, lv_color_hex(0xd51420));
            break;
    }
}

void TaskProcess::popProcess(lv_event_t* e) {
    (void)e;
    if (App::state() == nullptr) return;

    if (App::state()->type() == State::Type::FRAME) {
        Data::param.delta_over = Ui::getText(objects.obj30).toFloat();
        Data::work.task.PRODUCT_mm += Data::param.delta_over;
        Data::tasks.edit(Data::work.task);
        Ui::setText(objects.obj30, "0");
        App::state()->setFactory(App::state()->getNextTypeState());
        return;
    }

    if (App::state()->type() == State::Type::FINISH) {
        App::state()->setFactory(State::Type::CHECK);
        TaskRun::instance().show();
        return;
    }

    Stats::getInstance().onReject();
    Stats::getInstance().save();
    App::diag().addWarning(State::ErrorCode::BRAKING_PROCESS, "Процесс прерван пользователем");
}

}  // namespace Screen
