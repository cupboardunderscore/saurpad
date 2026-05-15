#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_wifi_1;
extern const lv_img_dsc_t img_wifi_2;
extern const lv_img_dsc_t img_wifi_3;
extern const lv_img_dsc_t img_battery_empty;
extern const lv_img_dsc_t img_battery_full;
extern const lv_img_dsc_t img_battery_half;
extern const lv_img_dsc_t img_battery_quarter;
extern const lv_img_dsc_t img_battery_three_quarters;
extern const lv_img_dsc_t img_bolt;
extern const lv_img_dsc_t img_bolt_full;
extern const lv_img_dsc_t img_volume_mute;
extern const lv_img_dsc_t img_sauropod;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[12];


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/