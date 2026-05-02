#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class main : public Page {
public:
    using Page::Page;
    static main& getInstance();

    PageId id() const override { return PageId::Main; }
};

namespace Main {
using main = Screen::main;
}

} // namespace Screen
