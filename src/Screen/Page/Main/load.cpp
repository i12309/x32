#include "Screen/Page/Main/load.h"

#include "App/App.h"

namespace Screen {

load& load::getInstance() {
    static load page(App::panel());
    return page;
}

void load::setProgressColor(uint8_t index, uint32_t color) {
    panel_.setLoadProgressColor(index, color);
}

} // namespace Screen
