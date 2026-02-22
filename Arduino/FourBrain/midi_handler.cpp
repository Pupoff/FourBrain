#include "midi_handler.h"
#include "ui_controls.h"   // update_led, midi_indicator_update, update_threeway (dirty flags), etc.

// ------------------------------------------------------
// update_midicontrol 
// ------------------------------------------------------
void update_midicontrol(int section, int param, int value) {
  int midinote = 0;
  int param_index = 0;

  if ((section == AMP || section == EQ) && param != 0 && param != 8) {
    param_index = param + (10 * parameters[AMP][8].value);
  } else {
    param_index = param;
  }

  parameters[section][param_index].value = value;
  midinote = parameters[section][param_index].midiNote;

  if (parameters[section][param_index].type == SWITCH && value) value = 90; // in second half
  if (parameters[section][param_index].type == THREE_WAY_SWITCH) value = value * 45;

  MIDIAddress controller = { midinote, Channel_2 };
  if (midi_connected) midi.sendControlChange(controller, value * 127 / 100);
}

// ------------------------------------------------------
// findParamIndexByMidi (DEPRECATED)
// ------------------------------------------------------
// bool findParamIndexByMidi(int midiNote, size_t &outI, size_t &outJ) {
//   for (size_t i = 0; i < SECTION_COUNT + 1; ++i) {
//     for (size_t j = 0; j < MAX_PARAMS; ++j) {
//       if (parameters[i][j].midiNote == midiNote) {
//         outI = i;
//         outJ = j;
//         return true;
//       }
//     }
//   }
//   return false;
// }

// ------------------------------------------------------
// findParamIndexByMidi (New, faster)
// ------------------------------------------------------
bool findParamIndexByMidi(uint8_t midiNote,
                          size_t &outI,
                          size_t &outJ) {

  if (!midiLookup[midiNote].valid)
    return false;

  outI = midiLookup[midiNote].i;
  outJ = midiLookup[midiNote].j;
  return true;
}

//With the init:
void buildMidiLookup() {
  for (int n = 0; n < 128; n++)
    midiLookup[n].valid = false;

  for (size_t i = 0; i < SECTION_COUNT + 1; ++i) {
    for (size_t j = 0; j < MAX_PARAMS; ++j) {

      int note = parameters[i][j].midiNote;

      if (note >= 0 && note < 128) {
        midiLookup[note] = { (uint8_t)i, (uint8_t)j, true };
      }
    }
  }
}
// ------------------------------------------------------
// channelMessageCallback
// ------------------------------------------------------
bool channelMessageCallback(ChannelMessage cm) {
  if (!midi_connected) midi_indicator_update(true);

  // MIDIMessageType type = cm.getMessageType(); // optional debug
  // Serial.println(String("Cable: ") + cm.cable.getOneBased() + "header" + cm.header + "note" + cm.data1 + "value:" + cm.data2);
  // Header=0x1b=177: 1011 0001=  B0 01: Control Change on channel 2
   
  if (cm.header == 0x90) {
    // Track arming info (debug)
    // if (cm.data1 < MAX_TRACK) Serial.println(...)
  }
  else if (cm.header == 0xb1) {
    size_t si, sj;
    if (findParamIndexByMidi(cm.data1, si, sj)) {

      // --- Special volume labels only when main screen ---

      if (si == OTHER && sj == OTHER_OUTPUT_GAIN && is_main_screen) {
        parameters[si][sj].value = (cm.data2 * 100 / 127) + 1;
        if (vol_out_selected) bigknob_PosCount = parameters[OTHER][OTHER_OUTPUT_GAIN].value;
        volume_dirty_midi[0]=true;
      }
      else if (si == OTHER && sj == OTHER_INPUT_GAIN && is_main_screen) {
        parameters[si][sj].value = (cm.data2 * 100 / 127) + 1;
        if (!vol_out_selected) bigknob_PosCount = parameters[OTHER][OTHER_INPUT_GAIN].value;
        volume_dirty_midi[1]=true;
      }

      // --- SWITCH param ---
      else if (parameters[si][sj].type == SWITCH) {
        parameters[si][sj].value = (cm.data2 > 70) ? 1 : 0;

        if (si == OTHER && sj == OTHER_DOUBLER) {
          doubler_led = parameters[si][sj].value;
        } else if (si == OTHER && sj == OTHER_ON_OFF) {
          onoff_led = parameters[si][sj].value;
        }
        else if (is_main_screen && sj == 0) {
          effectbutton_dirty_midi[si]=true;

        }
        else if (sj > 0 && !is_main_screen && si == actual_effect_screen) {
          if (actual_effect_screen == AMP || actual_effect_screen == EQ) {
            if ((int)(sj / 10) == parameters[AMP][8].value) {
              int ui = (int)(sj - (parameters[AMP][8].value * 10) - 1);
              switch_pending_midi[ui] = parameters[si][sj].value;
              switch_dirty_midi[ui] = true;
            }
          } else {
            int ui = (int)(sj - 1);
            switch_pending_midi[ui] = parameters[si][sj].value;
            switch_dirty_midi[ui] = true;
          }
        }
      }

      // --- THREE_WAY_SWITCH param ---
      else if (parameters[si][sj].type == THREE_WAY_SWITCH) {
        if (cm.data2 < 40) parameters[si][sj].value = 0;
        else if (cm.data2 < 80) parameters[si][sj].value = 1;
        else parameters[si][sj].value = 2;

        if (sj > 0 && !is_main_screen && si == actual_effect_screen) {
          int ui = (int)(sj - 1);
          threesw_pending_midi[ui] = parameters[si][sj].value;
          threesw_dirty_midi[ui] = true;
        }
      }

      // --- KNOB / slider-like params ---
      else {
        parameters[si][sj].value = cm.data2 * 100 / 127;

        if (sj > 0 && !is_main_screen && si == actual_effect_screen) {
          if (actual_effect_screen == AMP || actual_effect_screen == EQ) {
            if ((int)(sj / 10) == parameters[AMP][8].value) {
              int ui = (int)(sj - (parameters[AMP][8].value * 10) - 1);
              knob_pending_midi[ui] = parameters[si][sj].value;
              knob_dirty_midi[ui] = true;
            }
          } else {
            int ui = (int)(sj - 1);
            knob_pending_midi[ui] = parameters[si][sj].value;
            knob_dirty_midi[ui] = true;
          }
        }
      }
    }
  }

  return false;
}
