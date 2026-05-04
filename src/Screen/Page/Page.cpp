#include "Page.h"

namespace Screen {

Page* Page::activePage_ = nullptr;
Page* Page::previousPage_ = nullptr;

Page::Page(ScreensEnum screenId) : screenId_(screenId) {}

void Page::show() {
    prepareOnce();

    if (activePage_ != this) {
        Page* oldPage = activePage_;
        if (oldPage != nullptr) oldPage->onHide();
        previousPage_ = oldPage;
        activePage_ = this;
    }

    loadScreen(screenId_);
    onShow();
}

void Page::hide() {
    if (activePage_ == this) activePage_ = nullptr;
    onHide();
}

void Page::back() {
    Page* target = previousPage_;
    if (target == nullptr || target == this) return;

    previousPage_ = nullptr;
    if (activePage_ != nullptr && activePage_ != target) {
        activePage_->onHide();
    }

    activePage_ = target;
    target->prepareOnce();
    loadScreen(target->screenId_);
    target->onShow();
}

void Page::process() {
    if (activePage_ == nullptr) return;
    activePage_->onTick();
}

Page* Page::activePage() {
    return activePage_;
}

Page* Page::previousPage() {
    return previousPage_;
}

void Page::prepareOnce() {
    if (prepared_) return;
    prepared_ = true;
    onPrepare();
}

}  // namespace Screen
