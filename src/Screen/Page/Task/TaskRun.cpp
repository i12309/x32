#include "TaskRun.h"

#include "App/App.h"
#include "Core.h"
#include "Data.h"
#include "Screen/Page/Main/Info.h"
#include "Screen/Page/Main/Main.h"
#include "Screen/Page/Profile/ProfileList.h"
#include "Screen/Page/Service/Service.h"
#include "Screen/Page/Task/TaskList.h"
#include "Screen/Page/Task/TaskProcess.h"
#include "Screen/Panel/LvglHelpers.h"
#include "State/State.h"

#include <ui/screens.h>

namespace Screen {

TaskRun& TaskRun::instance() {
    static TaskRun page;
    return page;
}

void TaskRun::onPrepare() {
    Ui::onPop(objects.task_run_back, TaskRun::popBack);
    Ui::onPop(objects.task_run_start, TaskRun::popStart);
    Ui::onPop(objects.task_run_list_task, TaskRun::popListTask);
    Ui::onPop(objects.task_run_list_profile, TaskRun::popListProfile);
    Ui::onPop(objects.task_run_minus, TaskRun::popMinus);
    Ui::onPop(objects.task_run_plus, TaskRun::popPlus);
}

void TaskRun::onShow() {
    Ui::setText(objects.task_run_list_task,
                Data::work.task.valid() ? Data::work.task.NAME : "Выберите задание!");
    Ui::setText(objects.task_run_list_profile,
                Data::work.profile.valid() ? Data::work.profile.NAME : "Выберите профиль!");

    const bool fromMain = backPageStatus_ == Catalog::PageMode::pMain;
    Ui::setText(objects.task_run_cycles, fromMain ? "0" : "1");
    Ui::setHidden(objects.task_run_label, !fromMain);
    Ui::setHidden(objects.task_run_plus, !fromMain);
    Ui::setHidden(objects.task_run_minus, !fromMain);
}

void TaskRun::onTick() {
    if (App::state() == nullptr || App::state()->type() != State::Type::IDLE) return;
    //IButton* start = App::ctx().reg.getButton("START");    if (start != nullptr && start->isTrigger()) popStart(nullptr);
}

void TaskRun::popBack(lv_event_t* e) {
    (void)e;
    TaskRun& page = instance();
    if (page.backPageStatus_ == Catalog::PageMode::pService) {
        Service::instance().show();
        return;
    }
    Main::instance().show();
}

void TaskRun::popStart(lv_event_t* e) {
    (void)e;

    if (!Data::work.task.valid()) {
        Info::showInfo("", "Не выбрано задание", "", nullptr, nullptr, true);
        return;
    }
    if (!Data::work.profile.valid()) {
        Info::showInfo("", "Не выбран профиль", "", nullptr, nullptr, true);
        return;
    }

    String cycles = Ui::getText(objects.task_run_cycles);
    cycles.trim();
    if (!T::isStringValidInteger(cycles)) {
        Info::showInfo("", "Некорректное кол-во листов", "", nullptr, nullptr, true);
        return;
    }

    Data::work.TOTAL_CYCLES = cycles.toInt();
    if (Data::work.TOTAL_CYCLES <= 0) {
        Info::showInfo("", "Кол-во листов должно быть > 0", "", nullptr, nullptr, true);
        return;
    }

    Data::work.TOTAL_CUTS = Data::getCUTs_count();
    if (Data::work.TOTAL_CUTS <= 0) {
        Info::showInfo("", "Некорректные параметры задания", "", nullptr, nullptr, true);
        return;
    }

    App::state()->setNexTypeState(State::Type::PROCESS);
    App::state()->setFactory(State::Type::CHECK);
    TaskProcess::instance().show();
}

void TaskRun::popListTask(lv_event_t* e) {
    (void)e;
    TaskList::instance().setBackPage(Catalog::PageMode::pTaskRun);
    TaskList::instance().show();
}

void TaskRun::popListProfile(lv_event_t* e) {
    (void)e;
    ProfileList::instance().setBackPage(Catalog::PageMode::pTaskRun);
    ProfileList::instance().show();
}

void TaskRun::popMinus(lv_event_t* e) {
    (void)e;
    int count = Ui::getText(objects.task_run_cycles).toInt() - 1;
    if (count < 0) return;
    Ui::setText(objects.task_run_cycles, String(count));
}

void TaskRun::popPlus(lv_event_t* e) {
    (void)e;
    int count = Ui::getText(objects.task_run_cycles).toInt() + 1;
    if (count > 999) return;
    Ui::setText(objects.task_run_cycles, String(count));
}

}  // namespace Screen
