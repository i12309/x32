#pragma once

#include "Screen/Page/Page.h"

namespace Screen::Service {

class guillotine : public Page {
public:
    using Page::Page;
    PageId id() const override { return PageId::Guillotine; }
};

} // namespace Screen::Service
