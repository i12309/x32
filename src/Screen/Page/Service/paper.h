#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class paper : public Page {
public:
    using Page::Page;
    static paper& getInstance();

    PageId id() const override { return PageId::Paper; }
};

namespace Service {
using paper = Screen::paper;
}

} // namespace Screen
