#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class task_run : public Page {
public:
    using Page::Page;
    static task_run& getInstance();

};

namespace Task {
using task_run = Screen::task_run;
}

} // namespace Screen
