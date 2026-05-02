#pragma once

#include "Screen/Panel/Model/ScreenModel.h"

namespace Screen {

struct TaskModel {
    TextBinding title;
    TextBinding status;
    ValueBinding progress;
};

} // namespace Screen
