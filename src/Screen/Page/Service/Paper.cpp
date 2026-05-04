#include "Paper.h"

#include "Screen/Page/Service/Service.h"
#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Paper& Paper::instance() {
    static Paper page;
    return page;
}

void Paper::onPrepare() {
    Ui::onPop(objects.paper_back, Paper::popBack);
}

void Paper::popBack(lv_event_t* e) {
    (void)e;
    Service::instance().show();
}

}  // namespace Screen
