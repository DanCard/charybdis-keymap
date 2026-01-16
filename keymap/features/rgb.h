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

// Function prototypes
const char *get_rgb_mode_name(uint8_t mode);
void handle_rgb_mode_change(uint8_t mode);
bool housekeeping_rgb_indicators(void);
