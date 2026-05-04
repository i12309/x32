#include "Profile.h"

#include "App/App.h"
#include "Core.h"
#include "Data.h"
#include "Screen/Page/Main/Info.h"
#include "Screen/Page/Main/Wait.h"
#include "Screen/Page/Profile/ProfileList.h"
#include "Screen/Panel/LvglHelpers.h"
#include "State/State.h"

#include <ui/screens.h>

namespace Screen {

Profile& Profile::instance() {
    static Profile page;
    return page;
}

void Profile::onPrepare() {
    Ui::onPop(objects.profile_back, Profile::popBack);
    Ui::onPop(objects.profile_save, Profile::popSave);
    Ui::onPop(objects.profile_del, Profile::popDelete);
    // TODO(ui-lvgl): the "Профилировать" button has no named object in EEZ yet.
}

void Profile::onShow() {
    applyFormMode();
    fillFromWork();
}

void Profile::onTick() {
    if (App::state() != nullptr && App::state()->type() == State::Type::FINISH) {
        Ui::setText(objects.obj53, String(Data::work.profile.RATIO_mm, 3));
    }
}

void Profile::applyFormMode() {
    Ui::setHidden(objects.profile_del, formMode_ != Catalog::FormMode::EDIT);
}

void Profile::fillFromWork() {
    Ui::setText(objects.profile_title, formMode_ == Catalog::FormMode::CREATE ? "Новый профиль" : "Профиль");
    Ui::setText(objects.profile_name, Data::work.profile.valid() ? Data::work.profile.NAME : "");
    Ui::setText(objects.profile_name_1, Data::work.profile.valid() ? Data::work.profile.DESC : "");
    Ui::setText(objects.obj51, String(Data::work.profile.LENGHT_mm, 2));
    Ui::setText(objects.obj53, String(Data::work.profile.RATIO_mm, 3));
}

void Profile::saveProfile(bool create) {
    String name = Ui::getText(objects.profile_name);
    String desc = Ui::getText(objects.profile_name_1);
    String length = Ui::getText(objects.obj51);
    String ratio = Ui::getText(objects.obj53);
    name.trim();
    desc.trim();
    length.trim();
    ratio.trim();

    if (!T::isStringValidFloat(ratio.c_str()) || !T::isStringValidFloat(length.c_str())) {
        Info::showInfo("Профиль", "Некорректные числовые параметры", "", nullptr, nullptr, true);
        return;
    }

    if (create) Data::work.profile.setID(Data::profiles.maxID());
    Data::work.profile.NAME = name;
    Data::work.profile.DESC = desc;
    Data::work.profile.RATIO_mm = atof(ratio.c_str());
    Data::work.profile.LENGHT_mm = atof(length.c_str());

    if (create) {
        Data::profiles.add(Data::work.profile);
    } else if (Data::work.profile.valid()) {
        Data::profiles.edit(Data::work.profile);
    }

    if (App::state() != nullptr) App::state()->setFactory(State::Type::IDLE);
    ProfileList::instance().show();
}

void Profile::popBack(lv_event_t* e) {
    (void)e;
    if (App::state() != nullptr) App::state()->setFactory(State::Type::IDLE);
    ProfileList::instance().show();
}

void Profile::popSave(lv_event_t* e) {
    (void)e;
    Profile& page = instance();
    page.saveProfile(page.formMode_ == Catalog::FormMode::CREATE);
}

void Profile::popDelete(lv_event_t* e) {
    (void)e;
    if (!Data::work.profile.valid()) return;

    Info::showInfo("Удаление профиля", "Подтвердите удаление", Data::work.profile.NAME,
                   []() {
                       Data::profiles.remove(Data::work.profile.ID);
                       Data::work.profile.clear();
                       ProfileList::instance().show();
                   },
                   nullptr,
                   true);
}

void Profile::popProfile(lv_event_t* e) {
    (void)e;
    Info::showInfo("Начать профилирование", "Положите лист бумаги на стол", "для получения коэффициента",
                   []() {
                       Wait::wait("", "Профилирование...", "", 200,
                                  []() {
                                      App::state()->setNexTypeState(State::Type::PROFILING);
                                      App::state()->setFactory(State::Type::CHECK);
                                  },
                                  false);
                   },
                   nullptr,
                   true);
}

}  // namespace Screen
