#include QMK_KEYBOARD_H
#include <stdio.h>
#include "transactions.h"
#include <lib/lib8tion/lib8tion.h>

// Timestamp logging helper - prints [seconds.milliseconds]
#define LOG_TIME() do { \
    uint32_t _t = timer_read32(); \
    uprintf("[%lu.%03lu] ", (unsigned long)(_t / 1000), (unsigned long)(_t % 1000)); \
} while(0)

// Sync statistics tracking
static uint32_t sync_success_count = 0;
static uint32_t sync_fail_count = 0;
static uint32_t last_sync_time = 0;
static uint32_t last_heartbeat_time = 0;
#define HEARTBEAT_INTERVAL 15000  // 15 seconds

static const char *layer_change_reason = NULL;
static char layer_reason_buffer[64];

// Tap Dance Definitions
typedef struct {
  bool is_press_action;
  uint8_t state;
} tap_state_t;

enum {
  SINGLE_TAP = 1,
  SINGLE_HOLD = 2,
  DOUBLE_TAP = 3,
};

enum { TD_Z_LAYER = 0 };

// Custom Keycodes
enum custom_keycodes {
  KC_RAINBOW = QK_USER_0,
  KC_REACTIVE,
  KC_MOUSE_LOCK,
  KC_ENT_L2_EXIT,
  KC_PGUP_TO0,
  KC_P_TO0,
  KC_HOME_TO0,
  KC_L_TG1,
  KC_R_TG2,
  KC_ENT_MO4,
  KC_ENT_EXIT,
  KC_SPC_EXIT,
  KC_BSPC_EXIT,
  KC_EXIT,
  KC_TURBO,
  KC_MS_FAST_UP,
  KC_MS_FAST_DOWN,
  KC_MS_FAST_LEFT,
  KC_MS_FAST_RIGHT,
  KC_MS_DIAG_UL,
  KC_MS_DIAG_UR,
  KC_MS_DIAG_DL,
  KC_MS_DIAG_DR,
  KC_SCR_MODE,
  KC_1_TG1,
  KC_2_TG2,
  KC_3_TG3,
  KC_4_TG4,
  KC_Q_TG4,
  KC_JELLY,
  KC_SPIRAL,
  KC_CHEVRON,
  KC_RGB_AUTO,
  KC_PLUS_COLON,
  KC_MINS_TO0,
  KC_0_TG1,
  KC_9_TG2,
  KC_8_TG3,
  KC_7_TO0,
  KC_6_TO0,
  KC_ENT_TG2,
  KC_ENT_TG4,
  KC_SPC_TG2,
  KC_SPC_TG4,
  KC_PMNS_TG4,
  KC_MINS_TG4,
  KC_F12_EXIT,
  KC_FIRE,
  KC_CPFR,
  KC_SNIPE,
  KC_FAST,
  KC_MS_TMO_INC,
  KC_MS_TMO_DEC,
  KC_SET_LEFT,
  KC_SET_RIGHT,
  KC_DEBUG_SYNC,  // Debug key to dump sync state
  KC_JITTER,      // Toggle Mouse Jitter Filter
  KC_P_FRAC,      // Set Pixel Fractal Theme (29)
  KC_PINWHEEL,    // Set Cycle Pinwheel Theme (18)
  KC_DAY,         // Set brightness to Day level
  KC_NIGHT,       // Set brightness to Night level
  KC_FLASHLIGHT,  // Toggle Flashlight mode
  KC_SLSH_TO0,    // Tap: /, Hold: Temporary Layer 0
  KC_5_TG5,       // Tap: 5, Hold: Toggle Layer 5 (Settings)
  KC_L3_EXT_TO4,  // Layer 3 Left Thumb: Tap Exit, Hold TO(4)
  KC_L3_EXT_TO2,  // Layer 3 Left Thumb: Tap Exit, Hold TO(2)
  KC_L3_EXT_TO1   // Layer 3 Left Thumb: Tap Exit, Hold TO(1)
};

uint8_t cur_dance(tap_dance_state_t *state) {
  if (state->count == 1) {
    if (state->interrupted || !state->pressed)
      return SINGLE_TAP;
    else
      return SINGLE_HOLD;
  } else if (state->count == 2) {
    return DOUBLE_TAP;
  } else
    return 0;
}

static tap_state_t z_tap_state = {.is_press_action = true, .state = 0};
static bool is_flashlight = false;
static bool is_sniping_active = false;
static bool is_fast_mode_active = false;
static uint16_t snipe_timer = 0;
static uint16_t fast_mode_timer = 0;
static bool mouse_is_locked = false;
static bool is_jitter_filter_active = false; // Default: Filter OFF
static uint8_t saved_rgb_mode;
static uint8_t saved_rgb_h, saved_rgb_s, saved_rgb_v;
static uint8_t automatic_hue_tracker = 0;
static bool rgb_auto_cycle = false;
static uint16_t rgb_auto_timer = 0;
static uint16_t auto_mouse_timeout = 1500; // Default 1.5 seconds, adjustable
static uint16_t auto_mouse_timer = 0;
static bool auto_mouse_on = false;
static layer_state_t layer_state_to_restore = 0;

#define MY_TAPPING_TERM 250

// =============================================================================
// Tap/Hold Key State - Unified struct for all tap/hold keys
// =============================================================================
typedef struct {
    uint16_t timer;
    bool held;
    bool triggered;
} tap_hold_t;

// Index enum for tap_hold array
enum tap_hold_idx {
    TH_Q, TH_L1,
    TH_PGUP, TH_P, TH_SLSH, TH_HOME, TH_ENT_MO,
    TH_K1, TH_K2, TH_K3, TH_K4, TH_K5,
    TH_MINS, TH_K0, TH_K9, TH_K8, TH_K7, TH_K6,
    TH_ENT_TG4, TH_ENT_TG2, TH_SPC_TG2, TH_SPC_TG4, TH_PMNS_TG4,
    TH_F12,
    TH_L3_TO4, TH_L3_TO2, TH_L3_TO1,
    TH_COUNT  // Total count
};

static tap_hold_t th[TH_COUNT] = {0};

// Helper macros for cleaner access
#define TH_PRESS(idx) do { th[idx].held = true; th[idx].triggered = false; th[idx].timer = timer_read(); } while(0)
#define TH_CHECK(idx) (th[idx].held && !th[idx].triggered && timer_elapsed(th[idx].timer) > MY_TAPPING_TERM)
#define TH_RELEASE_TAP(idx) (th[idx].held = false, !th[idx].triggered)
#define TH_TRIGGER(idx) (th[idx].triggered = true)

// =============================================================================
// Helper Functions for Duplicate Code Consolidation
// =============================================================================

// Simple tap-hold key table: maps keycode -> (th_index, tap_keycode)
typedef struct {
    uint16_t keycode;
    uint8_t th_idx;
    uint16_t tap_key;
} simple_tap_hold_t;

static const simple_tap_hold_t simple_tap_holds[] = {
    {KC_PGUP_TO0, TH_PGUP, KC_PGUP},
    {KC_HOME_TO0, TH_HOME, KC_HOME},
    {KC_MINS_TO0, TH_MINS, KC_MINS},
    {KC_0_TG1, TH_K0, KC_0},
    {KC_9_TG2, TH_K9, KC_9},
    {KC_8_TG3, TH_K8, KC_8},
    {KC_7_TO0, TH_K7, KC_7},
    {KC_6_TO0, TH_K6, KC_6},
    {KC_PMNS_TG4, TH_PMNS_TG4, KC_PMNS},
    {KC_F12_EXIT, TH_F12, KC_F12},
};
#define SIMPLE_TAP_HOLD_COUNT (sizeof(simple_tap_holds) / sizeof(simple_tap_holds[0]))

// RGB mode key table: maps keycode -> rgb_mode
typedef struct {
    uint16_t keycode;
    uint8_t rgb_mode;
} rgb_mode_key_t;

static const rgb_mode_key_t rgb_mode_keys[] = {
    {KC_RAINBOW, RGB_MATRIX_CYCLE_LEFT_RIGHT},
    {KC_REACTIVE, RGB_MATRIX_SPLASH},
    {KC_JELLY, RGB_MATRIX_JELLYBEAN_RAINDROPS},
    {KC_SPIRAL, RGB_MATRIX_CYCLE_SPIRAL},
    {KC_CHEVRON, RGB_MATRIX_RAINBOW_MOVING_CHEVRON},
    {KC_P_FRAC, RGB_MATRIX_PIXEL_FRACTAL},
    {KC_PINWHEEL, RGB_MATRIX_CYCLE_PINWHEEL},
};
#define RGB_MODE_KEY_COUNT (sizeof(rgb_mode_keys) / sizeof(rgb_mode_keys[0]))

// Fast mouse movement helper
static bool handle_fast_mouse(uint16_t direction, bool pressed) {
    if (pressed) {
        if (!is_sniping_active) tap_code(MS_ACL1);
        register_code(direction);
    } else {
        unregister_code(direction);
        if (!is_sniping_active) tap_code(MS_ACL0);
    }
    return false;
}

// Diagonal mouse movement helper
static bool handle_diag_mouse(uint16_t dir1, uint16_t dir2, bool pressed) {
    if (pressed) {
        register_code(dir1);
        register_code(dir2);
    } else {
        unregister_code(dir1);
        unregister_code(dir2);
    }
    return false;
}

// Exit key helper (SPC_EXIT, ENT_EXIT, BSPC_EXIT)
static bool handle_exit_key(uint16_t tap_key, bool pressed) {
    if (pressed) {
        layer_state_to_restore = layer_state;
        layer_change_reason = "Exit Key: L0";
        layer_state_set(0);
        rgb_matrix_indicators_user();
        th[TH_ENT_MO].timer = timer_read();
    } else {
        if (timer_elapsed(th[TH_ENT_MO].timer) < MY_TAPPING_TERM) {
            tap_code(tap_key);
        }
        rgb_matrix_indicators_user();
    }
    return false;
}

// Layer 3 thumb key helper (tap=exit, hold=switch layer)
static bool handle_l3_thumb(uint8_t th_idx, uint8_t hold_layer, bool pressed) {
    if (pressed) {
        th[th_idx].held = true;
        th[th_idx].timer = timer_read();
    } else {
        th[th_idx].held = false;
        if (timer_elapsed(th[th_idx].timer) < MY_TAPPING_TERM) {
            layer_change_reason = "L3 Thumb Tap (Exit)";
            layer_move(0);
        } else {
            layer_change_reason = "L3 Thumb Hold";
            layer_move(hold_layer);
        }
        rgb_matrix_indicators_user();
    }
    return false;
}

