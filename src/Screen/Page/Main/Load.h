#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>

namespace Screen {

class Load : public Page {
public:
    static Load& instance();

    bool checkVersion();

protected:
    void onShow() override;

private:
    Load() : Page(SCREEN_ID_LOAD) {}
};

}  // namespace Screen
