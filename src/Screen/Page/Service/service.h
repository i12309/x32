#pragma once

#include "Screen/Page/Page.h"

namespace Screen::Service {

class service : public Page {
public:
    using Page::Page;
    PageId id() const override { return PageId::Service; }
};

} // namespace Screen::Service
