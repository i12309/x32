#include "Wait.h"

#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Wait& Wait::instance() {
    static Wait page;
    return page;
}

void Wait::wait(const String& text1,
                const String& text2,
                const String& text3,
                int timeMs,
                std::function<void()> callback,
                bool toBack) {
    Wait& page = instance();
    page.show();
    page.setTexts(text1, text2, text3);

    if (timeMs > 0) delay(timeMs);
    if (callback) callback();
    if (toBack) page.back();
}

void Wait::onShow() {
    setTexts("", "", "");
}

void Wait::setTexts(const String& text1, const String& text2, const String& text3) {
    Ui::setText(objects.wait_text1, text1);
    Ui::setText(objects.wait_text2, text2);
    Ui::setText(objects.wait_text3, text3);
}

}  // namespace Screen
