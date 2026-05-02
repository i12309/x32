#pragma once

#include "Screen/Page/Page.h"

namespace Screen::Task {

class task_run : public Page {
public:
    using Page::Page;
    PageId id() const override { return PageId::TaskRun; }
};

} // namespace Screen::Task
