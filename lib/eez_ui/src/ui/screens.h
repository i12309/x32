#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_LOAD0 = 1,
    SCREEN_ID_LOAD = 2,
    SCREEN_ID_MAIN = 3,
    SCREEN_ID_TASK_RUN = 4,
    SCREEN_ID_TASK_PROCESS = 5,
    SCREEN_ID_TASK = 6,
    SCREEN_ID_PROFILE = 7,
    SCREEN_ID_LIST = 8,
    SCREEN_ID_INFO = 9,
    SCREEN_ID_INPUT = 10,
    SCREEN_ID_INIT = 11,
    SCREEN_ID_WAIT = 12,
    SCREEN_ID_SERVICE = 13,
    SCREEN_ID_TABLE = 14,
    SCREEN_ID_PAPER = 15,
    SCREEN_ID_GUILLOTINE = 16,
    SCREEN_ID_SERVICE2 = 17,
    SCREEN_ID_THROWS = 18,
    SCREEN_ID_BIGEL = 19,
    SCREEN_ID_WIFI = 20,
    SCREEN_ID_KEYBOARD = 21,
    SCREEN_ID_STATS = 22,
    SCREEN_ID_UPDATE = 23,
    SCREEN_ID_CALIBRATION = 24,
    SCREEN_ID_SLICE = 25,
    SCREEN_ID_PAGE = 26,
    _SCREEN_ID_LAST = 26
};

