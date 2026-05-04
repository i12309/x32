#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>

namespace Screen {

class Load : public Page {
public:
    static Load& instance();

    bool checkVersion();
    void setModel(const String& text);

protected:
    void onShow() override;
    void onTick() override;

private:
    Load() : Page(SCREEN_ID_LOAD) {}
};

}  // namespace Screen
