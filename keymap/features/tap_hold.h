#pragma once

#include QMK_KEYBOARD_H

#define L_POP 255

// Index enum for tap_hold array
 enum tap_hold_idx {
    TH_Q, TH_L1,
    TH_PGUP, TH_P, TH_SLSH, TH_HOME, TH_ENT_MO,
    TH_K1, TH_K2, TH_K3, TH_K4, TH_K5,
    TH_MINS, TH_K0, TH_K9, TH_K8, TH_K7, TH_K6,
    TH_ENT_TG4, TH_ENT_TG2, TH_SPC_TG1, TH_SPC_TG2, TH_SPC_TG4, TH_PMNS_TG4,
    TH_F12,
    TH_L3_TO4, TH_L3_TO2, TH_L3_TO1,
    TH_EXIT_TO3,
    TH_K0_TO0,
    TH_Q_Z,  // Q tap, Z hold
    TH_W_X,  // W tap, X hold
    TH_K7_L6, // 7 tap, Layer 6 hold
    TH_MINS_EQL, // Minus tap, Equals hold
    TH_F1_F11, // F1 tap, F11 hold
    TH_F2_F12, // F2 tap, F12 hold
    TH_COUNT  // Total count
};

// Tap/Hold Key State - Unified struct for all tap/hold keys
typedef struct {
    uint16_t timer;
    bool held;
    bool triggered;
} tap_hold_t;

// Globals
extern tap_hold_t th[TH_COUNT];

// Simple tap-hold key table: maps keycode -> (th_index, tap_keycode)
typedef struct {
    uint16_t keycode;
    uint8_t th_idx;
    uint16_t tap_key;
} simple_tap_hold_t;

// Helper macros for cleaner access
#define LONG_PRESS_TIMEOUT TAPPING_TERM
#define TH_PRESS(idx) do { th[idx].held = true; th[idx].triggered = false; th[idx].timer = timer_read(); } while(0)
#define TH_CHECK(idx) (th[idx].held && !th[idx].triggered && timer_elapsed(th[idx].timer) > LONG_PRESS_TIMEOUT)
#define TH_RELEASE_TAP(idx) (th[idx].held = false, !th[idx].triggered)
#define TH_TRIGGER(idx) (th[idx].triggered = true)

// Function prototypes
bool process_tap_hold_key(uint16_t keycode, keyrecord_t *record);
bool handle_exit_key(uint16_t tap_key, bool pressed);
bool handle_l3_thumb(uint8_t th_idx, uint8_t tap_layer, uint8_t hold_layer, bool pressed);
bool handle_thumb_toggle(uint8_t th_idx, uint16_t tap_key, const char* name, bool pressed);
void layer_history_pop(void);
void housekeeping_tap_hold(void);
