#include "Info.h"

#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Info& Info::instance() {
    static Info page;
    return page;
}

void Info::showInfo(const String& text1,
                    const String& text2,
                    const String& text3,
                    std::function<void()> onOk,
                    std::function<void()> onCancel,
                    bool showCancel,
                    const String& okText,
                    const String& cancelText) {
    Info& page = instance();
    page.okCallback_ = onOk;
    page.cancelCallback_ = onCancel;
    page.show();
    page.render(text1, text2, text3, okText, cancelText, showCancel || onCancel != nullptr);
}

void Info::onPrepare() {
    Ui::onPop(objects.info_ok, Info::popOk);
    Ui::onPop(objects.info_cancel, Info::popCancel);
}

void Info::render(const String& text1,
                  const String& text2,
                  const String& text3,
                  const String& okText,
                  const String& cancelText,
                  bool showCancel) {
    Ui::setText(objects.info_title, text1);
    Ui::setText(objects.info_field1, text2);
    Ui::setText(objects.info_field2, text3);
    Ui::setText(objects.info_field3, "");

    Ui::setText(objects.info_ok, okText.length() > 0 ? okText : "OK");
    Ui::setText(objects.info_cancel, cancelText.length() > 0 ? cancelText : "Отмена");
    Ui::setHidden(objects.info_cancel, !showCancel);
}

void Info::resetCallbacks() {
    okCallback_ = nullptr;
    cancelCallback_ = nullptr;
}

void Info::popOk(lv_event_t* e) {
    (void)e;
    Info& page = instance();
    std::function<void()> callback = page.okCallback_;
    page.resetCallbacks();

    if (callback) {
        callback();
    } else {
        page.back();
    }
}

void Info::popCancel(lv_event_t* e) {
    (void)e;
    Info& page = instance();
    std::function<void()> callback = page.cancelCallback_;
    page.resetCallbacks();

    if (callback) {
        callback();
    } else {
        page.back();
    }
}

}  // namespace Screen
