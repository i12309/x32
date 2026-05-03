#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: Button_Style
lv_style_t *get_style_button_style_MAIN_DEFAULT();
void add_style_button_style(lv_obj_t *obj);
void remove_style_button_style(lv_obj_t *obj);

// Style: TitlePanel
lv_style_t *get_style_title_panel_MAIN_DEFAULT();
void add_style_title_panel(lv_obj_t *obj);
void remove_style_title_panel(lv_obj_t *obj);

// Style: Drop
lv_style_t *get_style_drop_MAIN_DEFAULT();
void add_style_drop(lv_obj_t *obj);
void remove_style_drop(lv_obj_t *obj);

// Style: keyButton
lv_style_t *get_style_key_button_MAIN_DEFAULT();
void add_style_key_button(lv_obj_t *obj);
void remove_style_key_button(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/