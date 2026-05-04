#include "List.h"

#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

List& List::instance() {
    static List page;
    return page;
}

int List::selectedIndex(lv_event_t* e) {
    return static_cast<int>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
}

void* List::selectedUserData(lv_event_t* e) {
    int index = selectedIndex(e);
    List& page = instance();
    if (index < 0 || static_cast<size_t>(index) >= page.rowCount_) return nullptr;
    return page.rows_[index].userData;
}

void List::showList(const String& title,
                    const ListRow* rows,
                    size_t rowCount,
                    const ListCallbacks& callbacks) {
    callbacks_ = callbacks;
    rowCount_ = min(rowCount, kMaxRows);

    for (size_t i = 0; i < kMaxRows; ++i) {
        rows_[i] = ListRow();
        if (rows != nullptr && i < rowCount_) rows_[i] = rows[i];
    }

    show();
    Ui::setText(objects.list_title, title);
    render();
}

void List::onPrepare() {
    Ui::onPop(objects.list_back, List::popBack);
    Ui::onPop(objects.list_add, List::popAdd);
    Ui::onPop(objects.list_del, List::popDelete);
    Ui::onPop(objects.list_next, List::popNext);
    attachRowEvents();
}

void List::render() {
    Ui::setHidden(objects.list_add, callbacks_.onAdd == nullptr);
    Ui::setHidden(objects.list_del, callbacks_.onDelete == nullptr);
    Ui::setHidden(objects.list_next, callbacks_.onNext == nullptr);

    for (size_t i = 0; i < kMaxRows; ++i) {
        RowObjects row = rowObjects(i);
        const bool visible = i < rowCount_;
        Ui::setHidden(row.check, !visible || !rows_[i].checkVisible);
        Ui::setHidden(row.item, !visible);
        Ui::setHidden(row.edit, !visible || !rows_[i].editVisible);

        if (!visible) {
            Ui::setText(row.item, "");
            continue;
        }

        Ui::setText(row.item, rows_[i].text);
        Ui::setChecked(Ui::firstChild(row.check), rows_[i].checked);
    }
}

void List::attachRowEvents() {
    for (size_t i = 0; i < kMaxRows; ++i) {
        void* userData = reinterpret_cast<void*>(i);
        RowObjects row = rowObjects(i);
        Ui::onPop(row.item, List::popRow, userData);
        Ui::onPop(row.edit, List::popEdit, userData);
        Ui::onPop(row.check, List::popCheck, userData);
    }
}

List::RowObjects List::rowObjects(size_t index) const {
    switch (index) {
        case 0: return {objects.list_check_1, objects.list_item_1, objects.list_edit_1};
        case 1: return {objects.list_check_2, objects.list_item_2, objects.list_edit_2};
        case 2: return {objects.list_check_3, objects.list_item_3, objects.list_edit_3};
        case 3: return {objects.list_check_4, objects.list_item_4, objects.list_edit_4};
        case 4: return {objects.list_check_5, objects.list_item_5, objects.list_edit_5};
        case 5: return {objects.list_check_6, objects.list_item_6, objects.list_edit_6};
        default: return {};
    }
}

void List::popBack(lv_event_t* e) {
    call(instance().callbacks_.onBack, e);
}

void List::popAdd(lv_event_t* e) {
    call(instance().callbacks_.onAdd, e);
}

void List::popDelete(lv_event_t* e) {
    call(instance().callbacks_.onDelete, e);
}

void List::popNext(lv_event_t* e) {
    call(instance().callbacks_.onNext, e);
}

void List::popRow(lv_event_t* e) {
    call(instance().callbacks_.onRow, e);
}

void List::popEdit(lv_event_t* e) {
    call(instance().callbacks_.onEdit, e);
}

void List::popCheck(lv_event_t* e) {
    call(instance().callbacks_.onCheck, e);
}

void List::call(lv_event_cb_t cb, lv_event_t* e) {
    if (cb != nullptr) cb(e);
}

}  // namespace Screen
