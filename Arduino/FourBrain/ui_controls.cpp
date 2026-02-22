#include "ui_controls.h"
#include "midi_handler.h"
#include "screen_interface.h"

// MIDI dot + volume labels are internal here
static lv_obj_t *midi_dot = nullptr;
static lv_style_t STYLE_VOL_TAG;
static lv_style_t STYLE_VOL_TAG_CHECKED;
static lv_style_transition_dsc_t TRANS_VOL;
static lv_obj_t *volume_out = nullptr;
static lv_obj_t *volume_in  = nullptr;

volatile int lastEncoded;


void update_threeway(int ui_index, int sel, lv_color_t accent, bool refresh) {
  if (sel < 0) sel = 0;
  if (sel > 2) sel = 2;

  for (int k = 0; k < 3; k++) {
    if (!threeway_labels[ui_index][k]) continue;
    bool active = (k == sel);

    lv_obj_set_style_text_font(threeway_labels[ui_index][k],
      active ? &lv_font_montserrat_24 : &lv_font_montserrat_14, 0);

    lv_obj_set_style_text_color(threeway_labels[ui_index][k],
      active ? accent : lv_color_hex(0x9A9A9F), 0);

    lv_obj_set_style_pad_top(threeway_labels[ui_index][k], active ? 4 : 0, 0);
    lv_obj_set_style_pad_bottom(threeway_labels[ui_index][k], active ? 4 : 0, 0);
  }

  if (actual_effect_screen == AMP && !is_main_screen && ui_index == 7) {
    update_midicontrol(AMP,8 , sel);
    if (refresh){
    lv_obj_del(screen_effect);
    create_effect_screen(actual_effect_screen);
    }
  }
}

void update_knob(int value_knob, int number_knob) {
  if (knobs[number_knob]) lv_arc_set_value(knobs[number_knob], value_knob);
  if (knoblabels[number_knob]) lv_label_set_text_fmt(knoblabels[number_knob], "%d", value_knob);
}

void update_switch(int value_switch, int number_switch) {
  if (!switches[number_switch]) return;
  if (value_switch) lv_obj_add_state(switches[number_switch], LV_STATE_CHECKED);
  else lv_obj_clear_state(switches[number_switch], LV_STATE_CHECKED);
}

void update_led(int led_nb, int color, int brightness) {
  Serial.println("toto");
  encoder_pixels[led_nb].setBrightness(brightness);
  encoder_pixels[led_nb].setPixelColor(0, color_table[color].r, color_table[color].g, color_table[color].b);
  encoder_pixels[led_nb].show();
}

void updatebigknob() {
  int MSB = digitalRead(ROT_A);
  int LSB = digitalRead(ROT_B);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) bigknob_PosCount++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) bigknob_PosCount--;

  lastEncoded = encoded;
}

 int update_encoder(uint8_t enc, int section_id, int controller_id) {
  int v = parameters[section_id][controller_id].value;
  int32_t d = encoders[enc].getEncoderDelta();

  if (d != 0) {
    uint32_t now = millis();
    uint32_t dt = now - last_ms[enc];
    last_ms[enc] = now;

    int step = BASE_STEP + (K / (int)dt);
    if (step > MAX_STEP) step = MAX_STEP;
    step = (d > 0) ? step : -step;

    v += step;

    if (parameters[actual_effect_screen][controller_id].type == THREE_WAY_SWITCH) {
      if (v < 0) v = 0;
      if (v > 2) v = 2;
    } else {
      if (v < 0) v = 0;
      if (v > 100) v = 100;
    }
  }

  parameters[section_id][controller_id].value = v;
  return v;
}


void syncEncoderPosAndPixel(int enc_nb, int section_id, int param_index ){
if(enc_nb < parameters_number[section_id]){
if (enc_nb < found_encoders) {
    switch (parameters[section_id][param_index].type) {
      case KNOB:
        {
            encoders[enc_nb].setEncoderPosition(parameters[section_id][param_index].value);
            encoder_pixels[enc_nb].setBrightness(100);
            encoder_pixels[enc_nb].setPixelColor(0, color_table[section_id].r, color_table[section_id].g, color_table[section_id].b);
                  break;
                  }
      case SWITCH:
        {
            encoder_pixels[enc_nb].setBrightness(parameters[section_id][param_index].value ? 100 : 10);
            encoder_pixels[enc_nb].setPixelColor(0, color_table[section_id].r, color_table[section_id].g, color_table[section_id].b);
                    break;
        }
      case THREE_WAY_SWITCH:
        {
            encoders[enc_nb].setEncoderPosition(parameters[section_id][param_index].value);
            encoder_pixels[enc_nb].setBrightness(100);
            encoder_pixels[enc_nb].setPixelColor(0, color_table[section_id].r, color_table[section_id].g, color_table[section_id].b);
                    break;

          }
  }
}
}else{
        encoder_pixels[enc_nb].setBrightness(0);
      encoder_pixels[enc_nb].setPixelColor(0, 0, 0, 0);
}

}