typedef struct _objects_t {
    lv_obj_t *load0;
    lv_obj_t *load;
    lv_obj_t *main;
    lv_obj_t *task_run;
    lv_obj_t *task_process;
    lv_obj_t *task;
    lv_obj_t *profile;
    lv_obj_t *list;
    lv_obj_t *info;
    lv_obj_t *input;
    lv_obj_t *init;
    lv_obj_t *wait;
    lv_obj_t *service;
    lv_obj_t *table;
    lv_obj_t *paper;
    lv_obj_t *guillotine;
    lv_obj_t *service2;
    lv_obj_t *throws;
    lv_obj_t *bigel;
    lv_obj_t *wifi;
    lv_obj_t *keyboard;
    lv_obj_t *stats;
    lv_obj_t *update;
    lv_obj_t *calibration;
    lv_obj_t *slice;
    lv_obj_t *page;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *load_model_1;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *load_model;
    lv_obj_t *load_ma_caddress;
    lv_obj_t *load_version;
    lv_obj_t *main_task;
    lv_obj_t *main_profile;
    lv_obj_t *main_net;
    lv_obj_t *main_service;
    lv_obj_t *main_stats;
    lv_obj_t *main_support;
    lv_obj_t *obj20;
    lv_obj_t *obj21;
    lv_obj_t *task_run_back;
    lv_obj_t *task_run_title;
    lv_obj_t *obj22;
    lv_obj_t *task_run_list_task;
    lv_obj_t *task_run_list_profile;
    lv_obj_t *task_run_label;
    lv_obj_t *task_run_cycles;
    lv_obj_t *task_run_plus;
    lv_obj_t *task_run_minus;
    lv_obj_t *task_run_start;
    lv_obj_t *obj23;
    lv_obj_t *obj24;
    lv_obj_t *title_1;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *obj27;
    lv_obj_t *obj28;
    lv_obj_t *obj29;
    lv_obj_t *obj30;
    lv_obj_t *obj31;
    lv_obj_t *obj32;
    lv_obj_t *obj33;
    lv_obj_t *obj34;
    lv_obj_t *obj35;
    lv_obj_t *obj36;
    lv_obj_t *task_back;
    lv_obj_t *task_title;
    lv_obj_t *obj37;
    lv_obj_t *task_del;
    lv_obj_t *task_save;
    lv_obj_t *obj38;
    lv_obj_t *obj39;
    lv_obj_t *task_name;
    lv_obj_t *task_list_profile;
    lv_obj_t *obj40;
    lv_obj_t *task_product_mm;
    lv_obj_t *obj41;
    lv_obj_t *task_over_mm;
    lv_obj_t *obj42;
    lv_obj_t *task_first_cut_mm;
    lv_obj_t *obj43;
    lv_obj_t *task_last_cut_mm;
    lv_obj_t *obj44;
    lv_obj_t *obj45;
    lv_obj_t *profile_back;
    lv_obj_t *profile_title;
    lv_obj_t *obj46;
    lv_obj_t *profile_del;
    lv_obj_t *profile_save;
    lv_obj_t *obj47;
    lv_obj_t *obj48;
    lv_obj_t *profile_name;
    lv_obj_t *obj49;
    lv_obj_t *profile_name_1;
    lv_obj_t *obj50;
    lv_obj_t *obj51;
    lv_obj_t *obj52;
    lv_obj_t *obj53;
    lv_obj_t *obj54;
    lv_obj_t *obj55;
    lv_obj_t *list_back;
    lv_obj_t *list_title;
    lv_obj_t *obj56;
    lv_obj_t *list_del;
    lv_obj_t *list_add;
    lv_obj_t *list_next;
    lv_obj_t *list_check_1;
    lv_obj_t *list_item_1;
    lv_obj_t *list_edit_1;
    lv_obj_t *obj57;
    lv_obj_t *list_check_2;
    lv_obj_t *list_item_2;
    lv_obj_t *list_edit_2;
    lv_obj_t *obj58;
    lv_obj_t *list_check_3;
    lv_obj_t *list_item_3;
    lv_obj_t *list_edit_3;
    lv_obj_t *obj59;
    lv_obj_t *list_check_4;
    lv_obj_t *list_item_4;
    lv_obj_t *list_edit_4;
    lv_obj_t *obj60;
    lv_obj_t *list_check_5;
    lv_obj_t *list_item_5;
    lv_obj_t *list_edit_5;
    lv_obj_t *obj61;
    lv_obj_t *list_check_6;
    lv_obj_t *list_item_6;
    lv_obj_t *list_edit_6;
    lv_obj_t *obj62;
    lv_obj_t *obj63;
    lv_obj_t *obj64;
    lv_obj_t *info_back;
    lv_obj_t *info_title;
    lv_obj_t *obj65;
    lv_obj_t *info_next;
    lv_obj_t *info_field1;
    lv_obj_t *info_field2;
    lv_obj_t *info_field3;
    lv_obj_t *info_cancel;
    lv_obj_t *info_ok;
    lv_obj_t *obj66;
    lv_obj_t *obj67;
    lv_obj_t *input_title_1;
    lv_obj_t *obj68;
    lv_obj_t *input_field1;
    lv_obj_t *input_field2;
    lv_obj_t *input_field4;
    lv_obj_t *input_field3;
    lv_obj_t *input_cancel;
    lv_obj_t *input_ok;
    lv_obj_t *obj69;
    lv_obj_t *obj70;
    lv_obj_t *init_title;
    lv_obj_t *obj71;
    lv_obj_t *init_http;
    lv_obj_t *init_ok;
    lv_obj_t *obj72;
    lv_obj_t *obj73;
    lv_obj_t *init_machine;
    lv_obj_t *obj74;
    lv_obj_t *init_group;
    lv_obj_t *obj75;
    lv_obj_t *init_name;
    lv_obj_t *init_access_point;
    lv_obj_t *init_r_access_point;
    lv_obj_t *init_test;
    lv_obj_t *init_r_test;
    lv_obj_t *wait_text1;
    lv_obj_t *wait_text2;
    lv_obj_t *wait_text3;
    lv_obj_t *obj76;
    lv_obj_t *obj77;
    lv_obj_t *service_back;
    lv_obj_t *service_title;
    lv_obj_t *obj78;
    lv_obj_t *next_2;
    lv_obj_t *service_table;
    lv_obj_t *service_paper;
    lv_obj_t *service_guillotine;
    lv_obj_t *service_slice;
    lv_obj_t *service_calibration;
    lv_obj_t *service_proba;
    lv_obj_t *obj79;
    lv_obj_t *obj80;
    lv_obj_t *table_back;
    lv_obj_t *table_title;
    lv_obj_t *obj81;
    lv_obj_t *obj82;
    lv_obj_t *obj83;
    lv_obj_t *obj84;
    lv_obj_t *obj85;
    lv_obj_t *obj86;
    lv_obj_t *obj87;
    lv_obj_t *obj88;
    lv_obj_t *obj89;
    lv_obj_t *obj90;
    lv_obj_t *paper_back;
    lv_obj_t *paper_title;
    lv_obj_t *obj91;
    lv_obj_t *paper_odo;
    lv_obj_t *b_sig1_14;
    lv_obj_t *b_sig1_15;
    lv_obj_t *b_sig1_16;
    lv_obj_t *obj92;
    lv_obj_t *obj93;
    lv_obj_t *obj94;
    lv_obj_t *obj95;
    lv_obj_t *obj96;
    lv_obj_t *obj97;
    lv_obj_t *obj98;
    lv_obj_t *obj99;
    lv_obj_t *obj100;
    lv_obj_t *obj101;
    lv_obj_t *obj102;
    lv_obj_t *obj103;
    lv_obj_t *guillotine_back;
    lv_obj_t *guillotine_title;
    lv_obj_t *obj104;
    lv_obj_t *b_sig1_12;
    lv_obj_t *b_sig1_13;
    lv_obj_t *obj105;
    lv_obj_t *obj106;
    lv_obj_t *obj107;
    lv_obj_t *obj108;
    lv_obj_t *obj109;
    lv_obj_t *obj110;
    lv_obj_t *obj111;
    lv_obj_t *obj112;
    lv_obj_t *service2_back;
    lv_obj_t *service2_title;
    lv_obj_t *obj113;
    lv_obj_t *service_table_1;
    lv_obj_t *service_paper_1;
    lv_obj_t *service_guillotine_1;
    lv_obj_t *obj114;
    lv_obj_t *obj115;
    lv_obj_t *obj116;
    lv_obj_t *obj117;
    lv_obj_t *obj118;
    lv_obj_t *throws_back;
    lv_obj_t *throws_title;
    lv_obj_t *obj119;
    lv_obj_t *throws_sig;
    lv_obj_t *obj120;
    lv_obj_t *obj121;
    lv_obj_t *obj122;
    lv_obj_t *obj123;
    lv_obj_t *obj124;
    lv_obj_t *obj125;
    lv_obj_t *obj126;
    lv_obj_t *obj127;
    lv_obj_t *bigel_back;
    lv_obj_t *bigel_title;
    lv_obj_t *obj128;
    lv_obj_t *b_sig1_17;
    lv_obj_t *b_sig1_19;
    lv_obj_t *obj129;
    lv_obj_t *obj130;
    lv_obj_t *obj131;
    lv_obj_t *obj132;
    lv_obj_t *obj133;
    lv_obj_t *obj134;
    lv_obj_t *obj135;
    lv_obj_t *obj136;
    lv_obj_t *wifi_back;
    lv_obj_t *wifi_title;
    lv_obj_t *obj137;
    lv_obj_t *wifi_del;
    lv_obj_t *wifi_add;
    lv_obj_t *wifi_save;
    lv_obj_t *obj138;
    lv_obj_t *wifi_ssid_label;
    lv_obj_t *wifi_ssid;
    lv_obj_t *wifi_rssi_label;
    lv_obj_t *wifi_rssi;
    lv_obj_t *wifi_ip_label;
    lv_obj_t *wifi_ip;
    lv_obj_t *wifi_auto_connect;
    lv_obj_t *init_r_access_point_1;
    lv_obj_t *wifi_connect;
    lv_obj_t *kbd_text;
    lv_obj_t *kbd_key;
    lv_obj_t *obj139;
    lv_obj_t *obj140;
    lv_obj_t *stats_back;
    lv_obj_t *stats_title;
    lv_obj_t *obj141;
    lv_obj_t *stats_next;
    lv_obj_t *stats_field1;
    lv_obj_t *stats_field2;
    lv_obj_t *stats_param1;
    lv_obj_t *stats_value1;
    lv_obj_t *stats_param2;
    lv_obj_t *stats_value2;
    lv_obj_t *stats_param3;
    lv_obj_t *stats_value3;
    lv_obj_t *stats_param4;
    lv_obj_t *stats_value4;
    lv_obj_t *stats_param5;
    lv_obj_t *stats_value5;
    lv_obj_t *stats_param6;
    lv_obj_t *stats_value6;
    lv_obj_t *obj142;
    lv_obj_t *obj143;
    lv_obj_t *update_back;
    lv_obj_t *update_title;
    lv_obj_t *obj144;
    lv_obj_t *obj145;
    lv_obj_t *update_dev_ver;
    lv_obj_t *update_dev;
    lv_obj_t *obj146;
    lv_obj_t *update_scr_ver;
    lv_obj_t *update_scr;
    lv_obj_t *update_auto;
    lv_obj_t *update_version;
    lv_obj_t *obj147;
    lv_obj_t *obj148;
    lv_obj_t *calibration_back;
    lv_obj_t *calibration_title;
    lv_obj_t *obj149;
    lv_obj_t *calibration_save;
    lv_obj_t *obj150;
    lv_obj_t *calibration_list_profile;
    lv_obj_t *obj151;
    lv_obj_t *task_name_1;
    lv_obj_t *task_name_2;
    lv_obj_t *task_name_3;
    lv_obj_t *obj152;
    lv_obj_t *task_product_mm_1;
    lv_obj_t *obj153;
    lv_obj_t *task_over_mm_1;
    lv_obj_t *task_first_cut_mm_2;
    lv_obj_t *task_first_cut_mm_1;
    lv_obj_t *obj154;
    lv_obj_t *obj155;
    lv_obj_t *obj156;
    lv_obj_t *slice_back;
    lv_obj_t *slice_title;
    lv_obj_t *obj157;
    lv_obj_t *slice_list_profile;
    lv_obj_t *obj158;
    lv_obj_t *slice_count_paper;
    lv_obj_t *slice_plus;
    lv_obj_t *slice_minus;
    lv_obj_t *obj159;
    lv_obj_t *task_first_cut_mm_3;
    lv_obj_t *slice_go;
    lv_obj_t *obj160;
    lv_obj_t *obj161;
    lv_obj_t *obj162;
    lv_obj_t *obj163;
    lv_obj_t *obj164;
    lv_obj_t *obj165;
    lv_obj_t *obj166;
    lv_obj_t *obj167;
    lv_obj_t *obj168;
    lv_obj_t *obj169;
} objects_t;

