#include "touchscreen_handler.h"
#include "ui_controls.h" // update_led
#include "screen_interface.h"

void mainscreen_event_handler(lv_event_t *e) {
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  int section_id = (intptr_t)lv_event_get_user_data(e);
  actual_effect_screen = section_id;
  is_main_screen = 0;
  lv_obj_del(screen_main);
  create_effect_screen(section_id);
}

void effectscreen_event_handler(lv_event_t *e) {
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  lv_obj_del(screen_effect);
  create_main_screen();
  is_main_screen = 1;
}

void distochange_event_handler(lv_event_t *e) {
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  parameters[AMP][8].value++;
  if (parameters[AMP][8].value == 3) parameters[AMP][8].value = 0;
  update_midicontrol(OTHER, 0, parameters[AMP][8].value);

  lv_obj_del(screen_effect);
  create_effect_screen(actual_effect_screen);
}

void knob_event_cb(lv_event_t *e) {
  lv_obj_t *knob = (lv_obj_t *)lv_event_get_target(e);
  int knob_id = (intptr_t)lv_event_get_user_data(e);

  int32_t v = lv_arc_get_value(knob);
  update_midicontrol(actual_effect_screen, knob_id + 1, (int)v);

  if (knob_id < 8 && knoblabels[knob_id]) {
    lv_label_set_text_fmt(knoblabels[knob_id], "%d", (int)v);
  }
}

void switch_event_cb(lv_event_t *e) {
  lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);
  int switch_id = (intptr_t)lv_event_get_user_data(e);

  int state = lv_obj_has_state(sw, LV_STATE_CHECKED) ? 1 : 0;
  update_midicontrol(actual_effect_screen, switch_id + 1, state);

  if (switch_id < 8) {
    update_led(switch_id, actual_effect_screen, state ? BRIGHTNESS_HIGH : BRIGHTNESS_LOW);
  }
}