// Verbose thumb toggle helper (ENT_TG2, ENT_TG4, SPC_TG2, SPC_TG4)
static bool handle_thumb_toggle(uint8_t th_idx, uint16_t tap_key, const char* name, bool pressed) {
    uint32_t now = timer_read32();
    if (pressed) {
        uprintf("[%lu.%03lu] %s Pressed\n", (unsigned long)(now/1000), (unsigned long)(now%1000), name);
        TH_PRESS(th_idx);
    } else {
        uprintf("[%lu.%03lu] %s Released. Duration: %u ms. Action: %s\n",
                (unsigned long)(now/1000), (unsigned long)(now%1000), name,
                timer_elapsed(th[th_idx].timer),
                !th[th_idx].triggered ? "Tap" : "Hold handled");
        th[th_idx].held = false;
        if (!th[th_idx].triggered) tap_code(tap_key);
    }
    return false;
}

// =============================================================================

// Deferred EEPROM update to avoid USB timeout on boot
static uint16_t pending_eeprom_config = 0;
static bool eeprom_update_pending = false;
static uint32_t eeprom_defer_timer = 0;

// Mouse Key globals
extern uint8_t mk_max_speed;
extern uint8_t mk_time_to_max;
extern uint8_t mk_interval;


/*
 * LED Layout for Charybdis 4x6 (from info.json rgb_matrix.layout)
 * Each half has 29 LEDs (0-28 local addressing). LEDs snake through columns.
 *
 * LEFT HALF (global 0-28):
 *   Col0    Col1    Col2    Col3    Col4    Col5
 *   Esc=0   1=7     2=8     3=15    4=16    5=20    <- Row 0 (number row)
 *   Tab=1   Q=6     W=9     E=14    R=17    T=21    <- Row 1
 *   Sft=2   A=5     S=10    D=13    F=18    G=22    <- Row 2
 *   Ctl=3   Z=4     X=11    C=12    V=19    B=23    <- Row 3
 *                   Thumb: 24=inner, 25, 26, 27, 28=outer
 *
 * RIGHT HALF (global 29-57, use local 0-28 in code):
 *   Col0    Col1    Col2    Col3    Col4    Col5
 *   -=0     0=7     9=8     8=15    7=16    6=20    <- Row 0 (number row)
 *   \=1     P=6     O=9     I=14    U=17    Y=21    <- Row 1
 *   '=2     ;=5     L=10    K=13    J=18    H=22    <- Row 2
 *   Sft=3   /=4     .=11    ,=12    M=19    N=23    <- Row 3
 *                   Thumb: 24, 25, 26 + Trackball underglow: 27, 28
 */

// LED indices for number keys 1-0 (uses global indices, subtract 29 for right side)
static const uint8_t number_key_leds[] = {7, 8, 15, 16, 20, 49, 45, 44, 37, 36};
// LED indices for letter keys Q-P (under numbers 1-0)
static const uint8_t letter_key_leds[] = {6, 9, 14, 17, 21, 50, 46, 43, 38, 35};
// Top row LEDs (left side, local: 1, 2, 3, 4, 5)
static const uint8_t top_row_left[] = {7, 8, 15, 16, 20};
// Top row LEDs (right side, local: 6, 7, 8, 9, 0)
static const uint8_t top_row_right[] = {20, 16, 15, 8, 7};
// Far left column LEDs (left keyboard, local: Esc, Tab, Shift, Ctrl)
static const uint8_t far_left_col[] = {0, 1, 2, 3};
// Far right column LEDs (right keyboard, local: Minus, Backslash, Quote, RShift)
static const uint8_t far_right_col[] = {0, 1, 2, 3};

// Custom Split Transport Logic
typedef struct _user_sync_info_t {
  bool is_flashlight;
  bool is_sniping_active;
  bool is_fast_mode_active;
  bool mouse_is_locked;
  bool is_jitter_filter_active;
  bool is_caps_lock_on;
  uint8_t rgb_mode;
  bool is_left_hand;
  uint16_t random_seed;  // Sync random seed for effects like PIXEL_RAIN/PIXEL_FLOW
} user_sync_info_t;

static bool is_caps_lock_on = false;

typedef struct _user_sync_info_response_t {
  bool did_rgb_sync;
  uint8_t slave_rgb_mode;
  uint16_t slave_task_counter;
  bool mouse_active;
} user_sync_info_response_t;

static bool sync_needed = false;
static bool slave_first_sync = true;
static bool master_rgb_init_pending = false;
static uint8_t master_rgb_init_mode = 0;
static uint16_t current_random_seed = 0;  // Shared random seed for RGB effects
static uint16_t slave_task_counter = 0; // Running counter on slave
static bool slave_mouse_active = false; // Flag for slave to report activity

void user_sync_info_slave_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
  const user_sync_info_t *sync_data = (const user_sync_info_t *)in_data;
  user_sync_info_response_t *response = (user_sync_info_response_t *)out_data;

  // Increment slave liveness counter
  slave_task_counter++;

  is_flashlight = sync_data->is_flashlight;
  is_sniping_active = sync_data->is_sniping_active;
  is_fast_mode_active = sync_data->is_fast_mode_active;
  mouse_is_locked = sync_data->mouse_is_locked;
  is_jitter_filter_active = sync_data->is_jitter_filter_active;
  is_caps_lock_on = sync_data->is_caps_lock_on;

  // Send slave's handedness back to master via response (not used currently, just stored locally)

  bool synced = false;
  uint8_t current_mode = rgb_matrix_get_mode();
  bool mode_changed = current_mode != sync_data->rgb_mode || slave_first_sync;
  bool seed_changed = current_random_seed != sync_data->random_seed;

  // Sync random seed for effects like PIXEL_RAIN/PIXEL_FLOW
  if (seed_changed) {
    current_random_seed = sync_data->random_seed;
    random16_set_seed(current_random_seed);
  }

  if (mode_changed) {
    LOG_TIME();
    uprintf("\033[93mSlave: Syncing RGB %d -> %d%s\033[0m\n", current_mode, sync_data->rgb_mode, slave_first_sync ? " (first sync)" : "");
    rgb_matrix_mode_noeeprom(sync_data->rgb_mode);
    slave_first_sync = false;
    // Verify the mode was applied
    uint8_t new_mode = rgb_matrix_get_mode();
    if (new_mode != sync_data->rgb_mode) {
      LOG_TIME();
      uprintf("\033[91mSlave: RGB sync FAILED! Expected %d, got %d\033[0m\n", sync_data->rgb_mode, new_mode);
      sync_fail_count++;
    } else {
      sync_success_count++;
    }
    last_sync_time = timer_read32();
    synced = true;
  }

  if (out_buflen >= sizeof(user_sync_info_response_t)) {
    response->did_rgb_sync = synced;
    response->slave_rgb_mode = rgb_matrix_get_mode();
    response->slave_task_counter = slave_task_counter;
    response->mouse_active = slave_mouse_active;
    slave_mouse_active = false; // Clear flag after reporting
  }
}

// RGB mode name lookup table (index = mode enum value)
// Standard modes are contiguous from 1, custom modes handled separately
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
    // Custom effects (indices start at RGB_MATRIX_EFFECT_MAX)
    [RGB_MATRIX_CUSTOM_fire] = "FIRE",
    [RGB_MATRIX_CUSTOM_wildfire] = "WILDFIRE",
    [RGB_MATRIX_CUSTOM_campfire] = "CAMPFIRE",
};

// Helper to get mode name via lookup table
const char *get_rgb_mode_name(uint8_t mode) {
    if (mode < sizeof(rgb_mode_names) / sizeof(rgb_mode_names[0]) && rgb_mode_names[mode]) {
        return rgb_mode_names[mode];
    }
    return "UNKNOWN";
}

void handle_rgb_mode_change(uint8_t mode) {
  rgb_matrix_mode_noeeprom(mode);
  LOG_TIME();
  uprintf("\033[93mRGB Mode Changed: %d (%s)\033[0m\n", mode, get_rgb_mode_name(mode));

  // Generate new random seed for effects like PIXEL_RAIN/PIXEL_FLOW
  // This ensures both halves of split keyboard show same random pattern
  current_random_seed = timer_read();
  random16_set_seed(current_random_seed);

  // If entering Hue Breathing, pick a random base hue
  if (mode == RGB_MATRIX_HUE_BREATHING) {
    uint8_t random_hue = timer_read() % 256;
    rgb_matrix_sethsv_noeeprom(random_hue, rgb_matrix_get_sat(),
                               rgb_matrix_get_val());
  } else if (mode == RGB_MATRIX_SOLID_COLOR || mode == RGB_MATRIX_BREATHING) {
    // Increment hue for Solid Color and Breathing modes every time they activate
    automatic_hue_tracker += 42;
    rgb_matrix_sethsv_noeeprom(automatic_hue_tracker, rgb_matrix_get_sat(), rgb_matrix_get_val());
    uprintf("Solid/Breathing Activated: Mode=%d (SOLID=%d, BREATHING=%d) Hue=%d\n",
            mode, RGB_MATRIX_SOLID_COLOR, RGB_MATRIX_BREATHING, automatic_hue_tracker);
  }

  sync_needed = true;
}

// Debug function to dump full sync state
void debug_dump_sync_state(void) {
  uint32_t since_sync = last_sync_time > 0 ? timer_elapsed32(last_sync_time) : 0;

  uprintf("\n\033[95m========== SYNC STATE DUMP ==========\033[0m\n");
  LOG_TIME();
  uprintf("Role: %s\n", is_keyboard_master() ? "MASTER" : "SLAVE");
  uprintf("Hand: %s\n", is_keyboard_left() ? "LEFT" : "RIGHT");
  uprintf("\n\033[96m--- RGB State ---\033[0m\n");
  uprintf("Mode: %d (%s)\n", rgb_matrix_get_mode(), get_rgb_mode_name(rgb_matrix_get_mode()));
  uprintf("Random Seed: %u\n", current_random_seed);
  uprintf("HSV: H=%d S=%d V=%d\n", rgb_matrix_get_hue(), rgb_matrix_get_sat(), rgb_matrix_get_val());
  uprintf("\n\033[96m--- Sync Stats ---\033[0m\n");
  uprintf("Success: %lu\n", (unsigned long)sync_success_count);
  uprintf("Failures: %lu\n", (unsigned long)sync_fail_count);
  uprintf("Last Sync: %lu.%03lu sec ago\n", (unsigned long)(since_sync / 1000), (unsigned long)(since_sync % 1000));
  uprintf("\n\033[96m--- Flags ---\033[0m\n");
  uprintf("sync_needed: %s\n", sync_needed ? "YES" : "no");
  uprintf("slave_first_sync: %s\n", slave_first_sync ? "YES" : "no");
  uprintf("is_flashlight: %s\n", is_flashlight ? "YES" : "no");
  uprintf("is_sniping_active: %s\n", is_sniping_active ? "YES" : "no");
  uprintf("is_fast_mode_active: %s\n", is_fast_mode_active ? "YES" : "no");
  uprintf("mouse_is_locked: %s\n", mouse_is_locked ? "YES" : "no");
  uprintf("is_jitter_filter_active: %s\n", is_jitter_filter_active ? "YES" : "no");
  uprintf("is_caps_lock_on: %s\n", is_caps_lock_on ? "YES" : "no");
  uprintf("\033[95m======================================\033[0m\n\n");
}

