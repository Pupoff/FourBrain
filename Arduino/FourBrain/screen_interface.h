#pragma once
#include "FourBrain.h"

void create_main_screen(void);
void create_effect_screen(int section_id);
void init_mainscreen_styles(void);

void header_create(lv_obj_t *screen);
void init_volume_styles(void);

void midi_indicator_create(lv_obj_t *header);
void midi_indicator_update(bool connected);

void volume_create(lv_obj_t *header);
void volume_set_selected(bool vol_out_selected);
void volume_update_value(int bigknob_value, bool out_selected);

lv_color_t auto_text_on(lv_color_t bg);
