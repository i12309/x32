#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>
#include <cstdint>

namespace Screen {

class PageMenu : public Page {
public:
    struct MenuItem {
        MenuItem() = default;
        MenuItem(const String& itemTitle, lv_event_cb_t itemOnPop, bool itemVisible = true)
            : title(itemTitle), onPop(itemOnPop), visible(itemVisible) {}

        String title;
        lv_event_cb_t onPop = nullptr;
        bool visible = true;
    };

    static constexpr size_t kMaxItems = 5;

    static PageMenu& instance();
    static int selectedIndex(lv_event_t* e);

    void showMenu(const String& title,
                  const MenuItem* items,
                  size_t itemCount,
                  lv_event_cb_t onBack = nullptr);

protected:
    void onPrepare() override;

private:
    PageMenu() : Page(SCREEN_ID_PAGE) {}

    void render();
    lv_obj_t* itemObject(size_t index) const;

    static void popBack(lv_event_t* e);
    static void popItem(lv_event_t* e);

    MenuItem items_[kMaxItems];
    size_t itemCount_ = 0;
    lv_event_cb_t onBack_ = nullptr;
};

}  // namespace Screen
