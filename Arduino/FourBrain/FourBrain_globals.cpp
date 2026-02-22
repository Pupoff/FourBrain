#include "FourBrain.h"

// Parameters
Parameter parameters[SECTION_COUNT + 1][MAX_PARAMS];
int parameters_number[SECTION_COUNT] = {0};

// Encoders + pixels
Adafruit_seesaw encoders[ENCODER_NB];
seesaw_NeoPixel encoder_pixels[ENCODER_NB] = {
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
};

uint8_t found_encoders = 0;

bool encoder_pressed[ENCODER_NB] = {0};
bool long_fired[ENCODER_NB] = {0};
unsigned long press_time[ENCODER_NB] = {0};

// Display / touch
Arduino_H7_Video GigaDisplay(800, 480, GigaDisplayShield);
Arduino_GigaDisplayTouch TouchDetector;

// Colors
Color color_table[] = {
  {0xFF,0x00,0x00},
  {0x10,0x30,0xFF},
  {0x00,0xFF,0x00},
  {0xFF,0xFF,0x00},
  {0xFF,0x50,0x00},
  {0xFF,0x00,0xFF},
  {0x00,0xBC,0xD4},{0xFF,0x10,0x10}
};

lv_color_t colors[9] = {
  lv_palette_main(LV_PALETTE_RED),
  lv_palette_main(LV_PALETTE_BLUE),
  lv_palette_main(LV_PALETTE_GREEN),
  lv_palette_main(LV_PALETTE_YELLOW),
  lv_palette_main(LV_PALETTE_ORANGE),
  lv_palette_main(LV_PALETTE_PURPLE),
  lv_palette_main(LV_PALETTE_CYAN),
  lv_palette_main(LV_PALETTE_PINK),
  lv_palette_main(LV_PALETTE_TEAL)
};

// LVGL objects
lv_obj_t *knobs[MAX_PARAMS] = {nullptr};
lv_obj_t *knoblabels[MAX_PARAMS] = {nullptr};
lv_obj_t *switches[MAX_PARAMS] = {nullptr};
lv_obj_t *section_btn[SECTION_COUNT] = {nullptr};

lv_obj_t* threeway_cont[MAX_PARAMS] = {nullptr};
lv_obj_t* threeway_labels[MAX_PARAMS][3] = {{nullptr}};

lv_obj_t *screen_main = nullptr;
lv_obj_t *screen_effect = nullptr;
lv_obj_t *screen_init = nullptr;
lv_obj_t *effectscreen_grid = nullptr;
lv_obj_t *effect_icon[MAX_PARAMS] = {nullptr};
lv_obj_t *header_main = nullptr;


int actual_effect_screen = 0;
int is_main_screen = 1;

// Styles
lv_style_t STYLE_EFFECTBUTTON_ENABLE;
lv_style_t STYLE_EFFECTBUTTON_DISABLE;
lv_style_t STYLE_EFFECTBUTTON_BASE;
lv_style_t STYLE_EFFECTBUTTON_LABEL;
lv_style_t STYLE_EFFECTBUTTON_PRESSED;
// MIDI
USBMIDI_Interface midi;
bool midi_connected = false;

volatile int knob_pending_midi[MAX_PARAMS] = {0};
volatile int switch_pending_midi[MAX_PARAMS] = {0};
volatile int threesw_pending_midi[MAX_PARAMS] = {0};
volatile bool knob_dirty_midi[MAX_PARAMS] = {0};
volatile bool switch_dirty_midi[MAX_PARAMS] = {0};
volatile bool threesw_dirty_midi[MAX_PARAMS] = {0};
volatile bool volume_dirty_midi[2] = {0};
volatile bool effectbutton_dirty_midi[MAX_PARAMS] = {0};
// Big knob / volume
volatile int bigknob_PosCount = 0;
int bigknob_lastpos = -1;
bool vol_out_selected = true;

//gpio

bool onoff_btn_old=1;
bool doubler_btn_old=1;
bool param_btn_old=1;

bool up_btn_old;
bool down_btn_old;
bool rot_sw_btn_old;

bool onoff_led= 0;
bool doubler_led= 0;

bool pressed_from_effect[MAX_PARAMS]={0};

int grid_page = 0;

ParamIndex midiLookup[128]; // 128 MIDI notes max