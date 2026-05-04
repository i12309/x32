#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>
#include <functional>

namespace Screen {

class Info : public Page {
public:
    static Info& instance();

    static void showInfo(const String& text1 = "",
                         const String& text2 = "",
                         const String& text3 = "",
                         std::function<void()> onOk = nullptr,
                         std::function<void()> onCancel = nullptr,
                         bool showCancel = false,
                         const String& okText = "",
                         const String& cancelText = "");

protected:
    void onPrepare() override;

private:
    Info() : Page(SCREEN_ID_INFO) {}

    void render(const String& text1,
                const String& text2,
                const String& text3,
                const String& okText,
                const String& cancelText,
                bool showCancel);
    void resetCallbacks();

    static void popOk(lv_event_t* e);
    static void popCancel(lv_event_t* e);

    std::function<void()> okCallback_ = nullptr;
    std::function<void()> cancelCallback_ = nullptr;
};

}  // namespace Screen
