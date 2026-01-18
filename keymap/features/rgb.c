#include "rgb.h"
#include "sync.h"
#include "mouse.h"
#include "keycodes.h"
#include <lib/lib8tion/lib8tion.h>

// RGB state variables
bool is_flashlight = false;
bool is_day_mode = false;
uint8_t automatic_hue_tracker = 0;
bool rgb_auto_cycle = false;
uint16_t rgb_auto_timer = 0;

uint8_t saved_rgb_mode;
uint8_t saved_rgb_h, saved_rgb_s, saved_rgb_v;

// LED indices for number keys 1-0 (uses global indices, subtract 29 for right side)
const uint8_t number_key_leds[] = {7, 8, 15, 16, 20, 49, 45, 44, 37, 36};
// LED indices for letter keys Q-P (under numbers 1-0)
const uint8_t letter_key_leds[] = {6, 9, 14, 17, 21, 50, 46, 43, 38, 35};
// Top row LEDs (left side, local: 1, 2, 3, 4, 5)
const uint8_t top_row_left[] = {7, 8, 15, 16, 20};
// Top row LEDs (right side, local: 6, 7, 8, 9, 0)
const uint8_t top_row_right[] = {20, 16, 15, 8, 7};
// Far left column LEDs (left keyboard, local: Esc, Tab, Shift, Ctrl)
const uint8_t far_left_col[] = {0, 1, 2, 3};
// Far right column LEDs (right keyboard, local: Minus, Backslash, Quote, RShift)
const uint8_t far_right_col[] = {0, 1, 2, 3};

// Helper for logging time
#define LOG_TIME() do { \
    uint32_t _t = timer_read32(); \
    uprintf("[%lu.%03lu] ", (unsigned long)(_t / 1000), (unsigned long)(_t % 1000)); \
} while(0)

// RGB mode name lookup table
static const char* const rgb_mode_names[] PROGMEM = {
    [0] = "NONE",
    [RGB_MATRIX_SOLID_COLOR] = "SOLID_COLOR",
    [RGB_MATRIX_ALPHAS_MODS] = "ALPHA_MODS",
    [RGB_MATRIX_GRADIENT_UP_DOWN] = "GRADIENT_UP_DOWN",
    [RGB_MATRIX_GRADIENT_LEFT_RIGHT] = "GRADIENT_LEFT_RIGHT",
    [RGB_MATRIX_BREATHING] = "BREATHING",
    [RGB_MATRIX_BAND_SAT] = "COLORBAND_SAT",
    [RGB_MATRIX_BAND_VAL] = "COLORBAND_VAL",
    [RGB_MATRIX_BAND_PINWHEEL_SAT] = "COLORBAND_PINWHEEL_SAT",
    [RGB_MATRIX_BAND_PINWHEEL_VAL] = "COLORBAND_PINWHEEL_VAL",
    [RGB_MATRIX_BAND_SPIRAL_SAT] = "COLORBAND_SPIRAL_SAT",
    [RGB_MATRIX_BAND_SPIRAL_VAL] = "COLORBAND_SPIRAL_VAL",
    [RGB_MATRIX_CYCLE_ALL] = "CYCLE_ALL",
    [RGB_MATRIX_CYCLE_LEFT_RIGHT] = "CYCLE_LEFT_RIGHT",
    [RGB_MATRIX_CYCLE_UP_DOWN] = "CYCLE_UP_DOWN",
    [RGB_MATRIX_CYCLE_OUT_IN] = "CYCLE_OUT_IN",
    [RGB_MATRIX_CYCLE_OUT_IN_DUAL] = "CYCLE_OUT_IN_DUAL",
    [RGB_MATRIX_RAINBOW_MOVING_CHEVRON] = "RAINBOW_MOVING_CHEVRON",
    [RGB_MATRIX_CYCLE_PINWHEEL] = "CYCLE_PINWHEEL",
    [RGB_MATRIX_CYCLE_SPIRAL] = "CYCLE_SPIRAL",
    [RGB_MATRIX_DUAL_BEACON] = "DUAL_BEACON",
    [RGB_MATRIX_RAINBOW_BEACON] = "RAINBOW_BEACON",
    [RGB_MATRIX_RAINBOW_PINWHEELS] = "RAINBOW_PINWHEELS",
    [RGB_MATRIX_RAINDROPS] = "RAINDROPS",
    [RGB_MATRIX_JELLYBEAN_RAINDROPS] = "JELLYBEAN_RAINDROPS",
    [RGB_MATRIX_HUE_BREATHING] = "HUE_BREATHING",
    [RGB_MATRIX_HUE_PENDULUM] = "HUE_PENDULUM",
    [RGB_MATRIX_HUE_WAVE] = "HUE_WAVE",
    [RGB_MATRIX_PIXEL_FRACTAL] = "PIXEL_FRACTAL",
    [RGB_MATRIX_PIXEL_FLOW] = "PIXEL_FLOW",
    [RGB_MATRIX_PIXEL_RAIN] = "PIXEL_RAIN",
    [RGB_MATRIX_TYPING_HEATMAP] = "TYPING_HEATMAP",
    [RGB_MATRIX_DIGITAL_RAIN] = "DIGITAL_RAIN",
    [RGB_MATRIX_SOLID_REACTIVE_SIMPLE] = "SOLID_REACTIVE_SIMPLE",
    [RGB_MATRIX_SOLID_REACTIVE] = "SOLID_REACTIVE",
    [RGB_MATRIX_SOLID_REACTIVE_WIDE] = "SOLID_REACTIVE_WIDE",
    [RGB_MATRIX_SOLID_REACTIVE_CROSS] = "SOLID_REACTIVE_CROSS",
    [RGB_MATRIX_SOLID_REACTIVE_MULTICROSS] = "SOLID_REACTIVE_MULTICROSS",
    [RGB_MATRIX_SOLID_REACTIVE_NEXUS] = "SOLID_REACTIVE_NEXUS",
    [RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS] = "SOLID_REACTIVE_MULTINEXUS",
    [RGB_MATRIX_SPLASH] = "SPLASH",
    [RGB_MATRIX_MULTISPLASH] = "MULTISPLASH",
    [RGB_MATRIX_SOLID_SPLASH] = "SOLID_SPLASH",
    [RGB_MATRIX_SOLID_MULTISPLASH] = "SOLID_MULTISPLASH",
    // Custom effects
    [RGB_MATRIX_CUSTOM_fire] = "FIRE",
    [RGB_MATRIX_CUSTOM_wildfire] = "WILDFIRE",
    [RGB_MATRIX_CUSTOM_campfire] = "CAMPFIRE",
};

