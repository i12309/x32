#pragma once

#include "Screen/Page/Page.h"

namespace Screen::Main {

class main : public Page {
public:
    using Page::Page;
    PageId id() const override { return PageId::Main; }
};

} // namespace Screen::Main
