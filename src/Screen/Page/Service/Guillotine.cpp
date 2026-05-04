#include "Guillotine.h"

#include "Screen/Page/Service/Service.h"
#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Guillotine& Guillotine::instance() {
    static Guillotine page;
    return page;
}

void Guillotine::onPrepare() {
    Ui::onPop(objects.guillotine_back, Guillotine::popBack);
}

void Guillotine::popBack(lv_event_t* e) {
    (void)e;
    Service::instance().show();
}

}  // namespace Screen
