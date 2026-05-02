#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class service : public Page {
public:
    using Page::Page;
    static service& getInstance();

    PageId id() const override { return PageId::Service; }
};

namespace Service {
using service = Screen::service;
}

} // namespace Screen