void z_finished(tap_dance_state_t *state, void *user_data) {
  z_tap_state.state = cur_dance(state);
  switch (z_tap_state.state) {
  case SINGLE_TAP: {
    uint8_t layer = get_highest_layer(layer_state);
    if (layer == 1)
      tap_code(KC_P0);
    else if (layer == 2)
      tap_code(KC_HOME);
    else if (layer == 4)
      tap_code(KC_SLSH);
    else
      register_code(KC_Z);
  } break;
  case SINGLE_HOLD:
    if (get_highest_layer(layer_state) == 4) {
      layer_change_reason = "Tap Dance Z Hold";
      layer_move(0); // Peek at base layer
    } else {
      layer_change_reason = "Tap Dance Z Hold";
      layer_move(4); // Peek at layer 4
    }
    rgb_matrix_indicators_user();
    break;
  }
}

void z_reset(tap_dance_state_t *state, void *user_data) {
  switch (z_tap_state.state) {
  case SINGLE_TAP: {
    uint8_t layer = get_highest_layer(layer_state);
    if (layer != 1 && layer != 2 && layer != 4) {
      unregister_code(KC_Z);
    }
  } break;
  case SINGLE_HOLD:
    if (get_highest_layer(layer_state) == 4) {
      // Was peeking at layer 4, restore base layer
      layer_change_reason = "Z(Hold Release): Base";
      layer_move(0);
    } else {
      // Was peeking at base layer, restore layer 4
      layer_change_reason = "Z(Hold Release): L4";
      layer_move(4);
    }
    break;
  }
  z_tap_state.state = 0;
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_Z_LAYER] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, z_finished, z_reset)};

// Combo Definitions
const uint16_t PROGMEM left_combo[] = {KC_A, KC_S, COMBO_END};
const uint16_t PROGMEM up_combo[] = {KC_S, KC_D, COMBO_END};
const uint16_t PROGMEM down_combo[] = {KC_D, KC_F, COMBO_END};
const uint16_t PROGMEM af_combo[] = {KC_A, KC_F, COMBO_END};
const uint16_t PROGMEM ad_combo[] = {KC_A, KC_D, COMBO_END};
const uint16_t PROGMEM home_combo[] = {TD(TD_Z_LAYER), KC_X, COMBO_END};
const uint16_t PROGMEM pgup_combo[] = {KC_X, KC_C, COMBO_END};
const uint16_t PROGMEM pgdn_combo[] = {KC_C, KC_V, COMBO_END};
const uint16_t PROGMEM xv_combo[] = {KC_X, KC_V, COMBO_END};
const uint16_t PROGMEM zv_combo[] = {TD(TD_Z_LAYER), KC_V, COMBO_END};
const uint16_t PROGMEM caps_combo[] = {KC_LSFT, KC_RSFT, COMBO_END};

combo_t key_combos[] = {
    COMBO(left_combo, KC_LEFT),   COMBO(up_combo, KC_UP),
    COMBO(down_combo, KC_DOWN),   COMBO(af_combo, KC_RIGHT),
    COMBO(ad_combo, KC_DEL),
    COMBO(home_combo, KC_HOME),   COMBO(pgup_combo, KC_PGUP),
    COMBO(pgdn_combo, KC_PGDN),   COMBO(xv_combo, C(S(KC_V))),
    COMBO(zv_combo, KC_END),
    COMBO(caps_combo, KC_CAPS),
};

bool is_fast_mouse = false;
bool is_scroll_mode = false;

static uint32_t last_key_time = 0;

