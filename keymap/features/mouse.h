#pragma once

#include QMK_KEYBOARD_H

// Mouse state variables
extern bool is_sniping_active;
extern bool is_fast_mode_active;
extern bool mouse_is_locked;
extern bool is_jitter_filter_active;
extern bool is_selection_locked;
extern bool auto_mouse_on;
extern uint16_t auto_mouse_timer;
extern uint16_t auto_mouse_timeout;

// Mouse Key globals (extern from QMK core)
extern uint8_t mk_max_speed;
extern uint8_t mk_time_to_max;
extern uint8_t mk_interval;

// Function prototypes
bool handle_fast_mouse(uint16_t direction, bool pressed);
bool handle_diag_mouse(uint16_t dir1, uint16_t dir2, bool pressed);
report_mouse_t housekeeping_mouse_task(report_mouse_t mouse_report);
void process_mouse_timeouts(void);
void activate_snipe_mode(void);
void activate_fast_mode(void);
void update_mouse_button_state(uint16_t keycode, bool pressed);
