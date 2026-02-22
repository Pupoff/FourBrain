#include "FourBrain.h"
#include "touchscreen_handler.h"
#include "ui_controls.h"
#include "midi_handler.h"
#include "screen_interface.h"
#include "Archetype-Henson.h"
#include "images.h"

/*Need to modify the following file:
Linux:  ~/.arduino15/packages/arduino/hardware/mbed_giga/4.4.1/libraries/Arduino_H7_Video/src/lv_conf_9.h 
Windows: C:\Users\<UserName>\AppData\Local\Arduino15\packages\arduino\hardware\mbed_giga\4.4.1\libraries\Arduino_H7_Video\src
And enable the followinf font size:
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
*/
#define TOUCHSCREEN 0  // Enable/disable Touchscreen

// ------------------------------------------------------
// MIDI callbacks
// ------------------------------------------------------
bool sysExMessageCallback(SysExMessage em) {
  return false;
}
bool sysCommonMessageCallback(SysCommonMessage cm) {
  return false;
}
bool realTimeMessageCallback(RealTimeMessage tm) {
  return false;
}


// ------------------------------------------------------
// setup
// ------------------------------------------------------
void setup() {
  Serial.begin(9600);

  GigaDisplay.begin();
#if TOUCHSCREEN
  TouchDetector.begin();
#endif
  init_images();
  initializeParameters();


  pinMode(UP_BTN, INPUT);
  pinMode(DOWN_BTN, INPUT);
  pinMode(ONOFF_BTN, INPUT_PULLUP);
  pinMode(DOUBLER_BTN, INPUT_PULLUP);
  pinMode(PARAM_BTN, INPUT_PULLUP);
  pinMode(ONFF_LED, OUTPUT);
  pinMode(DOUBLER_LED, OUTPUT);
  pinMode(ROT_SW_BTN, INPUT);
  pinMode(ROT_A, INPUT_PULLUP);
  pinMode(ROT_B, INPUT_PULLUP);

  up_btn_old = digitalRead(UP_BTN);
  down_btn_old = digitalRead(DOWN_BTN);
  rot_sw_btn_old = digitalRead(ROT_SW_BTN);
  attachInterrupt(digitalPinToInterrupt(ROT_A), updatebigknob, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROT_B), updatebigknob, CHANGE);

  screen_init = lv_obj_create(NULL);
  lv_scr_load(screen_init);
  lv_obj_set_style_bg_color(screen_init, BACKGROUND_COLOR, LV_PART_MAIN);
  lv_obj_t *img = lv_image_create(screen_init);
  lv_image_set_src(img, &startup_icon);
  lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
  lv_timer_handler();

  for (uint8_t enc = 0; enc < ENCODER_NB; enc++) {
    if (!encoders[enc].begin(SEESAW_BASE_ADDR + enc) || !encoder_pixels[enc].begin(SEESAW_BASE_ADDR + enc)) {
      //  Serial.print("Couldn't find encoder #");
      //  Serial.println(enc);
    } else {
      //  Serial.print("Found encoder + pixel #");
      //  Serial.println(enc);

      uint32_t version = ((encoders[enc].getVersion() >> 16) & 0xFFFF);
      if (version != 4991) {
        Serial.print("Wrong firmware loaded? ");
        Serial.println(version);
        while (1) delay(10);
      }

      encoders[enc].pinMode(SS_SWITCH, INPUT_PULLUP);
      delay(10);
      encoders[enc].setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
      encoders[enc].enableEncoderInterrupt();
      found_encoders++;
    }
  }

  Control_Surface.begin();
  Control_Surface.setMIDIInputCallbacks(channelMessageCallback,
                                        sysExMessageCallback,
                                        sysCommonMessageCallback,
                                        realTimeMessageCallback);
  buildMidiLookup();

  lv_obj_del(screen_init);
  create_main_screen();
  delay(1000);
}

