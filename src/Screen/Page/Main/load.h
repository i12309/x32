#pragma once

#include "Screen/Page/Page.h"

namespace Screen::Main {

class load : public Page {
public:
    using Page::Page;
    PageId id() const override { return PageId::Load; }

    void setStatus(const String& text) { model().main.load.statusText.setText(text); }
    void setProgressColor(uint8_t index, uint32_t color);
};

} // namespace Screen::Main
