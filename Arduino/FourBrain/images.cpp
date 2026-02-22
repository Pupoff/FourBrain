#include "images.h"
#include "startup_icon.c"
#include "amp_icon.c"
#include "eq_icon.c"
#include "delay_icon.c"
#include "compressor_icon.c"
#include "overdrive_icon.c"
#include "booster_icon.c"
#include "reverb_icon.c"
#include "chorus_icon.c"


 lv_image_dsc_t startup_icon;
 lv_image_dsc_t amp_icon;
 lv_image_dsc_t eq_icon;
 lv_image_dsc_t delay_icon;
 lv_image_dsc_t compressor_icon;
 lv_image_dsc_t overdrive_icon;
 lv_image_dsc_t booster_icon;
 lv_image_dsc_t reverb_icon;
 lv_image_dsc_t chorus_icon;

const lv_image_dsc_t* section_icons[SECTION_COUNT] = {
  &amp_icon,
  &eq_icon,
  &delay_icon,
  &compressor_icon,
  &overdrive_icon,
  &booster_icon,
  &reverb_icon,
  &chorus_icon
};


// To convert an image, go there:
//https://lvgl.io/tools/imageconverter
//choose ARGB8888
//comment the structur at the end of the file and copy it here, modifying it to fit the layout.
//The screen is 800x480 so don't oversize your images
static void init_img(lv_image_dsc_t* dsc,
                     const uint8_t* map,
                     uint32_t map_size,
                     uint16_t w,
                     uint16_t h) {
  dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
  dsc->header.cf = LV_COLOR_FORMAT_ARGB8888;
  dsc->header.w = w;
  dsc->header.h = h;
  dsc->data = map;
  dsc->data_size = map_size;
}

void init_images(void) {
  init_img(&startup_icon, (const uint8_t*)startup_icon_map, sizeof(startup_icon_map), 256, 256);

  init_img(&amp_icon, (const uint8_t*)amp_icon_map, sizeof(amp_icon_map), 50, 50);
  init_img(&eq_icon, (const uint8_t*)eq_icon_map, sizeof(eq_icon_map), 50, 50);
  init_img(&delay_icon, (const uint8_t*)delay_icon_map, sizeof(delay_icon_map), 50, 50);
  init_img(&compressor_icon, (const uint8_t*)compressor_icon_map, sizeof(compressor_icon_map), 50, 50);
  init_img(&overdrive_icon, (const uint8_t*)overdrive_icon_map, sizeof(overdrive_icon_map), 50, 50);
  init_img(&booster_icon, (const uint8_t*)booster_icon_map, sizeof(booster_icon_map), 50, 50);
  init_img(&reverb_icon, (const uint8_t*)reverb_icon_map, sizeof(reverb_icon_map), 50, 50);
  init_img(&chorus_icon, (const uint8_t*)chorus_icon_map, sizeof(chorus_icon_map), 50, 50);
}