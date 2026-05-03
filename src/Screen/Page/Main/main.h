#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class main : public Page {
public:
    using Page::Page;
    static main& getInstance();

};

namespace Main {
using main = Screen::main;
}

} // namespace Screen
