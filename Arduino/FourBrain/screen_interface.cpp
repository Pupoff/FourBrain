#include "screen_interface.h"
#include "midi_handler.h"
#include "images.h"
// MIDI dot + volume labels are internal here
static lv_obj_t *midi_dot = nullptr;
static lv_style_t STYLE_VOL_TAG;
static lv_style_t STYLE_VOL_TAG_CHECKED;
static lv_style_transition_dsc_t TRANS_VOL;
static lv_obj_t *volume_out = nullptr;
static lv_obj_t *volume_in = nullptr;

#define HEADER_HEIGHT 60

void create_main_screen(void) {
  grid_page = 0;
  screen_main = lv_obj_create(NULL);
  lv_scr_load(screen_main);
  lv_obj_set_style_bg_color(lv_scr_act(), BACKGROUND_COLOR, LV_PART_MAIN);

  for (uint8_t enc = 0; enc < found_encoders; enc++) {
    encoder_pixels[enc].setBrightness(0);
    encoder_pixels[enc].setPixelColor(0, 0, 0, 0);
  }

  header_create(screen_main);
  midi_indicator_create(header_main);  //
  volume_create(header_main);

  static lv_coord_t col_dsc[] = { SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, LV_GRID_TEMPLATE_LAST };
  static lv_coord_t row_dsc[] = { SQUARE_HEIGHT, SQUARE_HEIGHT, LV_GRID_TEMPLATE_LAST };

  lv_obj_t *mainscreen_grid = lv_obj_create(screen_main);
  lv_obj_set_grid_dsc_array(mainscreen_grid, col_dsc, row_dsc);
  lv_obj_set_size(mainscreen_grid, GigaDisplay.width(), GigaDisplay.height() - HEADER_HEIGHT);
  lv_obj_align(mainscreen_grid, LV_ALIGN_BOTTOM_MID, 0, 0);  // Position grid below header
  lv_obj_clear_flag(mainscreen_grid, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(mainscreen_grid, BACKGROUND_COLOR, 0);  // Grey background
  lv_obj_set_style_border_width(mainscreen_grid, 0, 0);             // Remove any border

  init_mainscreen_styles();

  uint32_t i;
  for (i = 0; i < SECTION_COUNT; i++) {
    uint8_t col = i % 4;
    uint8_t row = i / 4;

    section_btn[i] = lv_btn_create(mainscreen_grid);
    lv_obj_set_size(section_btn[i], SQUARE_WIDTH - 10, SQUARE_HEIGHT - 10);
    lv_obj_set_grid_cell(section_btn[i],
                         LV_GRID_ALIGN_CENTER, col, 1,
                         LV_GRID_ALIGN_CENTER, row, 1);
#if TOUCHSCREEN
    lv_obj_add_event_cb(section_btn[i], mainscreen_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)i);
#endif
    // Base card look
    lv_obj_add_style(section_btn[i], &STYLE_EFFECTBUTTON_BASE, 0);

    //Per-tile color
    lv_color_t base = colors[i];
    lv_obj_set_style_text_color(section_btn[i], base, 0);
    // Gradient variant:
    // lv_obj_set_style_border_color(section_btn[i], base, 0);
    // Subtle vertical gradient for depth
    //int ratio = 20;
    //lv_obj_set_style_bg_grad_dir(section_btn[i], LV_GRAD_DIR_VER, 0);
    //lv_obj_set_style_bg_grad_color(section_btn[i],
    //                               lv_color_mix(lv_color_white(), base, ratio), 0);
    // Auto text color (set on button so label inherits)

    //Pressed state
    lv_obj_add_style(section_btn[i], &STYLE_EFFECTBUTTON_PRESSED,
                     LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(section_btn[i], &STYLE_EFFECTBUTTON_PRESSED,
                     LV_PART_MAIN | LV_STATE_CHECKED);

    lv_color_t pressed_color = lv_color_mix(lv_color_black(), base, 60);
    lv_obj_set_style_bg_color(section_btn[i],
                              pressed_color,
                              LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_set_style_bg_color(section_btn[i],
                              pressed_color,
                              LV_PART_MAIN | LV_STATE_CHECKED);

    // Optional keyboard/encoder focus outline
    //lv_obj_set_style_outline_width(section_btn[i], 2, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    //lv_obj_set_style_outline_color(section_btn[i], lv_palette_main(LV_PALETTE_BLUE),
    //                               LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    //lv_obj_set_style_outline_pad(section_btn[i], 2, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // Label
    lv_obj_t *btn_label = lv_label_create(section_btn[i]);
    lv_obj_add_style(btn_label, &STYLE_EFFECTBUTTON_LABEL, 0);
    lv_label_set_text(btn_label, parameters[i][0].label);
    lv_obj_align(btn_label, LV_ALIGN_BOTTOM_MID, 0, -10);

    //Icon
    effect_icon[i] = lv_image_create(section_btn[i]);  // parent can be a screen or a container/grid
    lv_image_set_src(effect_icon[i], section_icons[i]);
    lv_obj_align(effect_icon[i], LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_img_recolor(effect_icon[i], lv_color_white(), 0);
    lv_obj_set_style_img_recolor_opa(effect_icon[i], LV_OPA_COVER, 0);

    //Is Param enable or disable? (effect active, amp on, etc)
    lv_obj_set_style_border_color(section_btn[i], parameters[i][0].value ? colors[i] : lv_color_hex(0x888888), 0);
    lv_obj_set_style_opa(effect_icon[i], parameters[i][0].value ? LV_OPA_100 : LV_OPA_30, 0);  // 50% opaque
    lv_obj_add_style(section_btn[i], parameters[i][0].value ? &STYLE_EFFECTBUTTON_ENABLE : &STYLE_EFFECTBUTTON_DISABLE, LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  for (uint8_t enc = 0; enc < SECTION_COUNT; enc++) {
    encoder_pixels[enc].setBrightness(parameters[enc][0].value ? 100 : 10);
    encoder_pixels[enc].setPixelColor(0, color_table[enc].r, color_table[enc].g, color_table[enc].b);
  }
  for (uint8_t enc = 0; enc < found_encoders; enc++) encoder_pixels[enc].show();
}




void create_effect_screen(int section_id) {
  grid_page = 0;
  screen_effect = lv_obj_create(NULL);
  lv_scr_load(screen_effect);
  lv_obj_set_style_bg_color(lv_scr_act(), BACKGROUND_COLOR, LV_PART_MAIN);

  for (uint8_t enc = 0; enc < found_encoders; enc++) {
    encoder_pixels[enc].setBrightness(0);
    encoder_pixels[enc].setPixelColor(0, 0, 0, 0);
  }

  // Create the header bar (this one is simplier than the main page one, so no dedicated function)
  lv_obj_t *header = lv_obj_create(screen_effect);
  lv_obj_set_size(header, GigaDisplay.width(), HEADER_HEIGHT);
  lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(header, lv_color_black(), 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_set_style_radius(header, 0, 0);

  midi_indicator_create(header);
  //volume_create(header); //No volume here for now

  // Dynamic label in the header
  lv_obj_t *header_label = lv_label_create(header);
  lv_label_set_text(header_label, parameters[section_id][0].label);  // Set text dynamically
  lv_obj_set_style_text_color(header_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(header_label, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_align(header_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(header_label, LV_ALIGN_CENTER, 0, 0);

#if TOUCHSCREEN
  // Back button in the header (yep, lots of line for a simple button, could be cleaned)
  lv_obj_t *back_btn = lv_btn_create(header);
  lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 8, 0);  // left side of the header
  lv_obj_set_height(back_btn, 40);
  lv_obj_set_style_pad_left(back_btn, 14, 0);
  lv_obj_set_style_pad_right(back_btn, 16, 0);
  lv_obj_set_style_pad_top(back_btn, 8, 0);
  lv_obj_set_style_pad_bottom(back_btn, 8, 0);
  lv_obj_set_style_radius(back_btn, 12, 0);                  // rounded corners
  lv_obj_set_style_bg_color(back_btn, lv_color_white(), 0);  // white bg
  lv_obj_set_style_bg_opa(back_btn, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(back_btn, 1, 0);
  lv_obj_set_style_border_color(back_btn, lv_color_hex(0xDDDDDD), 0);
  lv_obj_set_style_shadow_width(back_btn, 0, 0);  // flat look
    // Pressed feedback (slight grey)
  lv_obj_set_style_bg_color(back_btn, lv_palette_lighten(LV_PALETTE_GREY, 5), LV_STATE_PRESSED);

  // Label: chevron + text
  lv_obj_t *lbl = lv_label_create(back_btn);
  lv_label_set_text(lbl, LV_SYMBOL_LEFT "  Back");
  lv_obj_set_style_text_color(lbl, lv_color_black(), 0);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);
  lv_obj_center(lbl);
  lv_obj_add_event_cb(back_btn, effectscreen_event_handler, LV_EVENT_CLICKED, NULL);
#endif

  // Define grid layout
  static lv_coord_t col_dsc[] = { SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, SQUARE_WIDTH, LV_GRID_TEMPLATE_LAST };
  static lv_coord_t row_dsc[MAX_ROW + 1];  // +1 for LV_GRID_TEMPLATE_LAST

  int row_number = (parameters_number[section_id] <= 8) ? 2 : 3 + ((parameters_number[section_id] - 9) / 4);
  //i=8 row_number=2; i=9 to 12 row_number=3; i=13,16 row_number=4 etc...
  int rows = row_number;
  if (rows > MAX_ROW) rows = MAX_ROW;

  for (int r = 0; r < rows; ++r) row_dsc[r] = SQUARE_HEIGHT;
  row_dsc[rows] = LV_GRID_TEMPLATE_LAST;

  // Create the grid container
  effectscreen_grid = lv_obj_create(screen_effect);
  lv_obj_set_grid_dsc_array(effectscreen_grid, col_dsc, row_dsc);
  lv_obj_set_size(effectscreen_grid, GigaDisplay.width(), GigaDisplay.height() - HEADER_HEIGHT);
  lv_obj_align(effectscreen_grid, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_scroll_dir(effectscreen_grid, LV_DIR_VER);  // allow vertical handle only

  if (parameters_number[section_id] < 8 || (actual_effect_screen == AMP || actual_effect_screen == EQ)) lv_obj_clear_flag(effectscreen_grid, LV_OBJ_FLAG_SCROLLABLE);  // Make sure it's static
  lv_obj_set_style_bg_color(effectscreen_grid, BACKGROUND_COLOR, 0);                                                                                                   // Grey background
  //lv_obj_set_style_bg_opa(effectscreen_grid, LV_OPA_COVER); // Ensure the background is visible
  lv_obj_set_style_border_width(effectscreen_grid, 0, 0);  // Remove any border


  // Loop to create controllers.
  for (uint32_t i = 0; i < parameters_number[section_id]; i++) {

    uint8_t col = i % 4;
    uint8_t row = i / 4;

    // Create container for each controller
    lv_obj_t *controller_cont = lv_obj_create(effectscreen_grid);
    lv_obj_set_grid_cell(controller_cont, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
    lv_obj_set_style_bg_color(controller_cont, BACKGROUND_COLOR, 0);
    lv_obj_set_style_border_width(controller_cont, 0, 0);

    int param_index = 0;
    if (section_id == AMP && i != 7) {  //increment to have 3 config, except for 8th controller wich is common
      param_index = i + (10 * parameters[AMP][8].value) + 1;
    } else {
      param_index = i + 1;
    }

    switch (parameters[section_id][param_index].type) {
      case KNOB:
        {
          knobs[i] = lv_arc_create(controller_cont);
          //OLD
          // // Create the knob (black arc)
          // lv_obj_set_size(knobs[i], SQUARE_WIDTH - 40, SQUARE_HEIGHT - 40);             // Fit inside the container
          // lv_obj_set_style_arc_color(knobs[i], colors[section_id], LV_PART_INDICATOR);  // Black knob
          // lv_obj_set_style_arc_color(knobs[i], lv_color_black(), LV_PART_MAIN);         // Outer arc black
          // lv_obj_set_style_bg_color(knobs[i], lv_color_white(), LV_PART_KNOB);          // Knob center
          // lv_obj_set_style_arc_width(knobs[i], 1, LV_PART_MAIN);
          // lv_arc_set_value(knobs[i], controller_value);
          // lv_obj_center(knobs[i]);
          // ring knob: 270° sweep
          lv_arc_set_rotation(knobs[i], 135);
          lv_arc_set_bg_angles(knobs[i], 0, 270);
          lv_arc_set_value(knobs[i], parameters[section_id][param_index].value);
          lv_obj_set_size(knobs[i], SQUARE_WIDTH - 40, SQUARE_HEIGHT - 40);  // Fit inside the container

          // Track (MAIN) thin + neutral; Indicator thick + rounded
          lv_obj_set_style_arc_width(knobs[i], 10, LV_PART_MAIN);
          lv_obj_set_style_arc_color(knobs[i], lv_color_hex(0x3A3A3E), LV_PART_MAIN);
          lv_obj_set_style_arc_width(knobs[i], 12, LV_PART_INDICATOR);
          lv_obj_set_style_arc_rounded(knobs[i], true, LV_PART_INDICATOR);
          lv_obj_set_style_arc_color(knobs[i], colors[section_id], LV_PART_INDICATOR);

          // Hide center disc for a clean ring
          lv_obj_set_style_bg_opa(knobs[i], LV_OPA_TRANSP, LV_PART_KNOB);

#if TOUCHSCREEN
          lv_obj_add_event_cb(knobs[i], knob_event_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);  // Attach event callback
#endif

          // Create a label for the value below the knob
          knoblabels[i] = lv_label_create(controller_cont);
          lv_label_set_text_fmt(knoblabels[i], "%d", lv_arc_get_value(knobs[i]));  // Set initial value
          lv_obj_set_style_text_color(knoblabels[i], lv_color_white(), 0);
          lv_obj_align_to(knoblabels[i], knobs[i], LV_ALIGN_OUT_BOTTOM_MID, 0, -50);  // Below knob

          // Create label inside the knob (centered)
          lv_obj_t *label = lv_label_create(knobs[i]);
          lv_label_set_text(label, parameters[section_id][param_index].label);  // Set label dynamically
          lv_obj_set_style_text_color(label, lv_color_white(), 0);
          lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
          lv_obj_align(label, LV_ALIGN_CENTER, 0, -10);  // Center inside the knob
          break;
        }
      case SLIDER:

        // Create slider as a sibling of the grid (child of the screen)
        knobs[i] = lv_slider_create(screen_effect);
        // Size: narrow width, full height of the content area (same as the grid)
        lv_obj_set_width(knobs[i], 20);
        lv_obj_set_height(knobs[i], SQUARE_HEIGHT * 2);

        // Position: left edge, aligned to the grid’s vertical position
        lv_obj_set_x(knobs[i], (80 * i) + 70);  // left margin
        lv_obj_set_y(knobs[i], 80);             // align top with grid
        // or: lv_obj_align_to(vslider, grid, LV_ALIGN_LEFT_MID, 8, 0);
        lv_obj_move_foreground(knobs[i]);

        // --- Styling ---
        // Track (MAIN): black
        lv_obj_set_style_bg_color(knobs[i], lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(knobs[i], LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(knobs[i], 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(knobs[i], lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
        // Indicator (filled part): your per-section color
        lv_obj_set_style_bg_color(knobs[i], colors[section_id], LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(knobs[i], LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        // Knob: white center, (optional) thin border
        lv_obj_set_style_bg_color(knobs[i], lv_color_white(), LV_PART_KNOB | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(knobs[i], 1, LV_PART_KNOB | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(knobs[i], lv_color_black(), LV_PART_KNOB | LV_STATE_DEFAULT);

        // --- Range & value ---
        lv_slider_set_range(knobs[i], 0, 100);
        lv_slider_set_value(knobs[i], parameters[section_id][param_index].value, LV_ANIM_OFF);

        // Event: value changed (pass i like before)
        //  lv_obj_add_event_cb(knobs[i], knob_event_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);
        break;
      case SWITCH:
        {
          switches[i] = lv_switch_create(controller_cont);

          lv_obj_set_size(switches[i], 100, 50);  // Make switch bigger

          // // Set the indicator (active state) color
          lv_obj_center(switches[i]);

          // Track off state
          lv_obj_set_style_bg_color(switches[i], lv_color_hex(0x3A3A3C), LV_PART_MAIN);
          lv_obj_set_style_bg_opa(switches[i], LV_OPA_COVER, LV_PART_MAIN);
          lv_obj_set_style_radius(switches[i], 22, LV_PART_MAIN);

          // Track on state (indicator when checked)
          lv_obj_set_style_bg_color(switches[i], colors[section_id],
                                    LV_PART_INDICATOR | LV_STATE_CHECKED);
          lv_obj_set_style_bg_opa(switches[i], LV_OPA_COVER,
                                  LV_PART_INDICATOR | LV_STATE_CHECKED);
          lv_obj_set_style_radius(switches[i], 22, LV_PART_INDICATOR);

          // Knob (thumb)
          lv_obj_set_style_bg_color(switches[i], lv_color_white(), LV_PART_KNOB);
          lv_obj_set_style_radius(switches[i], 20, LV_PART_KNOB);
          lv_obj_set_style_shadow_width(switches[i], 10, LV_PART_KNOB);
          lv_obj_set_style_shadow_opa(switches[i], LV_OPA_20, LV_PART_KNOB);



          if (parameters[section_id][param_index].value) lv_obj_add_state(switches[i], LV_STATE_CHECKED);

          // Create label as a sibling (in 'obj', not inside the switch)
          lv_obj_t *label = lv_label_create(controller_cont);
          lv_label_set_text(label, parameters[section_id][param_index].label);  // Set label dynamically

          // Style the label
          lv_obj_set_style_text_color(label, lv_color_white(), 0);
          lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);

          // Align the label relative to the switch
          lv_obj_align_to(label, switches[i], LV_ALIGN_OUT_TOP_MID, 0, -10);


#if TOUCHSCREEN
          lv_obj_add_event_cb(switches[i], switch_event_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);  // Attach event callback
#endif

          break;
        }
      case THREE_WAY_SWITCH:
        {
          // Container for the 3 options
          threeway_cont[i] = lv_obj_create(controller_cont);
          lv_obj_set_size(threeway_cont[i], SQUARE_WIDTH - 40, SQUARE_HEIGHT - 40);
          lv_obj_center(threeway_cont[i]);
          lv_obj_align_to(threeway_cont[i], controller_cont, LV_ALIGN_TOP_MID, 0, 0);

          // Clean look (no border/background)
          lv_obj_set_style_bg_opa(threeway_cont[i], LV_OPA_TRANSP, 0);
          lv_obj_set_style_border_width(threeway_cont[i], 0, 0);
          lv_obj_set_style_pad_all(threeway_cont[i], 6, 0);

          // Vertical layout
          lv_obj_set_flex_flow(threeway_cont[i], LV_FLEX_FLOW_COLUMN);
          lv_obj_set_flex_align(threeway_cont[i],
                                LV_FLEX_ALIGN_CENTER,   // main axis (vertical) alignment
                                LV_FLEX_ALIGN_CENTER,   // cross axis alignment
                                LV_FLEX_ALIGN_CENTER);  // track alignment

          // Fetch names (fallback if missing)
          const Parameter *p = &parameters[section_id][param_index];
          const char *a = (p->optionLabels && p->optionLabels[2]) ? p->optionLabels[2] : "A";
          const char *b = (p->optionLabels && p->optionLabels[1]) ? p->optionLabels[1] : "B";
          const char *c = (p->optionLabels && p->optionLabels[0]) ? p->optionLabels[0] : "C";

          // Create the 3 labels
          threeway_labels[i][2] = lv_label_create(threeway_cont[i]);
          lv_label_set_text(threeway_labels[i][2], a);

          threeway_labels[i][1] = lv_label_create(threeway_cont[i]);
          lv_label_set_text(threeway_labels[i][1], b);

          threeway_labels[i][0] = lv_label_create(threeway_cont[i]);
          lv_label_set_text(threeway_labels[i][0], c);

          // Common styling
          for (int k = 0; k < 3; k++) {
            lv_obj_set_style_text_align(threeway_labels[i][k], LV_TEXT_ALIGN_CENTER, 0);
          }

          lv_obj_t *title = lv_label_create(controller_cont);
          lv_label_set_text(title, p->label);
          lv_obj_set_style_text_color(title, lv_color_white(), 0);
          lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
          lv_obj_align_to(title, threeway_cont[i], LV_ALIGN_OUT_TOP_MID, 0, 20);

          // Initial highlight based on current value (0..2)
          update_threeway(i, p->value, colors[section_id], 0);
        }
        break;
    }
    syncEncoderPosAndPixel(i, section_id,param_index);
  }
  for (uint8_t enc = 0; enc < found_encoders; enc++) encoder_pixels[enc].show();
}

void init_mainscreen_styles() {
  //static const lv_style_prop_t props[] = {
  // LV_STYLE_BG_COLOR, LV_STYLE_TRANSFORM_WIDTH, LV_STYLE_TRANSFORM_HEIGHT, LV_STYLE_SHADOW_WIDTH, 0
  //};
  // lv_style_transition_dsc_init(&TRANS_PRESS, props, lv_anim_path_ease_in_out, 120, 0, NULL);

  lv_style_init(&STYLE_EFFECTBUTTON_BASE);
  lv_style_set_radius(&STYLE_EFFECTBUTTON_BASE, 50);
  lv_style_set_bg_color(&STYLE_EFFECTBUTTON_BASE, lv_color_hex(0x111111));
  lv_style_set_border_width(&STYLE_EFFECTBUTTON_BASE, 8);
  lv_style_set_shadow_width(&STYLE_EFFECTBUTTON_BASE, 12);
  lv_style_set_shadow_ofs_y(&STYLE_EFFECTBUTTON_BASE, 3);
  lv_style_set_shadow_spread(&STYLE_EFFECTBUTTON_BASE, 5);
  lv_style_set_shadow_color(&STYLE_EFFECTBUTTON_BASE, lv_color_hex(0x000000));
  lv_style_set_shadow_opa(&STYLE_EFFECTBUTTON_BASE, LV_OPA_20);
  lv_style_set_pad_all(&STYLE_EFFECTBUTTON_BASE, 8);
  //lv_style_set_transition(&STYLE_EFFECTBUTTON_BASE, &TRANS_PRESS);

  lv_style_init(&STYLE_EFFECTBUTTON_LABEL);
  lv_style_set_text_font(&STYLE_EFFECTBUTTON_LABEL, &lv_font_montserrat_22);
  lv_style_set_text_letter_space(&STYLE_EFFECTBUTTON_LABEL, 1);

  //
  lv_style_init(&STYLE_EFFECTBUTTON_ENABLE);
  lv_style_set_bg_opa(&STYLE_EFFECTBUTTON_ENABLE, LV_OPA_100);
  lv_style_set_text_color(&STYLE_EFFECTBUTTON_ENABLE, lv_color_white());

  lv_style_init(&STYLE_EFFECTBUTTON_DISABLE);
  lv_style_set_bg_opa(&STYLE_EFFECTBUTTON_DISABLE, LV_OPA_30);
  lv_style_set_text_color(&STYLE_EFFECTBUTTON_DISABLE, lv_color_hex(0x222222));
  //lv_style_set_border_color(&STYLE_EFFECTBUTTON_DISABLE, lv_color_hex(0x111111));

  lv_style_init(&STYLE_EFFECTBUTTON_PRESSED);
  lv_style_set_transform_width(&STYLE_EFFECTBUTTON_PRESSED, -4);
  lv_style_set_transform_height(&STYLE_EFFECTBUTTON_PRESSED, -4);
  lv_style_set_shadow_width(&STYLE_EFFECTBUTTON_PRESSED, 6);
}

void init_volume_styles() {
  static bool inited = false;
  if (inited) return;
  inited = true;

  lv_style_init(&STYLE_VOL_TAG);
  lv_style_set_radius(&STYLE_VOL_TAG, 12);
  lv_style_set_pad_hor(&STYLE_VOL_TAG, 10);
  lv_style_set_pad_ver(&STYLE_VOL_TAG, 2);
  lv_style_set_bg_opa(&STYLE_VOL_TAG, LV_OPA_TRANSP);
  lv_style_set_border_width(&STYLE_VOL_TAG, 1);
  lv_style_set_border_color(&STYLE_VOL_TAG, lv_color_hex(0x3A3A3C));
  lv_style_set_text_color(&STYLE_VOL_TAG, lv_color_white());
  lv_style_set_transition(&STYLE_VOL_TAG, &TRANS_VOL);

  lv_style_init(&STYLE_VOL_TAG_CHECKED);
  lv_style_set_bg_opa(&STYLE_VOL_TAG_CHECKED, LV_OPA_COVER);
  lv_style_set_bg_color(&STYLE_VOL_TAG_CHECKED, lv_color_white());
  lv_style_set_text_color(&STYLE_VOL_TAG_CHECKED, lv_color_black());
  lv_style_set_shadow_width(&STYLE_VOL_TAG_CHECKED, 10);
  lv_style_set_shadow_opa(&STYLE_VOL_TAG_CHECKED, LV_OPA_20);
  lv_style_set_transform_scale(&STYLE_VOL_TAG_CHECKED, 260);
}

void header_create(lv_obj_t *screen) {
  // Create the header bar (local, not dynamically changed)
  header_main = lv_obj_create(screen);
  lv_obj_set_size(header_main, GigaDisplay.width(), HEADER_HEIGHT);
  lv_obj_align(header_main, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(header_main, lv_color_black(), 0);
  lv_obj_set_style_border_width(header_main, 0, 0);
  lv_obj_set_style_radius(header_main, 0, 0);
  lv_obj_clear_flag(header_main, LV_OBJ_FLAG_SCROLLABLE);

  // Plugin name
  lv_obj_t *plugin_name = lv_label_create(header_main);
  lv_label_set_text(plugin_name, "T.H");
  lv_obj_set_style_text_color(plugin_name, lv_color_white(), 0);
  lv_obj_set_style_text_font(plugin_name, &lv_font_montserrat_24, 0);
  lv_obj_align(plugin_name, LV_ALIGN_TOP_LEFT, 8, -2);  // top-right corner, 8px in
}

void midi_indicator_create(lv_obj_t *header) {
  midi_dot = lv_obj_create(header);
  lv_obj_add_flag(midi_dot, LV_OBJ_FLAG_FLOATING);
  lv_obj_set_size(midi_dot, 14, 14);
  lv_obj_set_style_radius(midi_dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(midi_dot, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(midi_dot, 2, 0);
  lv_obj_set_style_border_color(midi_dot, lv_color_black(), 0);
  lv_obj_align(midi_dot, LV_ALIGN_TOP_RIGHT, -8, 8);

  lv_obj_set_style_bg_color(
    midi_dot,
    midi_connected ? lv_palette_main(LV_PALETTE_GREEN) : lv_palette_main(LV_PALETTE_RED),
    0);
}

void midi_indicator_update(bool connected) {
  midi_connected = connected;
  if (!midi_dot) return;
  lv_obj_set_style_bg_color(
    midi_dot,
    midi_connected ? lv_palette_main(LV_PALETTE_GREEN) : lv_palette_main(LV_PALETTE_RED),
    0);
}

void volume_create(lv_obj_t *header) {
  init_volume_styles();

  volume_out = lv_label_create(header);
  lv_obj_add_flag(volume_out, LV_OBJ_FLAG_FLOATING);
  lv_obj_set_width(volume_out, 165);
  lv_label_set_long_mode(volume_out, LV_LABEL_LONG_CLIP);
  lv_label_set_text_fmt(volume_out, "Out: %d dB", bigknob_PosCount);
  lv_obj_set_style_text_font(volume_out, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_align(volume_out, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_add_style(volume_out, &STYLE_VOL_TAG, 0);
  lv_obj_add_style(volume_out, &STYLE_VOL_TAG_CHECKED, LV_STATE_CHECKED);
  lv_obj_align(volume_out, LV_ALIGN_TOP_LEFT, 230, -5);

  volume_in = lv_label_create(header);
  lv_obj_add_flag(volume_in, LV_OBJ_FLAG_FLOATING);
  lv_obj_set_width(volume_in, 140);
  lv_label_set_long_mode(volume_in, LV_LABEL_LONG_CLIP);
  lv_label_set_text_fmt(volume_in, "In: %d dB", bigknob_PosCount);
  lv_obj_set_style_text_font(volume_in, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_align(volume_in, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_add_style(volume_in, &STYLE_VOL_TAG, 0);
  lv_obj_add_style(volume_in, &STYLE_VOL_TAG_CHECKED, LV_STATE_CHECKED);
  lv_obj_align(volume_in, LV_ALIGN_TOP_LEFT, 70, -5);

  volume_set_selected(vol_out_selected);
}

void volume_set_selected(bool selected_out) {
  if (!volume_out || !volume_in) return;
  if (selected_out) {
    lv_obj_add_state(volume_out, LV_STATE_CHECKED);
    lv_obj_clear_state(volume_in, LV_STATE_CHECKED);
  } else {
    lv_obj_add_state(volume_in, LV_STATE_CHECKED);
    lv_obj_clear_state(volume_out, LV_STATE_CHECKED);
  }
}

void volume_update_value(int bigknob_value, bool out_selected) {
  // Convert 0..100 -> dB (comme tu fais)
  int db = (bigknob_value * 48 / 100) - 24;

  if (out_selected) {
    if (volume_out) lv_label_set_text_fmt(volume_out, "Out: %d dB", db);
  } else {
    if (volume_in) lv_label_set_text_fmt(volume_in, "In: %d dB", db);
  }
}


lv_color_t auto_text_on(lv_color_t bg) {
  uint8_t br = lv_color_brightness(bg);
  return (br > 140) ? lv_color_black() : lv_color_white();
}
