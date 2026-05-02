#pragma once

#include "Screen/Page/Page.h"

namespace Screen::Task {

class task : public Page {
public:
    using Page::Page;
    PageId id() const override { return PageId::Task; }
};

} // namespace Screen::Task