const char *get_rgb_mode_name(uint8_t mode) {
    if (mode < sizeof(rgb_mode_names) / sizeof(rgb_mode_names[0])) {
        return rgb_mode_names[mode];
    }
    return "UNKNOWN";
}

void handle_rgb_mode_change(uint8_t mode) {
  rgb_matrix_mode_noeeprom(mode);
  LOG_TIME();
  uprintf("\033[93mRGB Mode Changed: %d (%s)\033[0m\n", mode, get_rgb_mode_name(mode));

  if (is_keyboard_master()) {
    current_random_seed = timer_read();
    random16_set_seed(current_random_seed);

    if (mode == RGB_MATRIX_HUE_BREATHING) {
      uint8_t random_hue = timer_read() % 256;
      rgb_matrix_sethsv_noeeprom(random_hue, rgb_matrix_get_sat(),
                                 rgb_matrix_get_val());
    } else if (mode == RGB_MATRIX_SOLID_COLOR || mode == RGB_MATRIX_BREATHING) {
      automatic_hue_tracker += 42;
      rgb_matrix_sethsv_noeeprom(automatic_hue_tracker, rgb_matrix_get_sat(), rgb_matrix_get_val());
      uprintf("Solid/Breathing Activated: Mode=%d Hue=%d\n", mode, automatic_hue_tracker);
    }

    sync_needed = true;
  }
}

