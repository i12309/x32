#include "Input.h"

#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Input& Input::instance() {
    static Input page;
    return page;
}

void Input::showInput(const String& title,
                      const String& info1,
                      const String& info2,
                      const String& defaultValue,
                      InputCallback onOk,
                      CancelCallback onCancel,
                      int showField,
                      bool autoBack) {
    Input& page = instance();
    page.show();

    Ui::setText(objects.input_title_1, title);
    Ui::setText(objects.input_field1, info1);
    Ui::setText(objects.input_field2, info2);

    if (showField) {
        Ui::setText(objects.input_field3, defaultValue);
        Ui::setText(objects.input_field4, "Введите информацию");
    } else {
        Ui::setText(objects.input_field3, "");
        Ui::setText(objects.input_field4, "");
    }

    page.resetHandlers();
    page.onOkHandler_ = onOk;
    page.onCancelHandler_ = onCancel;
    page.autoBackOnClose_ = autoBack;
}

void Input::onPrepare() {
    Ui::onPop(objects.input_ok, Input::popOk);
    Ui::onPop(objects.input_cancel, Input::popCancel);
}

void Input::resetHandlers() {
    onOkHandler_ = nullptr;
    onCancelHandler_ = nullptr;
    autoBackOnClose_ = true;
}

void Input::popOk(lv_event_t* e) {
    (void)e;
    Input& page = instance();
    String value = Ui::getText(objects.input_field3);
    value.trim();

    InputCallback callback = page.onOkHandler_;
    bool shouldAutoBack = page.autoBackOnClose_;
    page.resetHandlers();

    if (callback) callback(value);
    if (shouldAutoBack) page.back();
}

void Input::popCancel(lv_event_t* e) {
    (void)e;
    Input& page = instance();
    CancelCallback callback = page.onCancelHandler_;
    bool shouldAutoBack = page.autoBackOnClose_;
    page.resetHandlers();

    if (callback) callback();
    if (shouldAutoBack) page.back();
}

}  // namespace Screen
