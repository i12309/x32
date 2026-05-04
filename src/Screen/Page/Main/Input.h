#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>
#include <functional>

namespace Screen {

class Input : public Page {
public:
    using InputCallback = std::function<void(const String&)>;
    using CancelCallback = std::function<void()>;

    static Input& instance();

    static void showInput(const String& title,
                          const String& info1,
                          const String& info2,
                          const String& defaultValue = "",
                          InputCallback onOk = nullptr,
                          CancelCallback onCancel = nullptr,
                          int showField = 0,
                          bool autoBack = true);

protected:
    void onPrepare() override;

private:
    Input() : Page(SCREEN_ID_INPUT) {}

    void resetHandlers();

    static void popOk(lv_event_t* e);
    static void popCancel(lv_event_t* e);

    InputCallback onOkHandler_ = nullptr;
    CancelCallback onCancelHandler_ = nullptr;
    bool autoBackOnClose_ = true;
};

}  // namespace Screen
