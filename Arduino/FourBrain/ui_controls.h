#pragma once
#include "FourBrain.h"

void update_threeway(int ui_index, int sel, lv_color_t accent, bool refresh);
void update_knob(int value_knob, int number_knob);
void update_switch(int value_switch, int number_switch);
void update_led(int led_nb, int color, int brightness);

void updatebigknob(void);
int update_encoder(uint8_t enc, int section_id, int controller_id);

void midi_indicator_create(lv_obj_t *header);
void midi_indicator_update(bool connected);

void volume_create(lv_obj_t *header);
void volume_set_selected(bool vol_out_selected);
void volume_update_value(int bigknob_value, bool out_selected);

void syncEncoderPosAndPixel(int enc_nb, int section_id, int param_index );
