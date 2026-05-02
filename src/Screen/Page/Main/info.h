#pragma once

#include <functional>

#include "Screen/Page/Page.h"

namespace Screen {

class info : public Page {
public:
    using Page::Page;
    static info& getInstance();

    PageId id() const override { return PageId::Info; }

    static void showInfo(const String& text1 = "",
                         const String& text2 = "",
                         const String& text3 = "",
                         std::function<void()> onOk = nullptr,
                         std::function<void()> onCancel = nullptr,
                         bool showCancel = false,
                         const String& okText = "",
                         const String& cancelText = "");

    void accept();
    void cancel();

private:
    std::function<void()> okCallback_ = nullptr;
    std::function<void()> cancelCallback_ = nullptr;
};

namespace Main {
using info = Screen::info;
}

} // namespace Screen
