#include "Screen/Page/Main/info.h"

#include "App/App.h"

namespace Screen {

info& info::getInstance() {
    static info page(App::panel());
    return page;
}

void info::showInfo(const String& text1,
                    const String& text2,
                    const String& text3,
                    std::function<void()> onOk,
                    std::function<void()> onCancel,
                    bool showCancel,
                    const String& okText,
                    const String& cancelText) {
    info& page = getInstance();
    page.okCallback_ = onOk;
    page.cancelCallback_ = onCancel;

    page.show();
    page.panel_.showInfo(text1,
                         text2,
                         text3,
                         okText.length() > 0 ? okText : "OK",
                         cancelText.length() > 0 ? cancelText : "Cancel",
                         showCancel || onCancel != nullptr);
}

void info::accept() {
    std::function<void()> callback = okCallback_;
    okCallback_ = nullptr;
    cancelCallback_ = nullptr;

    if (callback) {
        callback();
    } else {
        back();
    }
}

void info::cancel() {
    std::function<void()> callback = cancelCallback_;
    okCallback_ = nullptr;
    cancelCallback_ = nullptr;

    if (callback) {
        callback();
    } else {
        back();
    }
}

} // namespace Screen
