#include "Slice.h"

#include "App/App.h"
#include "Catalog.h"
#include "Core.h"
#include "Data.h"
#include "Screen/Page/Main/Info.h"
#include "Screen/Page/Profile/ProfileList.h"
#include "Screen/Page/Service/Service.h"
#include "Screen/Panel/LvglHelpers.h"
#include "State/State.h"

#include <ui/screens.h>

namespace Screen {

Slice& Slice::instance() {
    static Slice page;
    return page;
}

void Slice::onPrepare() {
    Ui::onPop(objects.slice_back, Slice::popBack);
    Ui::onPop(objects.slice_list_profile, Slice::popProfileList);
    Ui::onPop(objects.slice_minus, Slice::popMinus);
    Ui::onPop(objects.slice_plus, Slice::popPlus);
    Ui::onPop(objects.slice_go, Slice::popGo);
}

void Slice::onShow() {
    Ui::setText(objects.slice_list_profile,
                Data::work.profile.valid() ? Data::work.profile.NAME : "Выберите профиль");
    if (Ui::getText(objects.slice_count_paper).isEmpty()) Ui::setText(objects.slice_count_paper, "1");
}

void Slice::popBack(lv_event_t* e) {
    (void)e;
    Service::instance().show();
}

void Slice::popProfileList(lv_event_t* e) {
    (void)e;
    ProfileList::instance().setBackPage(Catalog::PageMode::pSlice);
    ProfileList::instance().show();
}

void Slice::popMinus(lv_event_t* e) {
    (void)e;
    int count = Ui::getText(objects.slice_count_paper).toInt() - 1;
    if (count < 1) count = 1;
    Ui::setText(objects.slice_count_paper, String(count));
}

void Slice::popPlus(lv_event_t* e) {
    (void)e;
    int count = Ui::getText(objects.slice_count_paper).toInt() + 1;
    if (count > 999) count = 999;
    Ui::setText(objects.slice_count_paper, String(count));
}

void Slice::popGo(lv_event_t* e) {
    (void)e;
    if (!Data::work.profile.valid()) {
        Info::showInfo("Резка", "Не выбран профиль", "", nullptr, nullptr, true);
        return;
    }
    if (App::state() != nullptr) {
        Data::work.TOTAL_CYCLES = Ui::getText(objects.slice_count_paper).toInt();
        App::state()->setNexTypeState(State::Type::SLICE);
        App::state()->setFactory(State::Type::CHECK);
    }
}

}  // namespace Screen
