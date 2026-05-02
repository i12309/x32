#pragma once

#include "Screen/Page/Page.h"

namespace Screen::Service {

class paper : public Page {
public:
    using Page::Page;
    PageId id() const override { return PageId::Paper; }
};

} // namespace Screen::Service
