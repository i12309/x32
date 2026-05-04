#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>
#include <functional>

namespace Screen {

class Wait : public Page {
public:
    static Wait& instance();

    static void wait(const String& text1,
                     const String& text2,
                     const String& text3,
                     int timeMs,
                     std::function<void()> callback,
                     bool toBack = true);

protected:
    void onShow() override;

private:
    Wait() : Page(SCREEN_ID_WAIT) {}

    void setTexts(const String& text1, const String& text2, const String& text3);
};

}  // namespace Screen
