#pragma once

#include QMK_KEYBOARD_H

// RGB state variables
extern bool is_flashlight;
extern bool is_day_mode;
extern uint8_t automatic_hue_tracker;
extern bool rgb_auto_cycle;
extern uint16_t rgb_auto_timer;

// Saved state for flashlight
extern uint8_t saved_rgb_mode;
extern uint8_t saved_rgb_h, saved_rgb_s, saved_rgb_v;

// LED indices for number keys 1-0 (uses global indices, subtract 29 for right side)
extern const uint8_t number_key_leds[];
// LED indices for letter keys Q-P (under numbers 1-0)
extern const uint8_t letter_key_leds[];
// Top row LEDs (left side, local: 1, 2, 3, 4, 5)
extern const uint8_t top_row_left[];
// Top row LEDs (right side, local: 6, 7, 8, 9, 0)
extern const uint8_t top_row_right[];
// Far left column LEDs (left keyboard, local: Esc, Tab, Shift, Ctrl)
extern const uint8_t far_left_col[];
// Far right column LEDs (right keyboard, local: Minus, Backslash, Quote, RShift)
extern const uint8_t far_right_col[];

// Function prototypes
const char *get_rgb_mode_name(uint8_t mode);
void handle_rgb_mode_change(uint8_t mode);
bool housekeeping_rgb_indicators(void);
