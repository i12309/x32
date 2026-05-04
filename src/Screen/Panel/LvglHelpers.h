#pragma once

#include <lvgl/lvgl.h>

namespace Ui {

inline void onPop(lv_obj_t* obj, lv_event_cb_t cb, void* userData = nullptr) {
    if (obj == nullptr || cb == nullptr) return;
    lv_obj_add_event_cb(obj, cb, LV_EVENT_RELEASED, userData);
}

inline void onPush(lv_obj_t* obj, lv_event_cb_t cb, void* userData = nullptr) {
    if (obj == nullptr || cb == nullptr) return;
    lv_obj_add_event_cb(obj, cb, LV_EVENT_PRESSED, userData);
}

inline void setText(lv_obj_t* obj, const char* text) {
    if (obj == nullptr) return;
    lv_label_set_text(obj, text ? text : "");
}

inline const char* getText(lv_obj_t* obj) {
    if (obj == nullptr) return "";
    return lv_label_get_text(obj);
}

inline void setBgColor(lv_obj_t* obj, lv_color_t color) {
    if (obj == nullptr) return;
    lv_obj_set_style_bg_color(obj, color, LV_PART_MAIN | LV_STATE_DEFAULT);
}

inline void setTextColor(lv_obj_t* obj, lv_color_t color) {
    if (obj == nullptr) return;
    lv_obj_set_style_text_color(obj, color, LV_PART_MAIN | LV_STATE_DEFAULT);
}

inline void setHidden(lv_obj_t* obj, bool hidden) {
    if (obj == nullptr) return;
    if (hidden) {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

inline lv_obj_t* firstChild(lv_obj_t* obj) {
    if (obj == nullptr || lv_obj_get_child_count(obj) == 0) return nullptr;
    return lv_obj_get_child(obj, 0);
}

inline lv_obj_t* firstLabel(lv_obj_t* obj) {
    if (obj == nullptr) return nullptr;

    const uint32_t childCount = lv_obj_get_child_count(obj);
    for (uint32_t i = 0; i < childCount; ++i) {
        lv_obj_t* child = lv_obj_get_child(obj, static_cast<int32_t>(i));
        if (child != nullptr && lv_obj_check_type(child, &lv_label_class)) return child;
    }

    return nullptr;
}

}  // namespace Ui
