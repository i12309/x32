#include "Service2.h"

#include "App/App.h"
#include "Screen/Page/Service/Bigel.h"
#include "Screen/Page/Service/Service.h"
#include "Screen/Page/Service/Throws.h"
#include "Screen/Panel/LvglHelpers.h"
#include "State/State.h"

#include <ui/screens.h>

namespace Screen {

Service2& Service2::instance() {
    static Service2 page;
    return page;
}

void Service2::onPrepare() {
    Ui::onPop(objects.service2_back, Service2::popBack);
    Ui::onPop(objects.service_table_1, Service2::popThrow);
    Ui::onPop(objects.service_paper_1, Service2::popBigel);
    Ui::onPop(objects.service_guillotine_1, Service2::popTest1);
    Ui::onPop(objects.obj114, Service2::popTest2);
    Ui::onPop(objects.obj115, Service2::popTest3);
}

void Service2::popBack(lv_event_t* e) {
    (void)e;
    Service::instance().show();
}

void Service2::popThrow(lv_event_t* e) {
    (void)e;
    Throws::instance().show();
}

void Service2::popBigel(lv_event_t* e) {
    (void)e;
    Bigel::instance().show();
}

void Service2::popTest1(lv_event_t* e) {
    (void)e;
    if (App::state() != nullptr) {
        App::state()->setNexTypeState(State::Type::T100B);
        App::state()->setFactory(State::Type::CHECK);
    }
}

void Service2::popTest2(lv_event_t* e) {
    (void)e;
    if (App::state() != nullptr) {
        App::state()->setNexTypeState(State::Type::TDL);
        App::state()->setFactory(State::Type::CHECK);
    }
}

void Service2::popTest3(lv_event_t* e) {
    (void)e;
    if (App::state() != nullptr) {
        App::state()->setNexTypeState(State::Type::TCUT);
        App::state()->setFactory(State::Type::CHECK);
    }
}

}  // namespace Screen
