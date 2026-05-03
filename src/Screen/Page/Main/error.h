#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class error : public Page {
public:
    using Page::Page;
    static error& getInstance();

    void showError(const String& title, const String& message);
};

namespace Main {
using error = Screen::error;
}

} // namespace Screen
