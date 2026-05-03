#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: Button_Style
//

void init_style_button_style_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_font(style, &ui_font_m_24);
    lv_style_set_radius(style, 0);
    lv_style_set_shadow_width(style, 0);
};

lv_style_t *get_style_button_style_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_button_style_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_button_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_button_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_button_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_button_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: TitlePanel
//

void init_style_title_panel_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_radius(style, 0);
    lv_style_set_border_width(style, 0);
    lv_style_set_bg_color(style, lv_color_hex(0x3e424e));
};

lv_style_t *get_style_title_panel_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_title_panel_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_title_panel(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_title_panel_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_title_panel(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_title_panel_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: Drop
//

void init_style_drop_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_font(style, &ui_font_m_24);
    lv_style_set_radius(style, 3);
};

lv_style_t *get_style_drop_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_drop_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_drop(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_drop_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_drop(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_drop_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: keyButton
//

void init_style_key_button_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_radius(style, 5);
    lv_style_set_flex_grow(style, 1);
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_VER);
    lv_style_set_bg_grad_color(style, lv_color_hex(0xd0d0d0));
    lv_style_set_border_color(style, lv_color_hex(0xa2a2a2));
    lv_style_set_border_width(style, 1);
    lv_style_set_text_color(style, lv_color_hex(0x5a5d5a));
    lv_style_set_bg_color(style, lv_color_hex(0xffffff));
    lv_style_set_text_font(style, &ui_font_m_24);
};

lv_style_t *get_style_key_button_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_key_button_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_key_button(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_key_button_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_key_button(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_key_button_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_button_style,
        add_style_title_panel,
        add_style_drop,
        add_style_key_button,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_button_style,
        remove_style_title_panel,
        remove_style_drop,
        remove_style_key_button,
    };
    remove_style_funcs[styleIndex](obj);
}