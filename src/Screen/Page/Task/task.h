#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class task : public Page {
public:
    using Page::Page;
    static task& getInstance();

    PageId id() const override { return PageId::Task; }
};

namespace Task {
using task = Screen::task;
}

} // namespace Screen
