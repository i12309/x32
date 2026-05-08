#include "Service2.h"

#include "Screen/Page/Service/Bigel.h"
#include "Screen/Page/Service/Service.h"
#include "Screen/Page/Service/Throws.h"
#include "Screen/Panel/LvglHelpers.h"

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

}  // namespace Screen