layer_state_t layer_state_set_user(layer_state_t state) {
  uint32_t now = timer_read32();
  uint32_t sec = now / 1000;
  uint32_t ms = now % 1000;

  uint8_t layer = get_highest_layer(state);
  uint8_t prev_layer = get_highest_layer(layer_state);

  uprintf("[%lu.%03lu] Layer change: %u -> %u (state=%lu) Reason: %s\n", sec, ms,
          prev_layer, layer, (unsigned long)state,
          layer_change_reason ? layer_change_reason : "Standard Keycode / Core");
  layer_change_reason = NULL;

  // Force RGB refresh when returning to layer 0 to clear stale indicator colors
  if (layer == 0 && prev_layer != 0) {
    handle_rgb_mode_change(rgb_matrix_get_mode());
  }
  if (layer == 3) {
    uprintf("[%lu.%03lu] Entering Layer 3. CPI: %u\n", sec, ms,
            pointing_device_get_cpi());
  }
  return state;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  uint32_t now = timer_read32();
  uint32_t sec = now / 1000;
  uint32_t ms = now % 1000;

  if (record->event.pressed) {
    // Shift cancels Caps Lock
    if (host_keyboard_led_state().caps_lock && (keycode == KC_LSFT || keycode == KC_RSFT)) {
      tap_code(KC_CAPS);
      return false;
    }

    uint32_t diff = now - last_key_time;
    last_key_time = now;

    switch (keycode) {
    case KC_A:
      uprintf("[%lu.%03lu] A (diff %lu)\n", sec, ms, diff);
      break;
    case KC_S:
      uprintf("[%lu.%03lu] S (diff %lu)\n", sec, ms, diff);
      break;
    case KC_D:
      uprintf("[%lu.%03lu] D (diff %lu)\n", sec, ms, diff);
      break;
    case KC_F:
      uprintf("[%lu.%03lu] F (diff %lu)\n", sec, ms, diff);
      break;
    case KC_G:
      uprintf("[%lu.%03lu] G (diff %lu)\n", sec, ms, diff);
      break;
    case KC_J:
      uprintf("[%lu.%03lu] J (diff %lu) L:%u\n", sec, ms, diff,
              get_highest_layer(layer_state));
      break;

    // Combo Keys
    case TD(TD_Z_LAYER):
      uprintf("[%lu.%03lu] Z(TD) (diff %lu)\n", sec, ms, diff);
      break;
    case KC_C:
      uprintf("[%lu.%03lu] C (diff %lu)\n", sec, ms, diff);
      break;
    case KC_V:
      uprintf("[%lu.%03lu] V (diff %lu)\n", sec, ms, diff);
      break;
    case KC_B:
      uprintf("[%lu.%03lu] B (diff %lu)\n", sec, ms, diff);
      break;

    // Thumb Toggles (Layer 0)
    case KC_ENT_TG2:
      uprintf("[%lu.%03lu] ENT_TG2 (diff %lu) L:%u\n", sec, ms, diff,
              get_highest_layer(layer_state));
      break;
    case KC_ENT_TG4:
      uprintf("[%lu.%03lu] ENT_TG4 (diff %lu) L:%u\n", sec, ms, diff,
              get_highest_layer(layer_state));
      break;
    case KC_SPC_TG2:
      uprintf("[%lu.%03lu] SPC_TG2 (diff %lu) L:%u\n", sec, ms, diff,
              get_highest_layer(layer_state));
      break;

    // Thumb Exits (Layer 1/2/3)
    case KC_ENT_EXIT:
      uprintf("[%lu.%03lu] ENT_EXIT (diff %lu) L:%u\n", sec, ms, diff,
              get_highest_layer(layer_state));
      break;
    case KC_SPC_EXIT:
      uprintf("[%lu.%03lu] SPC_EXIT (diff %lu) L:%u\n", sec, ms, diff,
              get_highest_layer(layer_state));
      break;

    // Standard Keys (Layer 4 etc)
    case KC_ENT:
      uprintf("[%lu.%03lu] ENT (diff %lu) L:%u\n", sec, ms, diff,
              get_highest_layer(layer_state));
      break;
    case KC_SPC:
      uprintf("[%lu.%03lu] SPC (diff %lu) L:%u\n", sec, ms, diff,
              get_highest_layer(layer_state));
      break;

    // Resulting Actions
    case KC_LEFT:
      uprintf("[%lu.%03lu] LEFT (diff %lu)\n", sec, ms, diff);
      break;
    case KC_UP:
      uprintf("[%lu.%03lu] UP (diff %lu)\n", sec, ms, diff);
      break;
    case KC_DOWN:
      uprintf("[%lu.%03lu] DOWN (diff %lu)\n", sec, ms, diff);
      break;
    case KC_RIGHT:
      uprintf("[%lu.%03lu] RIGHT (diff %lu)\n", sec, ms, diff);
      break;
    case KC_DEL:
      uprintf("[%lu.%03lu] DEL (diff %lu)\n", sec, ms, diff);
      break;
    case KC_HOME:
      uprintf("[%lu.%03lu] HOME (diff %lu)\n", sec, ms, diff);
      break;
    case KC_PGUP:
      uprintf("[%lu.%03lu] PGUP (diff %lu)\n", sec, ms, diff);
      break;
    case KC_PGDN:
      uprintf("[%lu.%03lu] PGDN (diff %lu)\n", sec, ms, diff);
      break;
    case KC_END:
      uprintf("[%lu.%03lu] END (diff %lu)\n", sec, ms, diff);
      break;
    }
  }

  if (is_scroll_mode && record->event.pressed) {
    switch (keycode) {
    case KC_MS_FAST_UP:
    case MS_UP:
      tap_code(MS_WHLU);
      return false;
    case KC_MS_FAST_DOWN:
    case MS_DOWN:
      tap_code(MS_WHLD);
      return false;
    case KC_MS_FAST_LEFT:
    case MS_LEFT:
      tap_code(MS_WHLL);
      return false;
    case KC_MS_FAST_RIGHT:
    case MS_RGHT:
      tap_code(MS_WHLR);
      return false;
    }
  }

  // Handle simple tap-hold keys via table lookup
  for (size_t i = 0; i < SIMPLE_TAP_HOLD_COUNT; i++) {
    if (keycode == simple_tap_holds[i].keycode) {
      uint8_t idx = simple_tap_holds[i].th_idx;
      if (record->event.pressed) {
        TH_PRESS(idx);
      } else {
        th[idx].held = false;
        if (!th[idx].triggered) {
          tap_code(simple_tap_holds[i].tap_key);
        }
      }
      return false;
    }
  }

  // Handle RGB mode keys via table lookup
  if (record->event.pressed) {
    for (size_t i = 0; i < RGB_MODE_KEY_COUNT; i++) {
      if (keycode == rgb_mode_keys[i].keycode) {
        handle_rgb_mode_change(rgb_mode_keys[i].rgb_mode);
        return false;
      }
    }
  }

  switch (keycode) {
  case KC_EXIT:
    if (record->event.pressed) {
      snprintf(layer_reason_buffer, sizeof(layer_reason_buffer), "KC_EXIT (Tap): L0 (Key: R%u, C%u)", record->event.key.row, record->event.key.col);
      layer_change_reason = layer_reason_buffer;
      layer_move(0);
      rgb_matrix_indicators_user();
    }
    return false;
  case KC_TURBO:
    if (record->event.pressed) {
      pointing_device_set_cpi(3000);
      uprintf("[%lu.%03lu] Turbo ON. CPI -> 3000\n", sec, ms);
    } else {
      pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
      uprintf("[%lu.%03lu] Turbo OFF. CPI -> Default (%u)\n", sec, ms,
              pointing_device_get_cpi());
    }
    return false;
  case KC_MS_FAST_UP:    return handle_fast_mouse(MS_UP, record->event.pressed);
  case KC_MS_FAST_DOWN:  return handle_fast_mouse(MS_DOWN, record->event.pressed);
  case KC_MS_FAST_LEFT:  return handle_fast_mouse(MS_LEFT, record->event.pressed);
  case KC_MS_FAST_RIGHT: return handle_fast_mouse(MS_RGHT, record->event.pressed);
  case KC_MS_DIAG_UL:    return handle_diag_mouse(MS_UP, MS_LEFT, record->event.pressed);
  case KC_MS_DIAG_UR:    return handle_diag_mouse(MS_UP, MS_RGHT, record->event.pressed);
  case KC_MS_DIAG_DL:    return handle_diag_mouse(MS_DOWN, MS_LEFT, record->event.pressed);
  case KC_MS_DIAG_DR:    return handle_diag_mouse(MS_DOWN, MS_RGHT, record->event.pressed);
  case KC_SCR_MODE:
    if (record->event.pressed) {
      is_scroll_mode = true;
    } else {
      is_scroll_mode = false;
    }
    return false;
  case KC_ENT_MO4:
    if (record->event.pressed) {
      th[TH_ENT_MO].held = true;
      th[TH_ENT_MO].triggered = false;
      th[TH_ENT_MO].timer = timer_read();
    } else {
      th[TH_ENT_MO].held = false;
      if (!th[TH_ENT_MO].triggered) {
        tap_code(KC_ENT);
      } else {
        layer_change_reason = "ENT_MO4 (Release): L4->L0";
        layer_off(4);
        rgb_matrix_indicators_user();
      }
    }
    return false;
  case KC_ENT_EXIT:
    if (record->event.pressed) {
      uint32_t now = timer_read32();
      uint32_t sec = now / 1000;
      uint32_t ms = now % 1000;
      uprintf("[%lu.%03lu] KC_ENT_EXIT Pressed. Layer state: %lu\n", sec, ms,
              (unsigned long)layer_state);
      layer_state_to_restore = layer_state;
      layer_change_reason = "ENT_EXIT (Tap): L0";
      layer_state_set(0); // Clear all layers (peek at base)
      rgb_matrix_indicators_user();
      th[TH_ENT_MO].timer = timer_read();
    } else {
      uprintf("KC_ENT_EXIT Released. Elapsed: %u. Action: %s\n",
              timer_elapsed(th[TH_ENT_MO].timer),
              (timer_elapsed(th[TH_ENT_MO].timer) < MY_TAPPING_TERM) ? "Tap (Exit)"
                                                              : "Hold (Exit)");
      if (timer_elapsed(th[TH_ENT_MO].timer) < MY_TAPPING_TERM) {
        tap_code(KC_ENT);
        // Tap = Permanent Exit. We stay at layer 0.
      } else {
        // Hold = Momentary Peek. Restore the full layer state.
        // layer_state_set(layer_state_to_restore);
      }
      rgb_matrix_indicators_user();
    }
    return false;
  case KC_SPC_EXIT:  return handle_exit_key(KC_SPC, record->event.pressed);
  case KC_BSPC_EXIT: return handle_exit_key(KC_BSPC, record->event.pressed);
  case KC_L_TG1:
    if (record->event.pressed) {
      th[TH_L1].held = true;
      th[TH_L1].triggered = false;
      th[TH_L1].timer = timer_read();
    } else {
      th[TH_L1].held = false;
      // If released quickly (Tap) -> Toggle L3 (Mouse)
      if (!th[TH_L1].triggered) {
        if (get_highest_layer(layer_state) == 3) {
          layer_change_reason = "L_TG1 (Tap): Toggle L3 (OFF)";
          layer_off(3);
        } else if (get_highest_layer(layer_state) > 0) {
          layer_change_reason = "L_TG1 (Tap): L0 (Reset)";
          layer_move(0);
        } else {
          layer_change_reason = "L_TG1 (Tap): Toggle L3 (Mouse)";
          layer_on(3);
        }
      }
      rgb_matrix_indicators_user();
    }
    return false;
  case KC_R_TG2:
    if (record->event.pressed) {
      if (get_highest_layer(layer_state) > 0) {
        layer_change_reason = "R_TG2 (Tap): L0 (Reset)";
        layer_move(0);
      } else {
        layer_change_reason = "R_TG2 (Tap): Toggle L2 (Invert)";
        layer_invert(2);
      }
      rgb_matrix_indicators_user();
    }
    return false;
  case KC_P_TO0:
    if (record->event.pressed) {
      th[TH_P].held = true;
      th[TH_P].triggered = false;
      th[TH_P].timer = timer_read();
    } else {
      th[TH_P].held = false;
      if (!th[TH_P].triggered) {
        tap_code(KC_P);
      } else {
        // Hold was triggered, restore layer 4
        layer_change_reason = "P(Hold Release): L4";
        layer_move(4);
      }
    }
    return false;
   case KC_Q_TG4:
     if (record->event.pressed) {
       th[TH_Q].held = true;
       th[TH_Q].triggered = false;
       th[TH_Q].timer = timer_read();
     } else {
       th[TH_Q].held = false;
       if (!th[TH_Q].triggered) {
         tap_code(KC_Q);
       } else {
         // Hold was triggered, restore base layer
         layer_change_reason = "Q(Hold Release): Base";
         layer_move(0);
       }
     }
     return false;
  // KC_PGUP_TO0, KC_HOME_TO0, KC_RAINBOW, KC_REACTIVE now handled by table lookup
  case RM_NEXT:
    if (record->event.pressed) {
      rgb_matrix_step_noeeprom();
      handle_rgb_mode_change(rgb_matrix_get_mode());
    }
    return false;
  case RM_PREV:
    if (record->event.pressed) {
      rgb_matrix_step_reverse_noeeprom();
      handle_rgb_mode_change(rgb_matrix_get_mode());
    }
    return false;
  case KC_MOUSE_LOCK:
    if (record->event.pressed) {
      mouse_is_locked = !mouse_is_locked;
      sync_needed = true;
      if (mouse_is_locked) {
        layer_change_reason = "Mouse Lock ON";
        layer_on(3);
      } else {
        layer_change_reason = "Mouse Lock OFF";
        layer_off(3);
      }
    }
    return false;
  case KC_SNIPE:
    if (record->event.pressed) {
      is_sniping_active = true;
      is_fast_mode_active = false;
      snipe_timer = timer_read();
      sync_needed = true;
      pointing_device_set_cpi(250);
      mk_max_speed = 1; // Very slow mouse keys
      mk_interval = 24;
      uprintf("[%lu.%03lu] Snipe ON (2.5s). CPI -> 250, MK -> Slow(1)\n", sec, ms);
    }
    return false;
  case KC_FAST:
    if (record->event.pressed) {
      is_fast_mode_active = true;
      is_sniping_active = false;
      fast_mode_timer = timer_read();
      sync_needed = true;
      pointing_device_set_cpi(3000);
      mk_max_speed = 30; // Fast mouse keys
      mk_interval = 10;
      uprintf("[%lu.%03lu] Fast Mode ON (2.5s). CPI -> 3000, MK -> Fast\n", sec, ms);
    }
    return false;
  case KC_ENT_L2_EXIT:
    if (record->event.pressed) {
      uprintf("KC_ENT_L2_EXIT Pressed. Exiting Layer 2.\n");
      layer_change_reason = "ENT_L2_EXIT";
      layer_off(2);
      tap_code(KC_ENT);
    }
    return false;
  case KC_1_TG1:
    if (record->event.pressed) {
      th[TH_K1].held = true;
      th[TH_K1].triggered = false;
      th[TH_K1].timer = timer_read();
    } else {
      th[TH_K1].held = false;
      if (!th[TH_K1].triggered) {
        uint8_t layer = get_highest_layer(layer_state);
        if (layer == 2)
          tap_code(KC_F1);
        else if (layer == 3) {
          handle_rgb_mode_change(RGB_MATRIX_CYCLE_LEFT_RIGHT);
        } else if (layer == 4)
          tap_code(KC_0);
        else
          tap_code(KC_1);
      }
    }
    return false;
  case KC_2_TG2:
    if (record->event.pressed) {
      th[TH_K2].held = true;
      th[TH_K2].triggered = false;
      th[TH_K2].timer = timer_read();
    } else {
      th[TH_K2].held = false;
      if (!th[TH_K2].triggered) {
        uint8_t layer = get_highest_layer(layer_state);
        if (layer == 2)
          tap_code(KC_F2);
        else if (layer == 3) {
          rgb_matrix_step_noeeprom();
          handle_rgb_mode_change(rgb_matrix_get_mode());
        } else if (layer == 4)
          tap_code(KC_9);
        else
          tap_code(KC_2);
      }
    }
    return false;
  case KC_3_TG3:
    if (record->event.pressed) {
      th[TH_K3].held = true;
      th[TH_K3].triggered = false;
      th[TH_K3].timer = timer_read();
    } else {
      th[TH_K3].held = false;
      if (!th[TH_K3].triggered) {
        uint8_t layer = get_highest_layer(layer_state);
        if (layer == 2)
          tap_code(KC_F3);
        else if (layer == 3)
          tap_code(KC_3);
        else if (layer == 4)
          tap_code(KC_8);
        else
          tap_code(KC_3);
      }
    }
    return false;
  case KC_4_TG4:
    if (record->event.pressed) {
      th[TH_K4].held = true;
      th[TH_K4].triggered = false;
      th[TH_K4].timer = timer_read();
    } else {
      th[TH_K4].held = false;
      if (!th[TH_K4].triggered) {
        uint8_t layer = get_highest_layer(layer_state);
        if (layer == 2)
          tap_code(KC_F4);
        else if (layer == 3)
          tap_code(KC_4);
        else if (layer == 4)
          tap_code(KC_7);
        else
          tap_code(KC_4);
      }
    }
    return false;
  case KC_5_TG5:
    if (record->event.pressed) {
      th[TH_K5].held = true;
      th[TH_K5].triggered = false;
      th[TH_K5].timer = timer_read();
    } else {
      th[TH_K5].held = false;
      if (!th[TH_K5].triggered) {
        uint8_t layer = get_highest_layer(layer_state);
        if (layer == 2)
          tap_code(KC_F5);
        else
          tap_code(KC_5);
      }
    }
    return false;
  // KC_JELLY, KC_SPIRAL, KC_CHEVRON now handled by RGB mode table lookup
  case KC_RGB_AUTO:
    if (record->event.pressed) {
      rgb_auto_cycle = !rgb_auto_cycle;
      rgb_auto_timer = timer_read();
      // Not syncing this explicitly as it's a background process, but maybe we
      // should? Let's rely on master driving animations if synced.
    }
    return false;
  case RM_HUEU:
  case RM_HUED:
  case RM_SATU:
  case RM_SATD:
    if (record->event.pressed) {
      uint8_t mode = rgb_matrix_get_mode();
      if (mode == RGB_MATRIX_CYCLE_LEFT_RIGHT || mode == RGB_MATRIX_CYCLE_ALL ||
          mode == RGB_MATRIX_CYCLE_SPIRAL) {
        handle_rgb_mode_change(RGB_MATRIX_SOLID_COLOR);
      }
    }
    return true;
  case RM_VALU:
  case RM_VALD:
    if (record->event.pressed) {
      uprintf("RM_VAL change requested. Current Val: %d\n",
              rgb_matrix_get_val());
    }
    return true;
  case KC_PLUS_COLON:
    if (record->event.pressed) {
      if (get_mods() & MOD_MASK_SHIFT) {
        // Shift held: send colon (shift is already active)
        tap_code(KC_SCLN);
      } else {
        // No shift: send plus
        tap_code16(S(KC_EQL));
      }
    }
    return false;
  // KC_MINS_TO0 now handled by simple tap-hold table lookup
  case KC_SLSH_TO0:
    if (record->event.pressed) {
      th[TH_SLSH].held = true;
      th[TH_SLSH].triggered = false;
      th[TH_SLSH].timer = timer_read();
    } else {
      th[TH_SLSH].held = false;
      if (!th[TH_SLSH].triggered) {
        tap_code(KC_SLSH);
      } else {
        // Hold was triggered, restore layer 4
        layer_change_reason = "/ (Hold Release): L4";
        layer_move(4);
      }
    }
    return false;
  // KC_0_TG1, KC_9_TG2, KC_8_TG3, KC_7_TO0, KC_6_TO0 now handled by simple tap-hold table lookup
  // Verbose thumb toggle keys with debug logging
  case KC_ENT_TG2: return handle_thumb_toggle(TH_ENT_TG2, KC_ENT, "KC_ENT_TG2", record->event.pressed);
  case KC_ENT_TG4: return handle_thumb_toggle(TH_ENT_TG4, KC_ENT, "KC_ENT_TG4", record->event.pressed);
  case KC_SPC_TG2: return handle_thumb_toggle(TH_SPC_TG2, KC_SPC, "KC_SPC_TG2", record->event.pressed);
  case KC_SPC_TG4: return handle_thumb_toggle(TH_SPC_TG4, KC_SPC, "KC_SPC_TG4", record->event.pressed);
  // KC_PMNS_TG4, KC_F12_EXIT now handled by simple tap-hold table lookup
  // KC_MINS_TG4 shares TH_PMNS_TG4 index but different tap key, keeping explicit
  case KC_MINS_TG4:
    if (record->event.pressed) {
      TH_PRESS(TH_PMNS_TG4);
    } else {
      th[TH_PMNS_TG4].held = false;
      if (!th[TH_PMNS_TG4].triggered) {
        tap_code(KC_MINS);
      }
    }
    return false;
  case KC_FIRE:
        if (record->event.pressed) {
          handle_rgb_mode_change(RGB_MATRIX_CUSTOM_fire);
        }
        return false;
      case KC_CPFR:
        if (record->event.pressed) {
          handle_rgb_mode_change(RGB_MATRIX_CUSTOM_campfire);
        }
        return false;
  case DPI_MOD:
    if (record->event.pressed) {
      // Allow core to handle it first (return true), but log current/new DPI?
      // Actually, core handles it on press. We can log after?
      // Or just log that we pressed it.
      uprintf("DPI+ Pressed. Current CPI: %u\n", pointing_device_get_cpi());
    }
    return true; // Let core handle the DPI change
  case DPI_RMOD:
    if (record->event.pressed) {
      uprintf("DPI- Pressed. Current CPI: %u\n", pointing_device_get_cpi());
    }
    return true; // Let core handle the DPI change
  case KC_MS_TMO_INC:
    if (record->event.pressed) {
      auto_mouse_timeout += 500;
      if (auto_mouse_timeout > 10000) {
        auto_mouse_timeout = 10000; // Max 10 seconds
      }
      uprintf("Auto-mouse timeout: %u ms\n", auto_mouse_timeout);
    }
    return false;
  case KC_MS_TMO_DEC:
    if (record->event.pressed) {
      if (auto_mouse_timeout > 500) {
        auto_mouse_timeout -= 500;
      }
      if (auto_mouse_timeout < 500) {
        auto_mouse_timeout = 500; // Min 0.5 seconds
      }
      uprintf("Auto-mouse timeout: %u ms\n", auto_mouse_timeout);
    }
    return false;
  // KC_P_FRAC, KC_PINWHEEL now handled by RGB mode table lookup
  case KC_DAY:
    if (record->event.pressed) {
      // Set High Brightness (Day Mode)
      // Keep current Hue/Sat, set Value to 225
      rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), 225);
      uprintf("Day Mode: Brightness set to 225\n");
    }
    return false;
  case KC_NIGHT:
    if (record->event.pressed) {
      // Set Low Brightness (Night Mode)
      // Keep current Hue/Sat, set Value to 16
      rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), 16);
      uprintf("Night Mode: Brightness set to 16\n");
    }
    return false;
  case KC_FLASHLIGHT:
    if (record->event.pressed) {
      if (!is_flashlight) {
        saved_rgb_mode = rgb_matrix_get_mode();
        saved_rgb_h = rgb_matrix_get_hue();
        saved_rgb_s = rgb_matrix_get_sat();
        saved_rgb_v = rgb_matrix_get_val();
        handle_rgb_mode_change(RGB_MATRIX_SOLID_COLOR);
        rgb_matrix_sethsv_noeeprom(HSV_WHITE);
        is_flashlight = true;
        uprintf("Flashlight ON\n");
      } else {
        handle_rgb_mode_change(saved_rgb_mode);
        rgb_matrix_sethsv_noeeprom(saved_rgb_h, saved_rgb_s, saved_rgb_v);
        is_flashlight = false;
        uprintf("Flashlight OFF\n");
      }
      sync_needed = true;
    }
    return false;
  case KC_SET_LEFT:
    if (record->event.pressed) {
      eeconfig_update_handedness(true);  // true = left hand
      uprintf("\n*** EEPROM SET: LEFT HAND ***\n");
      uprintf("Please unplug and replug keyboard for change to take effect.\n\n");
    }
    return false;
  case KC_SET_RIGHT:
    if (record->event.pressed) {
      eeconfig_update_handedness(false);  // false = right hand
      uprintf("\n*** EEPROM SET: RIGHT HAND ***\n");
      uprintf("Please unplug and replug keyboard for change to take effect.\n\n");
    }
    return false;
  case KC_JITTER:
    if (record->event.pressed) {
      is_jitter_filter_active = !is_jitter_filter_active;
      sync_needed = true;
      uprintf("[%lu.%03lu] Jitter Filter: %s\n", sec, ms, is_jitter_filter_active ? "ON" : "OFF");

      // Update EEPROM
      uint16_t current_config = eeconfig_read_user();
      if (is_jitter_filter_active) {
        current_config |= 0x0080; // Set bit 7
      } else {
        current_config &= ~0x0080; // Clear bit 7
      }
      eeconfig_update_user(current_config);
    }
    return false;
  case KC_DEBUG_SYNC:
    if (record->event.pressed) {
      debug_dump_sync_state();
    }
    return false;
  // Layer 3 Thumb Logic: Tap = Exit, Hold = Switch Layer
  case KC_L3_EXT_TO4: return handle_l3_thumb(TH_L3_TO4, 4, record->event.pressed);
  case KC_L3_EXT_TO2: return handle_l3_thumb(TH_L3_TO2, 2, record->event.pressed);
  case KC_L3_EXT_TO1: return handle_l3_thumb(TH_L3_TO1, 1, record->event.pressed);
  }
  return true;
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
  case RM_HUEU:
  case RM_HUED:
    if (record->event.pressed) {
      uprintf("Hue change: %d (S:%d, V:%d)\n", rgb_matrix_get_hue(),
              rgb_matrix_get_sat(), rgb_matrix_get_val());
    }
    break;
  case RM_SATU:
  case RM_SATD:
    if (record->event.pressed) {
      uprintf("Saturation change: %d (H:%d, V:%d)\n", rgb_matrix_get_sat(),
              rgb_matrix_get_hue(), rgb_matrix_get_val());
    }
    break;
  case RM_VALU:
  case RM_VALD:
    if (record->event.pressed) {
      uprintf("Brightness change: %d (H:%d, S:%d)\n", rgb_matrix_get_val(),
              rgb_matrix_get_hue(), rgb_matrix_get_sat());
    }
    break;
  }
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] =
        LAYOUT(QK_GESC, KC_1_TG1, KC_2_TG2, KC_3_TG3, KC_4_TG4, KC_5_TG5,   KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS,
               KC_TAB , KC_Q_TG4, KC_W    , KC_E    , KC_R    , KC_T    ,   KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,
               KC_LSFT, KC_A    , KC_S    , KC_D    , KC_F    , KC_G    ,   KC_H, KC_J, KC_K, KC_L, KC_PLUS_COLON, KC_QUOT,
               KC_LCTL, TD(TD_Z_LAYER), KC_X, KC_C  , KC_V    , KC_B    ,   KC_N, KC_M, KC_COMM, KC_DOT, LT(3, KC_SLSH), KC_RSFT,
                                        KC_SPC_TG4, KC_ENT_TG2, KC_L_TG1,   KC_DEL, KC_ENT_TG2,
                                                        KC_LALT, KC_BSPC,   KC_BSPC),
    [1] = LAYOUT(KC_PSCR, KC_EXIT    , TO(2), TO(3), TO(4), KC_EXIT,   KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT   , KC_PSCR   , QK_BOOT,
                 KC_TAB , KC_MINS_TO0, KC_7 , KC_8 , KC_9 , KC_EXIT,   KC_EXIT, KC_LBRC, KC_RBRC, S(KC_LBRC), S(KC_RBRC), HYPR(KC_N),
                 KC_EXIT, S(KC_EQL)  , KC_4 , KC_5 , KC_6 , KC_EXIT,   KC_EXIT, KC_LEFT, KC_UP  , KC_DOWN   , KC_RGHT   , KC_EXIT,
                 KC_LCTL, KC_0       , KC_1 , KC_2 , KC_3 , KC_EQL ,   KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT   , KC_EXIT   , KC_EXIT,
                                 KC_SPC_EXIT, KC_ENT_EXIT, KC_L_TG1,   KC_R_TG2, KC_ENT_EXIT,
                                              KC_LALT, KC_BSPC_EXIT,   KC_BSPC_EXIT),
    [2] = LAYOUT(KC_EXIT, KC_F1  , KC_F2  , KC_F3  , KC_F4  , KC_F5        ,   KC_F6      , KC_F7  , KC_F8  , KC_F9  , KC_F10 , QK_BOOT,
                 KC_PSCR, KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT, QK_BOOT      ,   KC_EXIT    , KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT, KC_F11,
                 KC_LSFT, KC_LEFT, KC_UP  , KC_DOWN, KC_RGHT, LALT(KC_HOME),   KC_EXIT    , KC_LEFT, KC_UP  , KC_DOWN, KC_RGHT, KC_F12,
                 KC_LCTL, KC_HOME, KC_PGUP, KC_PGDN, KC_END , KC_EXIT      ,   KC_EXIT, KC_HOME, KC_PGUP, KC_PGDN, KC_END , KC_RSFT,
                                         KC_SPC_EXIT, KC_ENT_EXIT, KC_L_TG1,   KC_EXIT, KC_ENT_EXIT,
                                                      KC_LALT, KC_BSPC_EXIT,   KC_BSPC_EXIT),
    [3] = LAYOUT(QK_GESC        , KC_EXIT      , KC_MS_FAST_UP, KC_EXIT       , KC_EXIT         , QK_BOOT,   KC_EXIT, KC_RCTL , KC_RALT, KC_RGUI       , KC_EXIT, QK_BOOT,
                 KC_EXIT        , KC_MS_DIAG_UL, MS_UP        , KC_MS_DIAG_UR , MS_BTN1         , KC_EXIT,   KC_EXIT, KC_SNIPE, KC_FAST, KC_EXIT       , KC_EXIT, KC_EXIT,
                 KC_MS_FAST_LEFT, MS_LEFT      , MS_BTN1      , MS_RGHT       , KC_MS_FAST_RIGHT, MS_BTN2,   KC_EXIT, MS_BTN1 , DRGSCRL, KC_MOUSE_LOCK , MS_BTN2, KC_EXIT,
                 KC_EXIT        , KC_MS_DIAG_DL, MS_DOWN      , KC_MS_DIAG_DR , MS_BTN3         , KC_EXIT,   KC_EXIT, KC_EXIT , MS_BTN3, MS_BTN3       , MS_BTN3, KC_RSFT,
                                               KC_L3_EXT_TO4 , KC_L3_EXT_TO2 , KC_L3_EXT_TO1   ,             KC_EXIT, KC_EXIT,
                                                                       KC_EXIT, KC_EXIT        ,             KC_EXIT),
    [4] = LAYOUT(KC_MINS_TO0, KC_0_TG1, KC_9_TG2, KC_8_TG3, KC_7_TO0, KC_6_TO0,   KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS,
                 KC_BSLS    , KC_P_TO0, KC_O    , KC_I    , KC_U    , KC_Y    ,   KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,
                 KC_QUOT, KC_PLUS_COLON, KC_L   , KC_K    , KC_J    , KC_H    ,   KC_H, KC_J, KC_K, KC_L, KC_PLUS_COLON, KC_QUOT,
                  KC_LCTL, KC_SLSH_TO0, KC_DOT, KC_COMM , KC_M    , KC_N    ,   KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH_TO0, KC_RCTL,
                                                  KC_SPC_EXIT, KC_ENT_EXIT, KC_LSFT,   KC_R_TG2, KC_ENT_EXIT,
                                                              KC_LALT, KC_BSPC,   KC_BSPC),
    // Settings Layer - RGB and Mouse configuration (accessed via Hold '5')
    [5] = LAYOUT(KC_PSCR, KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT    , KC_EXIT  ,   KC_DEBUG_SYNC, RM_HUEU   , RM_HUED, RM_SATU , RM_SATD  , KC_PSCR,
                 KC_EXIT, RM_TOGG, RM_NEXT, RM_PREV, KC_RGB_AUTO, KC_P_FRAC,   KC_FIRE    , KC_EXIT     , RM_VALU, RM_VALD, KC_EXIT  , QK_CLEAR_EEPROM,
                 KC_EXIT, KC_EXIT, KC_FLASHLIGHT, KC_EXIT, KC_DAY, KC_NIGHT,   KC_EXIT    , DPI_MOD     , DPI_RMOD, KC_JITTER, KC_EXIT, KC_EXIT,
                 KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT     , KC_EXIT ,   KC_PINWHEEL, KC_MS_TMO_INC, KC_MS_TMO_DEC, KC_EXIT, KC_EXIT, KC_EXIT,
                                                  KC_EXIT, KC_EXIT, KC_EXIT,   KC_EXIT, KC_EXIT,
                                                           KC_EXIT, KC_EXIT,   KC_EXIT),
};