// ------------------------------------------------------
// loop
// ------------------------------------------------------
void loop() {
  Control_Surface.loop();
  lv_timer_handler();

  digitalWrite(ONFF_LED, onoff_led);
  digitalWrite(DOUBLER_LED, doubler_led);

  // ON/OFF
  if (!digitalRead(ONOFF_BTN)) {
    if (onoff_btn_old == 1) {
      onoff_btn_old = 0;
      update_midicontrol(OTHER, OTHER_ON_OFF, !parameters[OTHER][OTHER_ON_OFF].value);
    }
  } else {
    onoff_btn_old = 1;
  }

  // DOUBLER
  if (!digitalRead(DOUBLER_BTN)) {
    if (doubler_btn_old == 1) {
      update_midicontrol(OTHER, OTHER_DOUBLER, !parameters[OTHER][OTHER_DOUBLER].value);
      doubler_btn_old = 0;
    }
  } else {
    doubler_btn_old = 1;
  }

  // PARAM PRINT
  if (!digitalRead(PARAM_BTN)) {
    if (param_btn_old == 1) {
      update_midicontrol(OTHER, OTHER_PARAM_OPEN, !parameters[OTHER][OTHER_PARAM_OPEN].value);
      param_btn_old = 0;
    }
  } else {
    param_btn_old = 1;
  }

  //SWITCH UP
  if (digitalRead(UP_BTN) != up_btn_old) {
    up_btn_old = digitalRead(UP_BTN);
    // Serial.println("BTN UP CLICKED");
    MIDIAddress note = { MIDI_Notes::C[0], Channel_3 };  //Don't know why, but control change doesn't work
    uint8_t velocity = 127;
    if (midi_connected) {
      midi.sendNoteOn(note, velocity);
      delay(10);
      midi.sendNoteOff(note, velocity);
    }
  }

  //SWITCH DOWN
  if (digitalRead(DOWN_BTN) != down_btn_old) {
    down_btn_old = digitalRead(DOWN_BTN);
    // Serial.println("DOWN_BTN CLICKED");
    MIDIAddress note = { MIDI_Notes::C[1], Channel_3 };
    uint8_t velocity = 127;
    if (midi_connected) {
      midi.sendNoteOn(note, velocity);
      delay(10);
      midi.sendNoteOff(note, velocity);
    }
  }


  // BIG KNOB SWITCH: select in/out
  if (digitalRead(ROT_SW_BTN) == 1 && digitalRead(ROT_SW_BTN) != rot_sw_btn_old) {
    rot_sw_btn_old = digitalRead(ROT_SW_BTN);
    if (is_main_screen) {
      vol_out_selected = !vol_out_selected;
      volume_set_selected(vol_out_selected);
      bigknob_PosCount = vol_out_selected ? parameters[OTHER][OTHER_OUTPUT_GAIN].value : parameters[OTHER][OTHER_INPUT_GAIN].value;
    }
  }
  if (digitalRead(ROT_SW_BTN) == 0 && digitalRead(ROT_SW_BTN) != rot_sw_btn_old) {
    rot_sw_btn_old = digitalRead(ROT_SW_BTN);
  }

  // BIG KNOB TURN
  if (bigknob_PosCount != bigknob_lastpos) {
    if (is_main_screen) {

      if (bigknob_PosCount < 0) bigknob_PosCount = 0;
      if (bigknob_PosCount > 100) bigknob_PosCount = 100;
      bigknob_lastpos = bigknob_PosCount;

      // update UI via ui_controls
      volume_update_value(bigknob_PosCount, vol_out_selected);

      //update model + send MIDI
      if (vol_out_selected) {
        parameters[OTHER][OTHER_OUTPUT_GAIN].value = bigknob_PosCount;
        update_midicontrol(OTHER, OTHER_OUTPUT_GAIN, bigknob_PosCount);
      } else {
        parameters[OTHER][OTHER_INPUT_GAIN].value = bigknob_PosCount;
        update_midicontrol(OTHER, OTHER_INPUT_GAIN, bigknob_PosCount);
      }
    } else {
      if (bigknob_PosCount < 0) {
        bigknob_PosCount = 0;
        grid_scroll_to_page(actual_effect_screen, 0);  // go up
      }
      if (bigknob_PosCount > 2) {                      //Hysteresis to remove false changes
        grid_scroll_to_page(actual_effect_screen, 1);  // go down
        bigknob_PosCount = 2;
      }
      bigknob_lastpos = bigknob_PosCount;
    }
  }

  if (volume_dirty_midi[0]) {
    volume_dirty_midi[0] = false;
    volume_update_value(parameters[OTHER][OTHER_OUTPUT_GAIN].value, 1);
  }
  if (volume_dirty_midi[1]) {
    volume_dirty_midi[1] = false;
    volume_update_value(parameters[OTHER][OTHER_INPUT_GAIN].value, 0);
  }

  // Encoders + MIDI UI updates
  for (uint8_t enc = 0; enc < found_encoders; enc++) {
    int param_to_control = enc + (grid_page * COL_NB);
    int effectscreen_encodernb_corrected = param_to_control + 1;

    // Apply MIDI-driven updates in main context
    if (knob_dirty_midi[enc]) {
      knob_dirty_midi[enc] = false;
      update_knob(knob_pending_midi[enc], enc);
    }
    if (switch_dirty_midi[enc]) {
      switch_dirty_midi[enc] = false;
      update_switch(switch_pending_midi[enc], enc);
      update_led(enc, actual_effect_screen, switch_pending_midi[enc] ? BRIGHTNESS_HIGH : BRIGHTNESS_LOW);
    }
    if (threesw_dirty_midi[enc]) {
      threesw_dirty_midi[enc] = false;
      update_threeway(enc, threesw_pending_midi[enc], colors[actual_effect_screen], true);
    }
    if (effectbutton_dirty_midi[enc]) {
      effectbutton_dirty_midi[enc] = false;

      lv_obj_remove_style(section_btn[enc], &STYLE_EFFECTBUTTON_ENABLE, LV_PART_MAIN);
      lv_obj_remove_style(section_btn[enc], &STYLE_EFFECTBUTTON_DISABLE, LV_PART_MAIN);
      lv_obj_add_style(section_btn[enc],
                       parameters[enc][0].value ? &STYLE_EFFECTBUTTON_ENABLE : &STYLE_EFFECTBUTTON_DISABLE,
                       LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_color(section_btn[enc], parameters[enc][0].value ? colors[enc] : lv_color_hex(0x888888), 0);
      lv_obj_set_style_opa(effect_icon[enc], parameters[enc][0].value ? LV_OPA_100 : LV_OPA_30, 0);  // 50% opaque
      update_led(enc, enc, parameters[enc][0].value ? BRIGHTNESS_HIGH : BRIGHTNESS_LOW);
    }
    // Encoder button: short/long press logic
    bool raw_pressed = !encoders[enc].digitalRead(SS_SWITCH);

    if (encoder_pressed[enc] != raw_pressed) {
      encoder_pressed[enc] = raw_pressed;
      if (!is_main_screen && raw_pressed) pressed_from_effect[enc] = 1;
      else if (!raw_pressed) pressed_from_effect[enc] = 0;
      else if (is_main_screen && !pressed_from_effect[enc] && raw_pressed) lv_obj_add_state(section_btn[enc], LV_STATE_CHECKED);

      if (encoder_pressed[enc]) {
        press_time[enc] = millis();
        long_fired[enc] = false;
      } else {
        if (!long_fired[enc]) {
          if (is_main_screen) {
            if (enc < SECTION_COUNT) {
              lv_obj_clear_state(section_btn[enc], LV_STATE_CHECKED);
              update_midicontrol(enc, 0, !parameters[enc][0].value);
              update_led(enc, enc, parameters[enc][0].value ? BRIGHTNESS_HIGH : BRIGHTNESS_LOW);

              lv_obj_remove_style(section_btn[enc], &STYLE_EFFECTBUTTON_ENABLE, LV_PART_MAIN);
              lv_obj_remove_style(section_btn[enc], &STYLE_EFFECTBUTTON_DISABLE, LV_PART_MAIN);
              lv_obj_add_style(section_btn[enc],
                               parameters[enc][0].value ? &STYLE_EFFECTBUTTON_ENABLE : &STYLE_EFFECTBUTTON_DISABLE,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
              lv_obj_set_style_border_color(section_btn[enc], parameters[enc][0].value ? colors[enc] : lv_color_hex(0x888888), 0);
              lv_obj_set_style_opa(effect_icon[enc], parameters[enc][0].value ? LV_OPA_100 : LV_OPA_30, 0);  // 50% opaque
            }
          } else {
            int controller_id = 0;
            if (actual_effect_screen == AMP || actual_effect_screen == EQ) controller_id = enc + (10 * parameters[AMP][8].value) + 1;
            else controller_id = param_to_control + 1;

            if (param_to_control < parameters_number[actual_effect_screen]) {
              if (parameters[actual_effect_screen][controller_id].type == SWITCH) {
                update_switch(!parameters[actual_effect_screen][controller_id].value, param_to_control);
                update_midicontrol(actual_effect_screen, effectscreen_encodernb_corrected,
                                   !parameters[actual_effect_screen][controller_id].value);
                update_led(enc, actual_effect_screen,parameters[actual_effect_screen][controller_id].value ? BRIGHTNESS_HIGH : BRIGHTNESS_LOW);
              }
            }
          }
        }
      }
    }

    if (encoder_pressed[enc] && !long_fired[enc]) {
      unsigned long duration = millis() - press_time[enc];
      if (duration >= LONG_PRESS_THRESHOLD) {
        long_fired[enc] = true;
        if (is_main_screen) {
          actual_effect_screen = enc;
          is_main_screen = 0;
          lv_obj_del(screen_main);
          create_effect_screen(enc);
        } else {
          lv_obj_del(screen_effect);
          create_main_screen();
          is_main_screen = 1;
        }
      }
    }


    // Encoder rotation
    if (!is_main_screen && param_to_control < parameters_number[actual_effect_screen] && parameters[actual_effect_screen][effectscreen_encodernb_corrected].type != SWITCH) {

      int controller_id = 0;
      if ((actual_effect_screen == AMP || actual_effect_screen == EQ) && enc != 7)
        controller_id = enc + (10 * parameters[AMP][8].value) + 1;
      else
        controller_id = param_to_control + 1;

      int old_position = parameters[actual_effect_screen][controller_id].value;
      int new_position = update_encoder(enc, actual_effect_screen, controller_id);

      if (old_position != new_position) {
        if (parameters[actual_effect_screen][controller_id].type == KNOB) {
          update_knob(new_position, param_to_control);
        } else if (parameters[actual_effect_screen][controller_id].type == THREE_WAY_SWITCH) {
          const Parameter *p = &parameters[actual_effect_screen][controller_id];
          update_threeway(param_to_control, p->value, colors[actual_effect_screen], true);
        }
        update_midicontrol(actual_effect_screen, effectscreen_encodernb_corrected, new_position);
      }
    }
  }

  delay(10);
}




static const int VISIBLE_ROWS = 2;

void grid_scroll_to_page(int section, int page) {
  if (!effectscreen_grid) return;

  // 3 rows total (example), so max page = ceil(3/2)-1 = 1
  int max_page = 1;
  if (page < 0) page = 0;
  if (page > max_page) page = max_page;

  grid_page = page;

  lv_coord_t y = page * (VISIBLE_ROWS * SQUARE_HEIGHT);
  lv_obj_scroll_to_y(effectscreen_grid, y, LV_ANIM_ON);

  for (uint8_t enc = 0; enc < found_encoders; enc++) {
    int param_to_control = enc + (grid_page * COL_NB) + 1;
    syncEncoderPosAndPixel(enc, section, param_to_control);
  }
  for (uint8_t enc = 0; enc < found_encoders; enc++) encoder_pixels[enc].show();
}
