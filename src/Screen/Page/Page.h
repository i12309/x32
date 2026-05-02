#pragma once

#include "Screen/Panel/Panel.h"

namespace Screen {

class Page {
public:
    explicit Page(Panel& panel) : panel_(panel) {}
    virtual ~Page() = default;

    virtual PageId id() const = 0;
    virtual void show() { panel_.show(id()); }
    virtual void process() {}

protected:
    PanelModel& model() { return panel_.model(); }
    Panel& panel_;
};

} // namespace Screen
