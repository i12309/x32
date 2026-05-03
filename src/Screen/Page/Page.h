#pragma once

#include "Screen/Panel/Panel.h"

namespace Screen {

class Page {
public:
    explicit Page(Panel& panel) : panel_(panel) {}
    virtual ~Page() = default;

    virtual void show() { panel_.show(*this); }
    virtual void hide() {}
    virtual void back() { panel_.back(); }
    virtual void process() {}

protected:
    Panel& panel_;
};

} // namespace Screen
