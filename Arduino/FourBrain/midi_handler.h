#pragma once
#include "FourBrain.h"
#include "ui_controls.h" // midi_indicator_update, update_led, update_knob/switch/threeway

void update_midicontrol(int section, int param, int value);
bool findParamIndexByMidi(int midiNote, size_t &outI, size_t &outJ);
bool channelMessageCallback(ChannelMessage cm);
void buildMidiLookup(void);
