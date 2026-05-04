#include "Service.h"

#include "App/App.h"
#include "Data.h"
#include "Screen/Page/Main/Main.h"
#include "Screen/Page/Service/Calibration.h"
#include "Screen/Page/Service/Guillotine.h"
#include "Screen/Page/Service/Paper.h"
#include "Screen/Page/Service/Service2.h"
#include "Screen/Page/Service/Slice.h"
#include "Screen/Page/Service/Table.h"
#include "Screen/Page/Task/TaskRun.h"
#include "Screen/Panel/LvglHelpers.h"
#include "Service/Stats.h"
#include "State/State.h"

#include <ui/screens.h>

namespace Screen {

Service& Service::instance() {
    static Service page;
    return page;
}

void Service::onPrepare() {
    Ui::onPop(objects.service_back, Service::popBack);
    Ui::onPop(objects.next_2, Service::popNext);
    Ui::onPop(objects.service_table, Service::popTable);
    Ui::onPop(objects.service_paper, Service::popPaper);
    Ui::onPop(objects.service_guillotine, Service::popGuillotine);
    Ui::onPop(objects.service_slice, Service::popSlice);
    Ui::onPop(objects.service_calibration, Service::popCalibration);
    Ui::onPop(objects.service_proba, Service::popProba);
}

void Service::onShow() {
    Data::work.clear();
    if (App::state() != nullptr) App::state()->setFactory(State::Type::SERVICE);
    Ui::setHidden(objects.next_2, App::ctx().swThrow == nullptr);
}

void Service::popBack(lv_event_t* e) {
    (void)e;
    if (App::state() != nullptr) App::state()->setFactory(State::Type::IDLE);
    App::plan().clear();
    Stats::getInstance().save();
    Main::instance().show();
}

void Service::popNext(lv_event_t* e) {
    (void)e;
    Service2::instance().show();
}

void Service::popTable(lv_event_t* e) {
    (void)e;
    Table::instance().show();
}

void Service::popPaper(lv_event_t* e) {
    (void)e;
    Paper::instance().show();
}

void Service::popGuillotine(lv_event_t* e) {
    (void)e;
    Guillotine::instance().show();
}

void Service::popSlice(lv_event_t* e) {
    (void)e;
    Slice::instance().show();
}

void Service::popCalibration(lv_event_t* e) {
    (void)e;
    Calibration::instance().show();
}

void Service::popProba(lv_event_t* e) {
    (void)e;
    Data::param.frame = 1;
    TaskRun::instance().setBackPage(Catalog::PageMode::pService);
    TaskRun::instance().show();
}

}  // namespace Screen
