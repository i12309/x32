#pragma once

#include "Screen/Panel/DisplayDriver.h"
#include "Screen/Panel/Panel.h"
#include "Screen/Panel/TouchDriver.h"

namespace Screen {

class EezPanel : public Panel {
public:
    bool init() override;
    void process() override;

private:
    DisplayDriver display_;
    TouchDriver touch_;
};

} // namespace Screen
