#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class load : public Page {
public:
    using Page::Page;
    static load& getInstance();

    PageId id() const override { return PageId::Load; }

    void setStatus(const String& text) { panel_.setLoadStatus(text); }
    void setProgressColor(uint8_t index, uint32_t color);
};

namespace Main {
using load = Screen::load;
}

} // namespace Screen
