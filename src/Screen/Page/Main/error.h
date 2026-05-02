#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class error : public Page {
public:
    using Page::Page;
    static error& getInstance();

    PageId id() const override { return PageId::Error; }

    void showError(const String& title, const String& message);
};

namespace Main {
using error = Screen::error;
}

} // namespace Screen
