#pragma once

#include "Screen/Panel/Model/ScreenModel.h"

namespace Screen {

struct MainLoadModel {
    TextBinding statusText;
    ColorBinding progress[8];
};

struct MainModel {
    MainLoadModel load;
};

} // namespace Screen
