#include "Bigel.h"

#include "Screen/Page/Service/Service2.h"
#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Bigel& Bigel::instance() {
    static Bigel page;
    return page;
}

void Bigel::onPrepare() {
    Ui::onPop(objects.bigel_back, Bigel::popBack);
}

void Bigel::popBack(lv_event_t* e) {
    (void)e;
    Service2::instance().show();
}

}  // namespace Screen
