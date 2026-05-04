#include "Throws.h"

#include "Screen/Page/Service/Service2.h"
#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Throws& Throws::instance() {
    static Throws page;
    return page;
}

void Throws::onPrepare() {
    Ui::onPop(objects.throws_back, Throws::popBack);
}

void Throws::popBack(lv_event_t* e) {
    (void)e;
    Service2::instance().show();
}

}  // namespace Screen
