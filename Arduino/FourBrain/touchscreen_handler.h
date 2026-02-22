#pragma once
#include "FourBrain.h"
#include "midi_handler.h"   // update_midicontrol
// create_main_screen/create_effect_screen déclarés dans midicortex.h

void mainscreen_event_handler(lv_event_t *e);
void effectscreen_event_handler(lv_event_t *e);
void distochange_event_handler(lv_event_t *e);
void knob_event_cb(lv_event_t *e);
void switch_event_cb(lv_event_t *e);
