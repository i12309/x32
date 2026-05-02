#pragma once

#include "Screen/Page/Page.h"

namespace Screen::Main {

class error : public Page {
public:
    using Page::Page;
    PageId id() const override { return PageId::Error; }
};

} // namespace Screen::Main