extern objects_t objects;

void create_screen_load0();
void tick_screen_load0();

void create_screen_load();
void tick_screen_load();

void create_screen_main();
void tick_screen_main();

void create_screen_task_run();
void tick_screen_task_run();

void create_screen_task_process();
void tick_screen_task_process();

void create_screen_task();
void tick_screen_task();

void create_screen_profile();
void tick_screen_profile();

void create_screen_list();
void tick_screen_list();

void create_screen_info();
void tick_screen_info();

void create_screen_input();
void tick_screen_input();

void create_screen_init();
void tick_screen_init();

void create_screen_wait();
void tick_screen_wait();

void create_screen_service();
void tick_screen_service();

void create_screen_table();
void tick_screen_table();

void create_screen_paper();
void tick_screen_paper();

void create_screen_guillotine();
void tick_screen_guillotine();

void create_screen_service2();
void tick_screen_service2();

void create_screen_throws();
void tick_screen_throws();

void create_screen_bigel();
void tick_screen_bigel();

void create_screen_wifi();
void tick_screen_wifi();

void create_screen_keyboard();
void tick_screen_keyboard();

void create_screen_stats();
void tick_screen_stats();

void create_screen_update();
void tick_screen_update();

void create_screen_calibration();
void tick_screen_calibration();

void create_screen_slice();
void tick_screen_slice();

void create_screen_page();
void tick_screen_page();

void create_user_widget_123(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_123(int startWidgetIndex);

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/