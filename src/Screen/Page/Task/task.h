#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class task : public Page {
public:
    using Page::Page;
    static task& getInstance();

};

namespace Task {
using task = Screen::task;
}

} // namespace Screen
