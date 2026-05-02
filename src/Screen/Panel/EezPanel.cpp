#include "Screen/Panel/EezPanel.h"

namespace Screen {

bool EezPanel::init() {
    return display_.init() && touch_.init() && Panel::init();
}

void EezPanel::process() {
    display_.process();
    touch_.process();
    Panel::process();
}

} // namespace Screen
