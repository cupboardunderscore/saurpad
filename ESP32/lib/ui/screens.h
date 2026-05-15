#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *startup;
    lv_obj_t *main;
    lv_obj_t *wifi;
    lv_obj_t *update;
    lv_obj_t *startup_text;
    lv_obj_t *startup_subtext;
    lv_obj_t *wifistatus;
    lv_obj_t *date;
    lv_obj_t *mute;
    lv_obj_t *clock;
    lv_obj_t *battery;
    lv_obj_t *batterystatus;
    lv_obj_t *batterycharge;
    lv_obj_t *volume;
    lv_obj_t *button_0;
    lv_obj_t *name_0;
    lv_obj_t *area_0;
    lv_obj_t *state_0;
    lv_obj_t *updated_0;
    lv_obj_t *button_1;
    lv_obj_t *name_1;
    lv_obj_t *area_1;
    lv_obj_t *state_1;
    lv_obj_t *updated_1;
    lv_obj_t *button_2;
    lv_obj_t *name_2;
    lv_obj_t *area_2;
    lv_obj_t *state_2;
    lv_obj_t *updated_2;
    lv_obj_t *button_3;
    lv_obj_t *name_3;
    lv_obj_t *area_3;
    lv_obj_t *state_3;
    lv_obj_t *updated_3;
    lv_obj_t *button_4;
    lv_obj_t *name_4;
    lv_obj_t *area_4;
    lv_obj_t *state_4;
    lv_obj_t *updated_4;
    lv_obj_t *button_5;
    lv_obj_t *name_5;
    lv_obj_t *area_5;
    lv_obj_t *state_5;
    lv_obj_t *updated_5;
    lv_obj_t *button_6;
    lv_obj_t *name_6;
    lv_obj_t *area_6;
    lv_obj_t *state_6;
    lv_obj_t *updated_6;
    lv_obj_t *button_7;
    lv_obj_t *name_7;
    lv_obj_t *area_7;
    lv_obj_t *state_7;
    lv_obj_t *updated_7;
    lv_obj_t *warn;
    lv_obj_t *warn_name;
    lv_obj_t *warn_area;
    lv_obj_t *warn_state;
    lv_obj_t *ssid;
    lv_obj_t *pass;
    lv_obj_t *wifi_qr;
    lv_obj_t *ip;
    lv_obj_t *updatebar;
    lv_obj_t *updatetext;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_STARTUP = 1,
    SCREEN_ID_MAIN = 2,
    SCREEN_ID_WIFI = 3,
    SCREEN_ID_UPDATE = 4,
};

void create_screen_startup();
void tick_screen_startup();

void create_screen_main();
void tick_screen_main();

void create_screen_wifi();
void tick_screen_wifi();

void create_screen_update();
void tick_screen_update();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/