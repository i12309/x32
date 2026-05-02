#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class guillotine : public Page {
public:
    using Page::Page;
    static guillotine& getInstance();

    PageId id() const override { return PageId::Guillotine; }
};

namespace Service {
using guillotine = Screen::guillotine;
}

} // namespace Screen
