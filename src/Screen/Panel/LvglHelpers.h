#pragma once

#include <Arduino.h>
#include <lvgl/lvgl.h>

namespace Ui {

inline lv_obj_t* firstLabel(lv_obj_t* obj);

inline void onPop(lv_obj_t* obj, lv_event_cb_t cb, void* userData = nullptr) {
    if (obj == nullptr || cb == nullptr) return;
    lv_obj_add_event_cb(obj, cb, LV_EVENT_RELEASED, userData);
}

inline void onPush(lv_obj_t* obj, lv_event_cb_t cb, void* userData = nullptr) {
    if (obj == nullptr || cb == nullptr) return;
    lv_obj_add_event_cb(obj, cb, LV_EVENT_PRESSED, userData);
}

inline void setText(lv_obj_t* obj, const String& text) {
    if (obj == nullptr) return;

    const char* value = text.c_str();
    if (lv_obj_check_type(obj, &lv_label_class)) {
        lv_label_set_text(obj, value);
        return;
    }

    if (lv_obj_check_type(obj, &lv_textarea_class)) {
        lv_textarea_set_text(obj, value);
        return;
    }

    if (lv_obj_check_type(obj, &lv_checkbox_class)) {
        lv_checkbox_set_text(obj, value);
        return;
    }

    lv_obj_t* label = firstLabel(obj);
    if (label != nullptr) lv_label_set_text(label, value);
}

inline void setText(lv_obj_t* obj, const char* text) {
    setText(obj, String(text ? text : ""));
}

inline String getText(lv_obj_t* obj) {
    if (obj == nullptr) return "";

    if (lv_obj_check_type(obj, &lv_label_class)) {
        return String(lv_label_get_text(obj));
    }

    if (lv_obj_check_type(obj, &lv_textarea_class)) {
        return String(lv_textarea_get_text(obj));
    }

    if (lv_obj_check_type(obj, &lv_checkbox_class)) {
        return String(lv_checkbox_get_text(obj));
    }

    lv_obj_t* label = firstLabel(obj);
    if (label != nullptr) return String(lv_label_get_text(label));

    return "";
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

inline void setChecked(lv_obj_t* obj, bool checked) {
    if (obj == nullptr) return;
    if (checked) {
        lv_obj_add_state(obj, LV_STATE_CHECKED);
    } else {
        lv_obj_remove_state(obj, LV_STATE_CHECKED);
    }
}

inline bool isChecked(lv_obj_t* obj) {
    if (obj == nullptr) return false;
    return (lv_obj_get_state(obj) & LV_STATE_CHECKED) != 0;
}

inline void dropdownSetOptions(lv_obj_t* obj, const String& options) {
    if (obj == nullptr) return;
    lv_dropdown_set_options(obj, options.c_str());
}

inline uint32_t dropdownSelected(lv_obj_t* obj) {
    if (obj == nullptr) return 0;
    return lv_dropdown_get_selected(obj);
}

inline void dropdownSetSelected(lv_obj_t* obj, uint32_t index) {
    if (obj == nullptr) return;
    lv_dropdown_set_selected(obj, index);
}

}  // namespace Ui
