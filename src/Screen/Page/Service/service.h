#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class service : public Page {
public:
    using Page::Page;
    static service& getInstance();

};

namespace Service {
using service = Screen::service;
}

} // namespace Screen
