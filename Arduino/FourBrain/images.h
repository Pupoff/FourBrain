#pragma once
#include "FourBrain.h"
#include "lvgl.h"

extern const lv_image_dsc_t* section_icons[SECTION_COUNT];
extern lv_image_dsc_t startup_icon;
extern lv_image_dsc_t amp_icon;
extern lv_image_dsc_t eq_icon;
extern lv_image_dsc_t delay_icon;
extern lv_image_dsc_t compressor_icon;
extern lv_image_dsc_t overdrive_icon;
extern lv_image_dsc_t booster_icon;
extern lv_image_dsc_t reverb_icon;
extern lv_image_dsc_t chorus_icon;
void init_images(void);
