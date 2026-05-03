#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class guillotine : public Page {
public:
    using Page::Page;
    static guillotine& getInstance();

};

namespace Service {
using guillotine = Screen::guillotine;
}

} // namespace Screen
