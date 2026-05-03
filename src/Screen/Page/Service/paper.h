#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class paper : public Page {
public:
    using Page::Page;
    static paper& getInstance();

};

namespace Service {
using paper = Screen::paper;
}

} // namespace Screen