bool housekeeping_rgb_indicators(void) {
  if (is_flashlight) {
    rgb_matrix_set_color_all(255, 255, 255);
    return false;
  }

  // Caps Lock indicator
  if (is_caps_lock_on) {
    bool phase = (timer_read() / 300) % 2;
    bool am_i_left = is_keyboard_left();

    if (am_i_left) {
      if (phase) rgb_matrix_set_color(far_left_col[0], 255, 255, 255);
      else rgb_matrix_set_color(far_left_col[0], 0, 0, 0);
    } else {
      if (!phase) rgb_matrix_set_color(far_right_col[0], 255, 255, 255);
      else rgb_matrix_set_color(far_right_col[0], 0, 0, 0);
    }
  }

  if (is_jitter_filter_active && !is_keyboard_left()) {
    rgb_matrix_set_color(27, 0, 255, 255);
    rgb_matrix_set_color(28, 0, 255, 255);
  }

  bool is_scroll_active = charybdis_get_pointer_dragscroll_enabled();

  if (is_sniping_active) {
    if (!is_keyboard_left()) {
      for (int i = 0; i < sizeof(top_row_right) / sizeof(top_row_right[0]); i++) rgb_matrix_set_color(top_row_right[i], 0, 0, 0);
      for (int i = 0; i < sizeof(far_right_col) / sizeof(far_right_col[0]); i++) {
        HSV hsv = {(uint8_t)((i * 64) + (timer_read() / 10)), 255, 255};
        RGB rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(far_right_col[i], rgb.r, rgb.g, rgb.b);
      }
    }
    return false;
  }

  if (is_fast_mode_active) {
    if (!is_keyboard_left()) {
      for (int i = 0; i < sizeof(top_row_right) / sizeof(top_row_right[0]); i++) rgb_matrix_set_color(top_row_right[i], 0, 0, 0);
      for (int i = 0; i < sizeof(far_right_col) / sizeof(far_right_col[0]); i++) rgb_matrix_set_color(far_right_col[i], 255, 0, 0);
    }
    return false;
  }

  if (is_scroll_active) {
    if (is_keyboard_left()) {
      for (int i = 0; i < sizeof(top_row_left) / sizeof(top_row_left[0]); i++) rgb_matrix_set_color(top_row_left[i], 0, 0, 0);
      for (int i = 0; i < sizeof(far_left_col) / sizeof(far_left_col[0]); i++) {
        HSV hsv = {(uint8_t)((i * 64) + (timer_read() / 10)), 255, 255};
        RGB rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(far_left_col[i], rgb.r, rgb.g, rgb.b);
      }
    }
    return false;
  }

  uint8_t layer = get_highest_layer(layer_state);
  switch (layer) {
  case 1: {
    if (is_keyboard_left()) {
      // Keys 2, 3, 4, 5 on left (Indices: 8, 15, 16, 20)
      rgb_matrix_set_color(8, 0, 0, 255);
      rgb_matrix_set_color(15, 0, 0, 255);
      rgb_matrix_set_color(16, 0, 0, 255);
      rgb_matrix_set_color(20, 0, 0, 255);
    } else {
      // Keys 6, 7, 8, 9 on right (Indices: 20, 16, 15, 8 local)
      rgb_matrix_set_color(20, 0, 0, 255);
      rgb_matrix_set_color(16, 0, 0, 255);
      rgb_matrix_set_color(15, 0, 0, 255);
      rgb_matrix_set_color(8, 0, 0, 255);
    }
    break;
  }
  case 2: {
    if (is_keyboard_left()) {
      static const uint8_t left[] = {5, 10, 13, 18, 4, 11, 12, 19};
      for (int i = 0; i < sizeof(left) / sizeof(left[0]); i++) rgb_matrix_set_color(left[i], 0, 255, 0);
    } else {
      static const uint8_t right[] = {18, 13, 10, 5, 19, 12, 11, 4};
      for (int i = 0; i < sizeof(right) / sizeof(right[0]); i++) rgb_matrix_set_color(right[i], 0, 255, 0);
    }
    break;
  }
  case 3: {
    if (is_keyboard_left()) {
      static const uint8_t left[] = {8, 1, 9, 14, 17, 21, 2, 5, 10, 13, 18, 4, 11, 12, 19, 26, 25, 24};
      for (int i = 0; i < sizeof(left) / sizeof(left[0]); i++) rgb_matrix_set_color(left[i], 255, 255, 0);
    } else {
      static const uint8_t right[] = {21, 14, 9, 22, 19, 12, 11, 4};
      for (int i = 0; i < sizeof(right) / sizeof(right[0]); i++) rgb_matrix_set_color(right[i], 255, 255, 0);
    }
    break;
  }
  case 4: {
    if (is_keyboard_left()) {
      static const uint8_t left[] = {0, 7, 8, 15, 16, 20, 1, 6, 9, 14, 17, 21, 2, 5, 10, 13, 18, 22, 3, 4, 11, 12, 19, 23, 26, 27, 28, 25, 24};
      for (int i = 0; i < sizeof(left) / sizeof(left[0]); i++) rgb_matrix_set_color(left[i], 255, 127, 0);
    }
    break;
  }
  case 5: {
    if (is_keyboard_left()) {
      static const uint8_t left[] = {6, 9, 14, 17, 5, 10, 13, 18, 4, 11};
      for (int i = 0; i < sizeof(left) / sizeof(left[0]); i++) rgb_matrix_set_color(left[i], 255, 110, 150);
    } else {
      static const uint8_t right[] = {17, 14, 9, 18, 13, 10};
      for (int i = 0; i < sizeof(right) / sizeof(right[0]); i++) rgb_matrix_set_color(right[i], 255, 110, 150);
    }
    break;
  }
  case 6: {
    // Police Theme: Top Left and Top Right keys flash Red/Blue
    bool phase = (timer_read() / 500) % 2;
    if (is_keyboard_left()) {
      // Left Top-Left (Esc) is LED 0
      if (phase) rgb_matrix_set_color(0, 255, 0, 0); // Red
      else       rgb_matrix_set_color(0, 0, 0, 255); // Blue
    } else {
      // Right Top-Right (Minus) is LED 0
      if (phase) rgb_matrix_set_color(0, 0, 0, 255); // Blue
      else       rgb_matrix_set_color(0, 255, 0, 0); // Red
    }
    break;
  }
  }

  // Layer indicator logic: When on layer 0 (base), clear the indicator for layer 1 (LED at index 9)
  // This provides visual feedback that we're on the base layer (no layer is active)
  // For layers 1-5, clear the corresponding LED index (layer - 1)
  uint8_t indicator_idx = (layer == 0) ? 9 : (layer - 1);
  if (indicator_idx < 10) {
    uint8_t num_led = number_key_leds[indicator_idx];
    uint8_t let_led = letter_key_leds[indicator_idx];
    bool am_i_left = is_keyboard_left();

    if (am_i_left && num_led < 29) {
      rgb_matrix_set_color(num_led, 0, 0, 0);
      rgb_matrix_set_color(let_led, 0, 0, 0);
    } else if (!am_i_left && num_led >= 29) {
      rgb_matrix_set_color(num_led - 29, 0, 0, 0);
      rgb_matrix_set_color(let_led - 29, 0, 0, 0);
    }
  }

  return false;
}
