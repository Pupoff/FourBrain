#pragma once
#include "Arduino_H7_Video.h"
#include "Arduino_GigaDisplayTouch.h"
#include "lvgl.h"
#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>
#include <Control_Surface.h>


// -------------------- Config --------------------
#define SECTION_COUNT 8
#define MAX_PARAMS 30
#define ENCODER_NB 8
#define MAX_ROW 4
#define COL_NB 4
#define BACKGROUND_COLOR LV_COLOR_MAKE(50, 50, 50)

#define BRIGHTNESS_HIGH 100
#define BRIGHTNESS_LOW 2


// -------------------- Types --------------------
  enum OtherParamId : uint8_t {
  OTHER_INPUT_GAIN = 0,
  OTHER_OUTPUT_GAIN = 1,
  OTHER_DOUBLER = 2,
  OTHER_PARAM_OPEN = 3,
  OTHER_ON_OFF = 4,
  OTHER_PRESET_UP = 5,
  OTHER_PRESET_DOWN = 6
};

typedef enum {
  AMP = 0, 
  EQ, 
  DELAY, 
  COMPRESSOR, 
  OVERDRIVE, 
  BOOST, 
  REVERB, 
  CHORUS, 
  OTHER
} SectionType;

typedef enum {
  KNOB = 0,
  SWITCH = 1,
  THREE_WAY_SWITCH = 2,
  SLIDER = 3
} ParameterType;

typedef struct {
  int value;
  int midiNote;
  ParameterType type;
  char label[15];
  const char* const* optionLabels; // 3 strings if THREE_WAY_SWITCH else nullptr
} Parameter;

// helper OK en header => inline
static inline const char* getThreeWayName(const Parameter* p) {
  if (!p || p->type != THREE_WAY_SWITCH || !p->optionLabels) return nullptr;
  int idx = p->value;
  if (idx < 0) idx = 0;
  if (idx > 2) idx = 2;
  return p->optionLabels[idx];
}

// -------------------- Globals (extern) --------------------
// Parameters
extern Parameter parameters[SECTION_COUNT + 1][MAX_PARAMS];
extern int parameters_number[SECTION_COUNT];

// Seesaw encoders + pixels
#define SS_SWITCH 24
#define SS_NEOPIX 6
#define SEESAW_BASE_ADDR 0x36
extern Adafruit_seesaw encoders[ENCODER_NB];
extern seesaw_NeoPixel encoder_pixels[ENCODER_NB];
extern uint8_t found_encoders;

extern bool encoder_pressed[ENCODER_NB];
extern bool long_fired[ENCODER_NB];
extern unsigned long press_time[ENCODER_NB];
#define LONG_PRESS_THRESHOLD 500   // durée en millisecondes

// tweak to test rotary behavio
static const int BASE_STEP = 1;
static const int MAX_STEP = 20;
static const int K = 180;  //Agressivity
static uint32_t last_ms[ENCODER_NB] = { 0 };

// Display/touch
extern Arduino_H7_Video GigaDisplay;
extern Arduino_GigaDisplayTouch TouchDetector;

// UI constants
#define SQUARE_HEIGHT 190
#define SQUARE_WIDTH 182
#define HEADER_HEIGHT 60
#define BACKGROUND_COLOR LV_COLOR_MAKE(50, 50, 50)

// Colors
typedef struct { uint8_t r,g,b; } Color;
extern Color color_table[];
extern lv_color_t colors[9];

// LVGL objects used cross-files
extern lv_obj_t *knobs[MAX_PARAMS];
extern lv_obj_t *knoblabels[MAX_PARAMS];
extern lv_obj_t *switches[MAX_PARAMS];
extern lv_obj_t *section_btn[SECTION_COUNT];

extern lv_obj_t* threeway_cont[MAX_PARAMS];
extern lv_obj_t* threeway_labels[MAX_PARAMS][3];

extern lv_obj_t *screen_main;
extern lv_obj_t *screen_effect;
extern lv_obj_t *screen_init;
extern lv_obj_t *effectscreen_grid;
extern lv_obj_t *effect_icon[MAX_PARAMS];
extern lv_obj_t *header_main;
extern int actual_effect_screen;
extern int is_main_screen;

// Styles used cross-files
extern lv_style_t STYLE_EFFECTBUTTON_ENABLE;
extern lv_style_t STYLE_EFFECTBUTTON_DISABLE;
extern lv_style_t STYLE_EFFECTBUTTON_BASE;
extern lv_style_t STYLE_EFFECTBUTTON_LABEL;
extern lv_style_t STYLE_EFFECTBUTTON_PRESSED;
// MIDI
extern USBMIDI_Interface midi;
extern bool midi_connected;

extern volatile int knob_pending_midi[MAX_PARAMS];
extern volatile int switch_pending_midi[MAX_PARAMS];
extern volatile int threesw_pending_midi[MAX_PARAMS];
extern volatile bool knob_dirty_midi[MAX_PARAMS];
extern volatile bool switch_dirty_midi[MAX_PARAMS];
extern volatile bool threesw_dirty_midi[MAX_PARAMS];
extern volatile bool volume_dirty_midi[2];
extern volatile bool effectbutton_dirty_midi[MAX_PARAMS];
// Big knob / volume
extern volatile int bigknob_PosCount;
extern int bigknob_lastpos;
extern bool vol_out_selected;


// GPIO
#define UP_BTN 3
#define DOWN_BTN 2

#define ROT_SW_BTN 4
#define ROT_A 5
#define ROT_B 6

#define ONOFF_BTN 13
#define DOUBLER_BTN 11
#define PARAM_BTN 12

#define ONFF_LED 9
#define DOUBLER_LED 10

extern bool onoff_btn_old;
extern bool doubler_btn_old;
extern bool param_btn_old;

extern bool up_btn_old;
extern bool down_btn_old;
extern bool rot_sw_btn_old;

// LEDs state
extern bool onoff_led;
extern bool doubler_led;

extern bool pressed_from_effect[MAX_PARAMS];

extern int grid_page;

struct ParamIndex {
  uint8_t i;
  uint8_t j;
  bool valid;
};

extern ParamIndex midiLookup[128];