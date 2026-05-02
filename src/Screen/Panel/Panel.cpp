#include "Screen/Panel/Panel.h"

#include "Screen/Page/Page.h"

namespace Screen {

bool Panel::init() {
    return true;
}

void Panel::process() {
    if (activePageObject_ != nullptr) {
        activePageObject_->process();
    }
}

void Panel::show(PageId page) {
    activePage_ = page;
}

void Panel::show(Page& page) {
    if (activePageObject_ != &page) {
        if (activePageObject_ != nullptr) {
            activePageObject_->hide();
        }
        previousPageObject_ = activePageObject_;
        activePageObject_ = &page;
    }

    show(page.id());
}

void Panel::back() {
    if (previousPageObject_ == nullptr) return;

    if (activePageObject_ != nullptr) {
        activePageObject_->hide();
    }

    activePageObject_ = previousPageObject_;
    previousPageObject_ = nullptr;
    show(activePageObject_->id());
}

void Panel::setLoadStatus(const String& text) {
    (void) text;
}

void Panel::setLoadProgressColor(uint8_t index, uint32_t color) {
    (void) index;
    (void) color;
}

void Panel::showInfo(const String& text1,
                     const String& text2,
                     const String& text3,
                     const String& okText,
                     const String& cancelText,
                     bool cancelVisible) {
    (void) text1;
    (void) text2;
    (void) text3;
    (void) okText;
    (void) cancelText;
    (void) cancelVisible;
}

void Panel::showError(const String& title, const String& message) {
    (void) title;
    (void) message;
}

} // namespace Screen
