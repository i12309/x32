#include "Table.h"

#include "Screen/Page/Service/Service.h"
#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Table& Table::instance() {
    static Table page;
    return page;
}

void Table::onPrepare() {
    Ui::onPop(objects.table_back, Table::popBack);
}

void Table::popBack(lv_event_t* e) {
    (void)e;
    Service::instance().show();
}

}  // namespace Screen
