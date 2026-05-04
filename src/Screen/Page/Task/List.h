#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>
#include <cstdint>

namespace Screen {

class List : public Page {
public:
    struct ListRow {
        String text;
        bool checked = false;
        bool checkVisible = true;
        bool editVisible = true;
        void* userData = nullptr;
    };

    struct ListCallbacks {
        lv_event_cb_t onBack = nullptr;
        lv_event_cb_t onAdd = nullptr;
        lv_event_cb_t onDelete = nullptr;
        lv_event_cb_t onNext = nullptr;
        lv_event_cb_t onRow = nullptr;
        lv_event_cb_t onEdit = nullptr;
        lv_event_cb_t onCheck = nullptr;
    };

    static constexpr size_t kMaxRows = 6;

    static List& instance();
    static int selectedIndex(lv_event_t* e);
    static void* selectedUserData(lv_event_t* e);

    void showList(const String& title,
                  const ListRow* rows,
                  size_t rowCount,
                  const ListCallbacks& callbacks);

protected:
    void onPrepare() override;

private:
    List() : Page(SCREEN_ID_LIST) {}

    struct RowObjects {
        RowObjects() = default;
        RowObjects(lv_obj_t* checkObj, lv_obj_t* itemObj, lv_obj_t* editObj)
            : check(checkObj), item(itemObj), edit(editObj) {}

        lv_obj_t* check = nullptr;
        lv_obj_t* item = nullptr;
        lv_obj_t* edit = nullptr;
    };

    void render();
    void attachRowEvents();
    RowObjects rowObjects(size_t index) const;

    static void popBack(lv_event_t* e);
    static void popAdd(lv_event_t* e);
    static void popDelete(lv_event_t* e);
    static void popNext(lv_event_t* e);
    static void popRow(lv_event_t* e);
    static void popEdit(lv_event_t* e);
    static void popCheck(lv_event_t* e);

    static void call(lv_event_cb_t cb, lv_event_t* e);

    ListRow rows_[kMaxRows];
    size_t rowCount_ = 0;
    ListCallbacks callbacks_;
};

}  // namespace Screen
