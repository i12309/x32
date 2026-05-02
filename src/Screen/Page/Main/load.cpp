#include "Screen/Page/Main/load.h"

namespace Screen::Main {

void load::setProgressColor(uint8_t index, uint32_t color) {
    if (index >= 8) return;
    model().main.load.progress[index].setColor(color);
}

} // namespace Screen::Main
