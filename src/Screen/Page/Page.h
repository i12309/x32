#pragma once

#include <ui/ui.h>

namespace Screen {

class Page {
public:
    explicit Page(ScreensEnum screenId);
    virtual ~Page() = default;

    void show();
    void hide();
    void back();

    static void process();
    static Page* activePage();
    static Page* previousPage();

protected:
    virtual void onPrepare() {}
    virtual void onShow() {}
    virtual void onHide() {}
    virtual void onTick() {}

private:
    void prepareOnce();

    ScreensEnum screenId_;
    bool prepared_ = false;

    static Page* activePage_;
    static Page* previousPage_;
};

}  // namespace Screen
