#include "Screen/Panel/Panel.h"

namespace Screen {

bool Panel::init() {
    return true;
}

void Panel::process() {}

void Panel::show(PageId page) {
    activePage_ = page;
}

} // namespace Screen
