#include "Page.h"

#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

PageMenu& PageMenu::instance() {
    static PageMenu page;
    return page;
}

int PageMenu::selectedIndex(lv_event_t* e) {
    return static_cast<int>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
}

void PageMenu::showMenu(const String& title,
                        const MenuItem* items,
                        size_t itemCount,
                        lv_event_cb_t onBack) {
    onBack_ = onBack;
    itemCount_ = min(itemCount, kMaxItems);

    for (size_t i = 0; i < kMaxItems; ++i) {
        items_[i] = MenuItem();
        if (items != nullptr && i < itemCount_) items_[i] = items[i];
    }

    show();
    // TODO(ui-lvgl): rename obj163 in EEZ to page_title.
    Ui::setText(objects.obj163, title);
    render();
}

void PageMenu::onPrepare() {
    // TODO(ui-lvgl): rename obj162 in EEZ to page_back.
    Ui::onPop(objects.obj162, PageMenu::popBack);

    for (size_t i = 0; i < kMaxItems; ++i) {
        Ui::onPop(itemObject(i), PageMenu::popItem, reinterpret_cast<void*>(i));
    }
}

void PageMenu::render() {
    // TODO(ui-lvgl): rename obj165-obj169 in EEZ to page_item_N.
    for (size_t i = 0; i < kMaxItems; ++i) {
        lv_obj_t* obj = itemObject(i);
        bool visible = i < itemCount_ && items_[i].visible;
        Ui::setHidden(obj, !visible);
        if (visible) Ui::setText(obj, items_[i].title);
    }

    // TODO(ui-lvgl): obj164 is the generated Next button; PageMenu does not use it yet.
    Ui::setHidden(objects.obj164, true);
}

lv_obj_t* PageMenu::itemObject(size_t index) const {
    switch (index) {
        case 0: return objects.obj165;
        case 1: return objects.obj166;
        case 2: return objects.obj167;
        case 3: return objects.obj168;
        case 4: return objects.obj169;
        default: return nullptr;
    }
}

void PageMenu::popBack(lv_event_t* e) {
    PageMenu& page = instance();
    if (page.onBack_ != nullptr) {
        page.onBack_(e);
    } else {
        page.back();
    }
}

void PageMenu::popItem(lv_event_t* e) {
    int index = selectedIndex(e);
    PageMenu& page = instance();
    if (index < 0 || static_cast<size_t>(index) >= page.itemCount_) return;
    if (!page.items_[index].visible || page.items_[index].onPop == nullptr) return;
    page.items_[index].onPop(e);
}

}  // namespace Screen
