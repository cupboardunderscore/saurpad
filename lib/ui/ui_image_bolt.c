#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif
#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_IMG_BOLT
#define LV_ATTRIBUTE_IMG_IMG_BOLT
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_IMG_BOLT uint8_t img_bolt_map[] = {
  0x03, 0xe0, 
  0x07, 0xe0, 
  0x0e, 0x60, 
  0x0c, 0x60, 
  0x0c, 0xe0, 
  0x1c, 0xf8, 
  0x18, 0x78, 
  0x18, 0x18, 
  0x18, 0x38, 
  0x1f, 0xb0, 
  0x0f, 0xf0, 
  0x01, 0xe0, 
  0x03, 0xe0, 
  0x03, 0xc0, 
  0x03, 0x80, 
  0x01, 0x80, 
};

const lv_img_dsc_t img_bolt = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 16,
  .header.h = 16,
  .data_size = 32,
  .data = img_bolt_map,
};
