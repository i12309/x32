#include "Task.h"

#include "Core.h"
#include "Data.h"
#include "Screen/Page/Main/Info.h"
#include "Screen/Page/Profile/ProfileList.h"
#include "Screen/Page/Task/TaskList.h"
#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Task& Task::instance() {
    static Task page;
    return page;
}

void Task::onPrepare() {
    Ui::onPop(objects.task_back, Task::popBack);
    Ui::onPop(objects.task_save, Task::popSave);
    Ui::onPop(objects.task_del, Task::popDelete);
    Ui::onPop(objects.task_list_profile, Task::popListProfile);
}

void Task::onShow() {
    applyFormMode();
    fillFromWork();
}

void Task::applyFormMode() {
    Ui::setHidden(objects.task_del, formMode_ != Catalog::FormMode::EDIT);
}

void Task::fillFromWork() {
    Ui::setText(objects.task_title, formMode_ == Catalog::FormMode::CREATE ? "Новое задание" : "Задание");
    Ui::setText(objects.task_name, Data::work.task.valid() ? Data::work.task.NAME : "");
    Ui::setText(objects.task_product_mm, String(Data::work.task.PRODUCT_mm));
    Ui::setText(objects.task_over_mm, String(Data::work.task.OVER_mm));
    Ui::setText(objects.task_last_cut_mm, String(Data::work.task.LASTCUT_mm));
    Ui::setText(objects.task_first_cut_mm,
                Data::work.task.MARK == 0 ? String(Data::work.task.FIRST_CUT_mm) : String(Data::work.task.MARK_mm));
    Ui::dropdownSetSelected(objects.obj42, Data::work.task.MARK == 0 ? 1 : 0);
    Ui::setText(objects.task_list_profile,
                Data::work.profile.valid() ? Data::work.profile.NAME : "Выберите профиль");
}

bool Task::validateTaskInputs(const String& name, const String& product) const {
    String trimmedName = name;
    trimmedName.trim();
    if (trimmedName.isEmpty()) {
        Info::showInfo("Задание", "Не заполнено название", "", nullptr, nullptr, true);
        return false;
    }
    if (!Data::work.profile.valid()) {
        Info::showInfo("Задание", "Не выбран профиль", "", nullptr, nullptr, true);
        return false;
    }
    float productValue = atof(product.c_str());
    if (!T::isStringValidFloat(product.c_str()) || productValue <= 0.0f) {
        Info::showInfo("Задание", "Изделие должно быть > 0", "", nullptr, nullptr, true);
        return false;
    }
    return true;
}

void Task::saveTask(bool create) {
    String name = Ui::getText(objects.task_name);
    String product = Ui::getText(objects.task_product_mm);
    String over = Ui::getText(objects.task_over_mm);
    String lastCut = Ui::getText(objects.task_last_cut_mm);
    String firstCut = Ui::getText(objects.task_first_cut_mm);

    name.trim();
    product.trim();
    over.trim();
    lastCut.trim();
    firstCut.trim();

    if (!validateTaskInputs(name, product)) return;
    if (!T::isStringValidFloat(over.c_str()) ||
        !T::isStringValidFloat(lastCut.c_str()) ||
        !T::isStringValidFloat(firstCut.c_str())) {
        Info::showInfo("Задание", "Некорректные числовые параметры", "", nullptr, nullptr, true);
        return;
    }

    const bool withoutMark = Ui::dropdownSelected(objects.obj42) == 1;
    if (create) Data::work.task.setID(Data::tasks.maxID());

    Data::work.task.NAME = name;
    Data::work.task.PRODUCT_mm = atof(product.c_str());
    Data::work.task.OVER_mm = atof(over.c_str());
    Data::work.task.LASTCUT_mm = atof(lastCut.c_str());
    Data::work.task.MARK = withoutMark ? 0 : 1;
    Data::work.task.FIRST_CUT_mm = withoutMark ? atof(firstCut.c_str()) : 0;
    Data::work.task.MARK_mm = withoutMark ? 0 : atof(firstCut.c_str());
    Data::work.task.PROFILE_ID = Data::work.profile.ID;

    if (create) {
        Data::tasks.add(Data::work.task);
    } else if (Data::work.task.valid()) {
        Data::tasks.edit(Data::work.task);
        Data::work.task.clear();
    }

    Data::work.profile.clear();
    TaskList::instance().show();
}

void Task::popBack(lv_event_t* e) {
    (void)e;
    TaskList::instance().show();
}

void Task::popSave(lv_event_t* e) {
    (void)e;
    Task& page = instance();
    page.saveTask(page.formMode_ == Catalog::FormMode::CREATE);
}

void Task::popDelete(lv_event_t* e) {
    (void)e;
    if (!Data::work.task.valid()) return;

    Info::showInfo("Удаление задания", "Подтвердите удаление", Data::work.task.NAME,
                   []() {
                       Data::tasks.remove(Data::work.task.ID);
                       Data::work.task.clear();
                       TaskList::instance().show();
                   },
                   nullptr,
                   true);
}

void Task::popListProfile(lv_event_t* e) {
    (void)e;
    ProfileList::instance().setBackPage(Catalog::PageMode::pTask);
    ProfileList::instance().show();
}

}  // namespace Screen