bool rgb_matrix_indicators_user(void) {
  if (is_flashlight) {
    rgb_matrix_set_color_all(255, 255, 255);
    return false;
  }

  // Caps Lock indicator: alternating blink on top-left and top-right keys
  if (is_caps_lock_on) {
    bool phase = (timer_read() / 300) % 2; // Alternates every 300ms
    bool am_i_left = is_keyboard_left();

    if (am_i_left) {
      // Top-left key (LED 0 = Escape) on left half
      if (phase) {
        rgb_matrix_set_color(far_left_col[0], 255, 255, 255);
      } else {
        rgb_matrix_set_color(far_left_col[0], 0, 0, 0);
      }
    } else {
      // Top-right key (Minus) on right half - use same index pattern as snipe mode
      if (!phase) {
        rgb_matrix_set_color(far_right_col[0], 255, 255, 255);
      } else {
        rgb_matrix_set_color(far_right_col[0], 0, 0, 0);
      }
    }
  }

  if (is_jitter_filter_active) {
    if (!is_keyboard_left()) {
      // Trackball underglow LEDs (Right Side) -> Cyan for Stability
      rgb_matrix_set_color(27, 0, 255, 255);
      rgb_matrix_set_color(28, 0, 255, 255);
    }
  }

  bool is_scroll_active = charybdis_get_pointer_dragscroll_enabled();

  if (is_sniping_active) {
    // Snipe mode: Black out top row on RIGHT side, rainbow far right column
    if (!is_keyboard_left()) {
      for (int i = 0; i < sizeof(top_row_right); i++) {
        rgb_matrix_set_color(top_row_right[i], 0, 0, 0);
      }
      for (int i = 0; i < sizeof(far_right_col); i++) {
        uint8_t hue = (i * 64) + (timer_read() / 10); // Cycling rainbow
        HSV hsv = {hue, 255, 255};
        RGB rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(far_right_col[i], rgb.r, rgb.g, rgb.b);
      }
    }
    return false;
  }

  if (is_fast_mode_active) {
    // Fast mode: Black out top row on RIGHT side, RED far right column
    if (!is_keyboard_left()) {
      for (int i = 0; i < sizeof(top_row_right); i++) {
        rgb_matrix_set_color(top_row_right[i], 0, 0, 0);
      }
      for (int i = 0; i < sizeof(far_right_col); i++) {
        rgb_matrix_set_color(far_right_col[i], 255, 0, 0);
      }
    }
    return false;
  }

  if (is_scroll_active) {
    // Scroll mode: Black out top row on LEFT side, rainbow far left column
    if (is_keyboard_left()) {
      for (int i = 0; i < sizeof(top_row_left); i++) {
        rgb_matrix_set_color(top_row_left[i], 0, 0, 0);
      }
      for (int i = 0; i < sizeof(far_left_col); i++) {
        uint8_t hue = (i * 64) + (timer_read() / 10); // Cycling rainbow
        HSV hsv = {hue, 255, 255};
        RGB rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(far_left_col[i], rgb.r, rgb.g, rgb.b);
      }
    }
    return false;
  }

  uint8_t layer = get_highest_layer(layer_state);
  switch (layer) {
  case 1: {
    // Numpad (Blue)
    static const uint8_t leds[] = {6,  9,  14, 17, 21, 5,  10, 13,
                                   18, 22, 4,  11, 12, 19, 23};
    for (int i = 0; i < sizeof(leds); i++)
      rgb_matrix_set_color(leds[i], 0, 0, 255);
    break;
  }
  case 2: {
    // Arrow (Green)
    if (is_keyboard_left()) {
      static const uint8_t left[] = {5, 10, 13, 18, 4, 11, 12, 19};
      for (int i = 0; i < sizeof(left); i++)
        rgb_matrix_set_color(left[i], 0, 255, 0);
    }
    if (!is_keyboard_left()) {
      static const uint8_t right[] = {18, 13, 10, 5, 19, 12, 11, 4};
      for (int i = 0; i < sizeof(right); i++)
        rgb_matrix_set_color(right[i], 0, 255, 0);
    }
    break;
  }
  case 3: {
    // Mouse (Yellow)
    if (is_keyboard_left()) {
      static const uint8_t left[] = {8,  1,  9, 14, 17, 21, 2,  5,  10,
                                     13, 18, 4, 11, 12, 19, 26, 25, 24};
      for (int i = 0; i < sizeof(left); i++)
        rgb_matrix_set_color(left[i], 255, 255, 0);
    }
    if (!is_keyboard_left()) {
      static const uint8_t right[] = {21, 14, 9, 22, 19, 12, 11, 4};
      for (int i = 0; i < sizeof(right); i++)
        rgb_matrix_set_color(right[i], 255, 255, 0);
    }
    break;
  }
  case 4: {
    // Left Side (Orange)
    if (is_keyboard_left()) {
      static const uint8_t left[] = {0,  7,  8,  15, 16, 20, 1,  6,  9, 14,
                                     17, 21, 2,  5,  10, 13, 18, 22, 3, 4,
                                     11, 12, 19, 23, 26, 27, 28, 25, 24};
      for (int i = 0; i < sizeof(left); i++)
        rgb_matrix_set_color(left[i], 255, 127, 0);
    }
    break;
  }
  case 5: {
    // Settings (Hot Pink)
    if (is_keyboard_left()) {
      // Left side: RGB control keys (Q, W, E, R positions + A, S, D, F + Z, X)
      static const uint8_t left[] = {6, 9, 14, 17, 5, 10, 13, 18, 4, 11};
      for (int i = 0; i < sizeof(left); i++)
        rgb_matrix_set_color(left[i], 255, 110, 150);
    }
    if (!is_keyboard_left()) {
      // Right side: Mouse setting keys (U, I, O positions + J, K, L)
      static const uint8_t right[] = {17, 14, 9, 18, 13, 10};
      for (int i = 0; i < sizeof(right); i++)
        rgb_matrix_set_color(right[i], 255, 110, 150);
    }
    break;
  }
  default:
    break;
  }

  // Black out the number and letter for the current active layer (0-4)
  // This creates a "negative" indicator hole in the lighting
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

void keyboard_post_init_user(void) {
  transaction_register_rpc(USER_SYNC_INFO, user_sync_info_slave_handler);



  if (!eeconfig_is_enabled()) {
    uprintf("Init: EEPROM not enabled, initializing...\n");
    eeconfig_init();
  }

  // Read user config from EEPROM
  // Bits 0-5: Theme Index (Max 63)
  // Bit 7: Jitter Filter Active
  // Upper 8 bits: Start Hue
  uint16_t user_config = eeconfig_read_user();

  // Restore State
  is_jitter_filter_active = (user_config & 0x0080) ? true : false;

  uint8_t saved_theme = (uint8_t)(user_config & 0x3F);
  uint8_t saved_hue = (uint8_t)((user_config >> 8) & 0xFF);
  uint8_t max_effects = RGB_MATRIX_EFFECT_MAX;

  uprintf("Init: EEPROM Read=%u (Theme %d, Hue %d, Jitter %d)\n", user_config, saved_theme,
          saved_hue, is_jitter_filter_active);

  // Increment Theme (Cycle 1 to MAX-1)
  uint8_t next_theme = saved_theme + 1;
  if (next_theme >= max_effects || next_theme == 0) {
    next_theme = 1;
  }

  // Increment Hue (Shift by ~42 for distinct color steps)
  // 0=Red, 42=Yellow, 84=Green, 126=Cyan, 168=Blue, 210=Magenta
  uint8_t next_hue = saved_hue + 42;

  // Save Combined - Defer write to avoid USB timeout
  uint16_t new_config = ((uint16_t)next_hue << 8) | (next_theme & 0x3F);
  if (is_jitter_filter_active) {
    new_config |= 0x0080;
  }
  
  pending_eeprom_config = new_config;
  eeprom_update_pending = true;
  eeprom_defer_timer = timer_read32();
  
  uprintf("Init: Deferred EEPROM write. Pending Config=%u\n", new_config);

  // Initialize the tracker with the OLD hue, so the first increment
  // in handle_rgb_mode_change() lands exactly on 'next_hue'.
  automatic_hue_tracker = saved_hue;

  uprintf("Init: Next Theme=%d, Next Hue=%d. Saved=%u. CPI: %u\n", next_theme, next_hue,
          new_config, pointing_device_get_cpi());

  // Defer RGB mode application to matrix_scan for timing reliability
  master_rgb_init_mode = next_theme;
  master_rgb_init_pending = true;

  rgb_auto_cycle = true;
  rgb_auto_timer = timer_read();
}

void housekeeping_task_user(void) {
  if (is_keyboard_master()) {
    static uint32_t last_sync = 0;
    // Poll faster (50ms) to detect slave mouse movement for layer switching
    bool needs_periodic = timer_elapsed32(last_sync) > 50;

    // Update caps lock state on master
    is_caps_lock_on = host_keyboard_led_state().caps_lock;



    static user_sync_info_response_t last_slave_response = {0};

    if (sync_needed || needs_periodic) {
      user_sync_info_t sync_data = {
          .is_flashlight = is_flashlight,
          .is_sniping_active = is_sniping_active,
          .is_fast_mode_active = is_fast_mode_active,
          .mouse_is_locked = mouse_is_locked,
          .is_jitter_filter_active = is_jitter_filter_active,
          .is_caps_lock_on = is_caps_lock_on,
          .rgb_mode = rgb_matrix_get_mode(),
          .is_left_hand = is_keyboard_left(),
          .random_seed = current_random_seed};

      user_sync_info_response_t response_data = {0};

      static uint8_t last_synced_mode = 0;
      if (transaction_rpc_exec(USER_SYNC_INFO, sizeof(sync_data), &sync_data,
                               sizeof(response_data), &response_data)) {
        last_sync = timer_read32();
        last_sync_time = last_sync;
        sync_success_count++;
        last_slave_response = response_data; // Cache for heartbeat

        if (response_data.did_rgb_sync && sync_data.rgb_mode != last_synced_mode) {
          LOG_TIME();
          uprintf("\033[92mMaster: Slave synced RGB to mode %d (seed=%u)\033[0m\n", sync_data.rgb_mode, current_random_seed);
          last_synced_mode = sync_data.rgb_mode;
        }
        
        // Handle Slave Mouse Activity (Auto Layer 3)
        if (response_data.mouse_active) {
            if (!auto_mouse_on) {
                layer_change_reason = "Slave Mouse Movement";
                layer_on(3);
                auto_mouse_on = true;
                uprintf("Master: Activated Layer 3 due to Slave Mouse\n");
            }
            // Keep timer alive
            auto_mouse_timer = timer_read();
        }

        sync_needed = false;
      } else {
        sync_fail_count++;
      }
    }

    // Periodic heartbeat - show sync health every HEARTBEAT_INTERVAL
    if (timer_elapsed32(last_heartbeat_time) > HEARTBEAT_INTERVAL) {
      last_heartbeat_time = timer_read32();
      uint32_t since_sync = timer_elapsed32(last_sync_time);
      LOG_TIME();
      uprintf("\033[36m[Heartbeat] Mode=%d (%s) Seed=%u Syncs=%lu/%lu LastSync=%lu.%03lus ago Slave(Mode=%d Ctr=%u)\033[0m\n",
              rgb_matrix_get_mode(), get_rgb_mode_name(rgb_matrix_get_mode()),
              current_random_seed, (unsigned long)sync_success_count, (unsigned long)sync_fail_count,
              (unsigned long)(since_sync / 1000), (unsigned long)(since_sync % 1000),
              last_slave_response.slave_rgb_mode, last_slave_response.slave_task_counter);
    }
  }
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
  int8_t x = mouse_report.x;
  int8_t y = mouse_report.y;
  uint32_t now = timer_read32();
  uint32_t sec = now / 1000;
  uint32_t ms = now % 1000;

  if (get_highest_layer(layer_state) == 3 && (x != 0 || y != 0)) {
    static uint16_t last_cpi = 0;
    static uint32_t last_cpi_print = 0;
    uint16_t current_cpi = pointing_device_get_cpi();

    // Log if CPI changed OR if it's been > 2 seconds since last log
    if (current_cpi != last_cpi || timer_elapsed32(last_cpi_print) > 2000) {
      uprintf("[%lu.%03lu] Mouse L3 Move. CPI: %u (x=%d, y=%d)\n", sec, ms,
              current_cpi, x, y);
      last_cpi = current_cpi;
      last_cpi_print = now;
    }
  }

  int8_t threshold = 0; // Respond to any movement (threshold 0)
  static uint8_t movement_streak = 0;

  if (is_jitter_filter_active) {
    // Filter out 1-unit orthogonal movements (jitter)
    // Keep diagonals (e.g. x=1, y=1) or larger movements
    if ((x == 0 && (y == 1 || y == -1)) || (y == 0 && (x == 1 || x == -1))) {
      static uint32_t last_jitter_log = 0;
      if (timer_elapsed32(last_jitter_log) > 1000) {
        LOG_TIME();
        uprintf("\033[95mMouse: Jitter Filtered (x=%d, y=%d)\033[0m\n", x, y);
        last_jitter_log = timer_read32();
      }
      // Zero out the report so it's ignored by the host AND by the auto-layer logic below
      mouse_report.x = 0;
      mouse_report.y = 0;
      x = 0;
      y = 0;
    }
  }

  if (x != 0 || y != 0) {
    if (auto_mouse_on) {
      // Any movement resets the timer to allow fine precision without timeout
      auto_mouse_timer = timer_read();
    } else if (x > threshold || x < -threshold || y > threshold ||
               y < -threshold) {
      movement_streak++;
      if (movement_streak > 1) { // Require 2 consecutive movement events
        // Only activate layer for significant movement
        if (is_keyboard_master()) {
             uprintf("Mouse: Activated (x=%d, y=%d)\n", x, y);
             layer_change_reason = "Auto Mouse Movement";
             layer_on(3); // Switch to Mouse Layer (3)
             auto_mouse_on = true;
             auto_mouse_timer = timer_read();
        } else {
             // Slave side: Signal master
             slave_mouse_active = true;
        }
        movement_streak = 0;
      }
    } else {
      // Only log jitter if it's above 0 to reduce console noise
      if (x != 0 || y != 0) {
        uprintf("Mouse: Jitter Ignored (x=%d, y=%d)\n", x, y);
      }
      movement_streak = 0;
    }
  } else {
    movement_streak = 0;
  }
  return mouse_report;
}

void matrix_scan_user(void) {
  // Handle deferred EEPROM update (wait >1s after boot)
  if (eeprom_update_pending && timer_elapsed32(eeprom_defer_timer) > 1500) {
    eeconfig_update_user(pending_eeprom_config);
    eeprom_update_pending = false;
    uprintf("Deferred EEPROM update executed. Config=%u\n", pending_eeprom_config);
  }

  // Deferred RGB init - apply after matrix is fully ready
  if (master_rgb_init_pending) {
    if (is_keyboard_master()) {
      handle_rgb_mode_change(master_rgb_init_mode);
    } else {
      rgb_matrix_mode_noeeprom(master_rgb_init_mode);
    }
    master_rgb_init_pending = false;
    uprintf("Deferred RGB init: mode %d applied\n", master_rgb_init_mode);
  }

  // Timeout for Snipe Mode (2.5s)
  if (is_sniping_active && timer_elapsed(snipe_timer) > 2500) {
    is_sniping_active = false;
    pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
    mk_max_speed = 12; // Restore default
    mk_interval = 16;
    sync_needed = true;
    uprintf("Snipe Mode Timeout. CPI -> Default\n");
  }

  // Timeout for Fast Mode (2.5s)
  if (is_fast_mode_active && timer_elapsed(fast_mode_timer) > 2500) {
    is_fast_mode_active = false;
    pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
    mk_max_speed = 12; // Restore default
    mk_interval = 16;
    sync_needed = true;
    uprintf("Fast Mode Timeout. CPI -> Default\n");
  }

  if (rgb_auto_cycle && timer_elapsed(rgb_auto_timer) > 30000) {
    rgb_matrix_step_noeeprom();
    rgb_auto_timer = timer_read();
    if (is_keyboard_master()) {
      handle_rgb_mode_change(rgb_matrix_get_mode());
    }
  }

  if (auto_mouse_on && !mouse_is_locked &&
      timer_elapsed(auto_mouse_timer) > auto_mouse_timeout) {
    snprintf(layer_reason_buffer, sizeof(layer_reason_buffer), "Auto Mouse Timeout (%u ms)", auto_mouse_timeout);
    layer_change_reason = layer_reason_buffer;
    layer_off(3);
    auto_mouse_on = false;
  }

  if (th[TH_PGUP].held && !th[TH_PGUP].triggered &&
      timer_elapsed(th[TH_PGUP].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "PGUP(Hold): Exit to Base";
    layer_move(0);
    th[TH_PGUP].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_HOME].held && !th[TH_HOME].triggered &&
      timer_elapsed(th[TH_HOME].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "HOME(Hold): Exit to Base";
    layer_move(0);
    th[TH_HOME].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_P].held && !th[TH_P].triggered && timer_elapsed(th[TH_P].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "P(Hold): Peek at Base";
    layer_state_set(0); // Peek at base layer
    th[TH_P].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_SLSH].held && !th[TH_SLSH].triggered && timer_elapsed(th[TH_SLSH].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "/(Hold): Peek at Base";
    layer_state_set(0); // Peek at base layer
    th[TH_SLSH].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_Q].held && !th[TH_Q].triggered && timer_elapsed(th[TH_Q].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "Q(Hold): Peek at One-Hand (L4)";
    layer_move(4); // Peek at layer 4
    th[TH_Q].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_L1].held && !th[TH_L1].triggered && timer_elapsed(th[TH_L1].timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 1) {
       layer_change_reason = "L_TG1(Hold): Toggle L1 (OFF)";
       layer_invert(1);
    } else {
       layer_change_reason = "L_TG1(Hold): Toggle L1 (ON)";
       layer_invert(1);
    }
    th[TH_L1].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_ENT_MO].held && !th[TH_ENT_MO].triggered &&
      timer_elapsed(th[TH_ENT_MO].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "ENT(Hold): MO(4)";
    layer_on(4);
    th[TH_ENT_MO].triggered = true;
    rgb_matrix_indicators_user();
  }

  if (th[TH_K1].held && !th[TH_K1].triggered &&
      timer_elapsed(th[TH_K1].timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 0) {
      layer_change_reason = "K1(Hold): Toggle L1 (ON)";
      layer_move(1);
    } else {
      layer_change_reason = "K1(Hold): Toggle L1 (OFF)";
      layer_move(0);
    }
    th[TH_K1].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_K2].held && !th[TH_K2].triggered &&
      timer_elapsed(th[TH_K2].timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 0) {
      layer_change_reason = "K2(Hold): Toggle L2 (ON)";
      layer_move(2);
    } else {
      layer_change_reason = "K2(Hold): Toggle L2 (OFF)";
      layer_move(0);
    }
    th[TH_K2].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_K3].held && !th[TH_K3].triggered &&
      timer_elapsed(th[TH_K3].timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 0) {
      layer_change_reason = "K3(Hold): Toggle L3 (ON)";
      layer_move(3);
    } else {
      layer_change_reason = "K3(Hold): Toggle L3 (OFF)";
      layer_move(0);
    }
    th[TH_K3].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_K4].held && !th[TH_K4].triggered &&
      timer_elapsed(th[TH_K4].timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 0) {
      layer_change_reason = "K4(Hold): Toggle L4 (ON)";
      layer_move(4);
    } else {
      layer_change_reason = "K4(Hold): Toggle L4 (OFF)";
      layer_move(0);
    }
    th[TH_K4].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_K5].held && !th[TH_K5].triggered &&
      timer_elapsed(th[TH_K5].timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 0) {
      layer_change_reason = "K5(Hold): Toggle L5 (ON)";
      layer_move(5);
    } else {
      layer_change_reason = "K5(Hold): Toggle L5 (OFF)";
      layer_move(0);
    }
    th[TH_K5].triggered = true;
    rgb_matrix_indicators_user();
  }

  // New Layer 4 Keys Logic
  if (th[TH_MINS].held && !th[TH_MINS].triggered &&
      timer_elapsed(th[TH_MINS].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "MINS(Hold): L4->Base";
    layer_move(0);
    th[TH_MINS].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_K0].held && !th[TH_K0].triggered &&
      timer_elapsed(th[TH_K0].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "K0(Hold): L4->L1 (Jump)";
    layer_move(1);
    th[TH_K0].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_K9].held && !th[TH_K9].triggered &&
      timer_elapsed(th[TH_K9].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "K9(Hold): L4->L2 (Jump)";
    layer_move(2);
    th[TH_K9].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_K8].held && !th[TH_K8].triggered &&
      timer_elapsed(th[TH_K8].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "K8(Hold): L4->L3 (Jump)";
    layer_move(3);
    th[TH_K8].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_K7].held && !th[TH_K7].triggered &&
      timer_elapsed(th[TH_K7].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "K7(Hold): L4->Base";
    layer_move(0);
    th[TH_K7].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_K6].held && !th[TH_K6].triggered &&
      timer_elapsed(th[TH_K6].timer) > MY_TAPPING_TERM) {
    layer_change_reason = "K6(Hold): L4->Base";
    layer_move(0);
    th[TH_K6].triggered = true;
    rgb_matrix_indicators_user();
  }

  // Thumb Toggle Logic
  if (th[TH_ENT_TG2].held && !th[TH_ENT_TG2].triggered &&
      timer_elapsed(th[TH_ENT_TG2].timer) > MY_TAPPING_TERM) {
    uprintf(">> Thumb Hold Triggered: ENT_TG2 -> Toggle Layer 2\n");
    if (get_highest_layer(layer_state) == 2) {
      layer_change_reason = "ENT(Hold): Toggle L2 (OFF)";
      layer_move(0);
    } else {
      layer_change_reason = "ENT(Hold): Toggle L2 (ON)";
      layer_move(2);
    }
    th[TH_ENT_TG2].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_ENT_TG4].held && !th[TH_ENT_TG4].triggered &&
      timer_elapsed(th[TH_ENT_TG4].timer) > MY_TAPPING_TERM) {
    uprintf(">> Thumb Hold Triggered: ENT_TG4 -> Toggle Layer 4\n");
    if (get_highest_layer(layer_state) == 4) {
      layer_change_reason = "ENT(Hold): Toggle L4 (OFF)";
      layer_move(0);
    } else {
      layer_change_reason = "ENT(Hold): Toggle L4 (ON)";
      layer_move(4);
    }
    th[TH_ENT_TG4].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_SPC_TG2].held && !th[TH_SPC_TG2].triggered &&
      timer_elapsed(th[TH_SPC_TG2].timer) > MY_TAPPING_TERM) {
    uprintf(">> Thumb Hold Triggered: SPC_TG2 -> Toggle Layer 2\n");
    if (get_highest_layer(layer_state) == 2) {
      layer_change_reason = "SPC(Hold): Toggle L2 (OFF)";
      layer_move(0);
    } else {
      layer_change_reason = "SPC(Hold): Toggle L2 (ON)";
      layer_move(2);
    }
    th[TH_SPC_TG2].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_SPC_TG4].held && !th[TH_SPC_TG4].triggered &&
      timer_elapsed(th[TH_SPC_TG4].timer) > MY_TAPPING_TERM) {
    uprintf(">> Thumb Hold Triggered: SPC_TG4 -> Toggle Layer 4\n");
    if (get_highest_layer(layer_state) == 4) {
      layer_change_reason = "SPC(Hold): Toggle L4 (OFF)";
      layer_move(0);
    } else {
      layer_change_reason = "SPC(Hold): Toggle L4 (ON)";
      layer_move(4);
    }
    th[TH_SPC_TG4].triggered = true;
    rgb_matrix_indicators_user();
  }
  if (th[TH_F12].held && !th[TH_F12].triggered &&
      timer_elapsed(th[TH_F12].timer) > MY_TAPPING_TERM) {
    uprintf(">> Thumb Hold Triggered: F12 -> Exit to Base\n");
    layer_change_reason = "F12(Hold): Exit to Base";
    layer_move(0);
    th[TH_F12].triggered = true;
    rgb_matrix_indicators_user();
  }
}
