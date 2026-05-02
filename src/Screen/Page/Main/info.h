#pragma once

#include "Screen/Page/Page.h"

namespace Screen::Main {

class info : public Page {
public:
    using Page::Page;
    PageId id() const override { return PageId::Info; }
};

} // namespace Screen::Main
