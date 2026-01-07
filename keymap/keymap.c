#include QMK_KEYBOARD_H
#include "transactions.h"

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
  KC_PMNS_TG4,
  KC_MINS_TG4,
  KC_F12_EXIT,
  KC_FIRE,
  KC_SNIPE,
  KC_MS_TMO_INC,
  KC_MS_TMO_DEC,
  KC_SET_LEFT,
  KC_SET_RIGHT
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
static bool mouse_is_locked = false;
static uint8_t saved_rgb_mode;
static uint8_t saved_rgb_h, saved_rgb_s, saved_rgb_v;
static uint8_t automatic_hue_tracker = 0;
static bool rgb_auto_cycle = false;
static uint16_t rgb_auto_timer = 0;
static uint16_t auto_mouse_timeout = 2000; // Default 2 seconds, adjustable
static uint16_t auto_mouse_timer = 0;
static bool auto_mouse_on = false;

// Handedness reporting state
static uint8_t handedness_print_count = 0;
static uint32_t handedness_print_timer = 0;

// Show mode state
static bool show_mode_active = false;
static uint8_t show_mode_digits[2];
static uint8_t show_mode_digit_count = 0;
static uint8_t show_mode_current_digit = 0;
static uint16_t show_mode_timer = 0;
static uint8_t show_mode_phase = 0; // 0=off, 1=on

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
  bool mouse_is_locked;
  bool show_mode_active;
  bool is_caps_lock_on;
  uint8_t show_mode_digits[2];
  uint8_t show_mode_digit_count;
  uint8_t show_mode_current_digit;
  uint8_t show_mode_phase;
  uint8_t rgb_mode;
  bool is_left_hand;
} user_sync_info_t;

static bool is_caps_lock_on = false;

typedef struct _user_sync_info_response_t {
  bool did_rgb_sync;
} user_sync_info_response_t;

static bool sync_needed = false;
static bool slave_first_sync = true;
static bool master_rgb_init_pending = false;
static uint8_t master_rgb_init_mode = 0;

void user_sync_info_slave_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
  const user_sync_info_t *sync_data = (const user_sync_info_t *)in_data;
  user_sync_info_response_t *response = (user_sync_info_response_t *)out_data;

  is_flashlight = sync_data->is_flashlight;
  is_sniping_active = sync_data->is_sniping_active;
  mouse_is_locked = sync_data->mouse_is_locked;
  show_mode_active = sync_data->show_mode_active;
  is_caps_lock_on = sync_data->is_caps_lock_on;
  show_mode_digits[0] = sync_data->show_mode_digits[0];
  show_mode_digits[1] = sync_data->show_mode_digits[1];
  show_mode_digit_count = sync_data->show_mode_digit_count;
  show_mode_current_digit = sync_data->show_mode_current_digit;
  show_mode_phase = sync_data->show_mode_phase;

  // Send slave's handedness back to master via response (not used currently, just stored locally)

  bool synced = false;
  uint8_t current_mode = rgb_matrix_get_mode();
  if (current_mode != sync_data->rgb_mode || slave_first_sync) {
    uprintf("\033[93mSlave: Syncing RGB %d -> %d%s\033[0m\n", current_mode, sync_data->rgb_mode, slave_first_sync ? " (first sync)" : "");
    rgb_matrix_mode_noeeprom(sync_data->rgb_mode);
    slave_first_sync = false;
    // Verify the mode was applied
    uint8_t new_mode = rgb_matrix_get_mode();
    if (new_mode != sync_data->rgb_mode) {
      uprintf("\033[91mSlave: RGB sync FAILED! Expected %d, got %d\033[0m\n", sync_data->rgb_mode, new_mode);
    }
    synced = true;
  }

  if (out_buflen >= sizeof(user_sync_info_response_t)) {
    response->did_rgb_sync = synced;
  }
}

// Helper to get mode name
const char *get_rgb_mode_name(uint8_t mode) {
  switch (mode) {
  case RGB_MATRIX_SOLID_COLOR:
    return "SOLID_COLOR";
  case RGB_MATRIX_ALPHAS_MODS:
    return "ALPHA_MODS";
  case RGB_MATRIX_GRADIENT_UP_DOWN:
    return "GRADIENT_UP_DOWN";
  case RGB_MATRIX_GRADIENT_LEFT_RIGHT:
    return "GRADIENT_LEFT_RIGHT";
  case RGB_MATRIX_BREATHING:
    return "BREATHING";
  case RGB_MATRIX_BAND_SAT:
    return "COLORBAND_SAT";
  case RGB_MATRIX_BAND_VAL:
    return "COLORBAND_VAL";
  case RGB_MATRIX_BAND_PINWHEEL_SAT:
    return "COLORBAND_PINWHEEL_SAT";
  case RGB_MATRIX_BAND_PINWHEEL_VAL:
    return "COLORBAND_PINWHEEL_VAL";
  case RGB_MATRIX_BAND_SPIRAL_SAT:
    return "COLORBAND_SPIRAL_SAT";
  case RGB_MATRIX_BAND_SPIRAL_VAL:
    return "COLORBAND_SPIRAL_VAL";
  case RGB_MATRIX_CYCLE_ALL:
    return "CYCLE_ALL";
  case RGB_MATRIX_CYCLE_LEFT_RIGHT:
    return "CYCLE_LEFT_RIGHT";
  case RGB_MATRIX_CYCLE_UP_DOWN:
    return "CYCLE_UP_DOWN";
  case RGB_MATRIX_CYCLE_OUT_IN:
    return "CYCLE_OUT_IN";
  case RGB_MATRIX_CYCLE_OUT_IN_DUAL:
    return "CYCLE_OUT_IN_DUAL";
  case RGB_MATRIX_RAINBOW_MOVING_CHEVRON:
    return "RAINBOW_MOVING_CHEVRON";
  case RGB_MATRIX_CYCLE_PINWHEEL:
    return "CYCLE_PINWHEEL";
  case RGB_MATRIX_CYCLE_SPIRAL:
    return "CYCLE_SPIRAL";
  case RGB_MATRIX_DUAL_BEACON:
    return "DUAL_BEACON";
  case RGB_MATRIX_RAINBOW_BEACON:
    return "RAINBOW_BEACON";
  case RGB_MATRIX_RAINBOW_PINWHEELS:
    return "RAINBOW_PINWHEELS";
  case RGB_MATRIX_RAINDROPS:
    return "RAINDROPS";
  case RGB_MATRIX_JELLYBEAN_RAINDROPS:
    return "JELLYBEAN_RAINDROPS";
  case RGB_MATRIX_HUE_BREATHING:
    return "HUE_BREATHING";
  case RGB_MATRIX_HUE_PENDULUM:
    return "HUE_PENDULUM";
  case RGB_MATRIX_HUE_WAVE:
    return "HUE_WAVE";
  case RGB_MATRIX_PIXEL_FRACTAL:
    return "PIXEL_FRACTAL";
  case RGB_MATRIX_PIXEL_FLOW:
    return "PIXEL_FLOW";
  case RGB_MATRIX_PIXEL_RAIN:
    return "PIXEL_RAIN";
  case RGB_MATRIX_TYPING_HEATMAP:
    return "TYPING_HEATMAP";
  case RGB_MATRIX_DIGITAL_RAIN:
    return "DIGITAL_RAIN";
  case RGB_MATRIX_SOLID_REACTIVE_SIMPLE:
    return "SOLID_REACTIVE_SIMPLE";
  case RGB_MATRIX_SOLID_REACTIVE:
    return "SOLID_REACTIVE";
  case RGB_MATRIX_SOLID_REACTIVE_WIDE:
    return "SOLID_REACTIVE_WIDE";
  case RGB_MATRIX_SOLID_REACTIVE_CROSS:
    return "SOLID_REACTIVE_CROSS";
  case RGB_MATRIX_SOLID_REACTIVE_MULTICROSS:
    return "SOLID_REACTIVE_MULTICROSS";
  case RGB_MATRIX_SOLID_REACTIVE_NEXUS:
    return "SOLID_REACTIVE_NEXUS";
  case RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS:
    return "SOLID_REACTIVE_MULTINEXUS";
  case RGB_MATRIX_SPLASH:
    return "SPLASH";
  case RGB_MATRIX_MULTISPLASH:
    return "MULTISPLASH";
  case RGB_MATRIX_SOLID_SPLASH:
    return "SOLID_SPLASH";
  case RGB_MATRIX_SOLID_MULTISPLASH:
    return "SOLID_MULTISPLASH";
  case RGB_MATRIX_CUSTOM_fire:
    return "FIRE";
  default:
    return "UNKNOWN";
  }
}

void start_show_mode(void) {
  uint8_t mode = rgb_matrix_get_mode();
  uprintf("\033[93mRGB Mode Changed: %d (%s)\033[0m\n", mode, get_rgb_mode_name(mode));

  // If entering Hue Breathing, pick a random base hue
  if (mode == RGB_MATRIX_HUE_BREATHING) {
    uint8_t random_hue = timer_read() % 256;
    rgb_matrix_sethsv_noeeprom(random_hue, rgb_matrix_get_sat(),
                               rgb_matrix_get_val());
  } else if (mode == RGB_MATRIX_SOLID_COLOR || mode == RGB_MATRIX_BREATHING) {
    // Increment hue for Solid Color and Breathing modes every time they activate
    automatic_hue_tracker += 42;
    rgb_matrix_sethsv_noeeprom(automatic_hue_tracker, 255, 255);
    uprintf("Solid/Breathing Activated: Mode=%d (SOLID=%d, BREATHING=%d) Hue=%d\n",
            mode, RGB_MATRIX_SOLID_COLOR, RGB_MATRIX_BREATHING, automatic_hue_tracker);
  }

  show_mode_digit_count = 0;

  // Extract digits (handle 1-2 digit numbers)
  if (mode >= 10) {
    show_mode_digits[show_mode_digit_count++] = mode / 10;
  }
  show_mode_digits[show_mode_digit_count++] = mode % 10;

  show_mode_current_digit = 0;
  show_mode_phase = 1; // Start with flash on
  show_mode_timer = timer_read();
  show_mode_active = true;
  sync_needed = true;
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
      layer_move(0);
    } else {
      layer_move(4);
    }
    rgb_matrix_indicators_user();
    break;
  case DOUBLE_TAP:
    if (!is_flashlight) {
      saved_rgb_mode = rgb_matrix_get_mode();
      saved_rgb_h = rgb_matrix_get_hue();
      saved_rgb_s = rgb_matrix_get_sat();
      saved_rgb_v = rgb_matrix_get_val();
      rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
      rgb_matrix_sethsv_noeeprom(HSV_WHITE);
      is_flashlight = true;
    } else {
      rgb_matrix_mode_noeeprom(saved_rgb_mode);
      rgb_matrix_sethsv_noeeprom(saved_rgb_h, saved_rgb_s, saved_rgb_v);
      is_flashlight = false;
    }
    sync_needed = true;
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
const uint16_t PROGMEM right_combo[] = {KC_F, KC_G, COMBO_END};
const uint16_t PROGMEM af_combo[] = {KC_A, KC_F, COMBO_END};
const uint16_t PROGMEM delete_combo[] = {KC_J, KC_K, COMBO_END};
const uint16_t PROGMEM home_combo[] = {TD(TD_Z_LAYER), KC_X, COMBO_END};
const uint16_t PROGMEM pgup_combo[] = {KC_X, KC_C, COMBO_END};
const uint16_t PROGMEM pgdn_combo[] = {KC_C, KC_V, COMBO_END};
const uint16_t PROGMEM xv_combo[] = {KC_X, KC_V, COMBO_END};
const uint16_t PROGMEM end_combo[] = {KC_V, KC_B, COMBO_END};
const uint16_t PROGMEM zv_combo[] = {TD(TD_Z_LAYER), KC_V, COMBO_END};
const uint16_t PROGMEM caps_combo[] = {KC_LSFT, KC_RSFT, COMBO_END};

combo_t key_combos[] = {
    COMBO(left_combo, KC_LEFT),   COMBO(up_combo, KC_UP),
    COMBO(down_combo, KC_DOWN),   COMBO(right_combo, KC_RIGHT),
    COMBO(af_combo, KC_RIGHT),    COMBO(delete_combo, KC_DEL),
    COMBO(home_combo, KC_HOME),   COMBO(pgup_combo, KC_PGUP),
    COMBO(pgdn_combo, KC_PGDN),   COMBO(xv_combo, C(S(KC_V))),
    COMBO(end_combo, KC_END),     COMBO(zv_combo, KC_END),
    COMBO(caps_combo, KC_CAPS),
};

static uint16_t pgup_tap_timer = 0;
static bool pgup_held = false;
static bool pgup_triggered = false;
static uint16_t p_tap_timer = 0;
static bool p_held = false;
static bool p_triggered = false;
static uint16_t home_tap_timer = 0;
static bool home_held = false;
static bool home_triggered = false;
static uint16_t ent_mo_timer = 0;
static bool ent_mo_held = false;
static bool ent_mo_triggered = false;
static layer_state_t layer_state_to_restore = 0;

static uint16_t k1_tap_timer = 0;
static bool k1_held = false;
static bool k1_triggered = false;
static uint16_t k2_tap_timer = 0;
static bool k2_held = false;
static bool k2_triggered = false;
static uint16_t k3_tap_timer = 0;
static bool k3_held = false;
static bool k3_triggered = false;
static uint16_t k4_tap_timer = 0;
static bool k4_held = false;
static bool k4_triggered = false;

// Variables for new Layer 4 keys
static uint16_t mins_tap_timer = 0;
static bool mins_held = false;
static bool mins_triggered = false;
static uint16_t k0_tap_timer = 0;
static bool k0_held = false;
static bool k0_triggered = false;
static uint16_t k9_tap_timer = 0;
static bool k9_held = false;
static bool k9_triggered = false;
static uint16_t k8_tap_timer = 0;
static bool k8_held = false;
static bool k8_triggered = false;
static uint16_t k7_tap_timer = 0;
static bool k7_held = false;
static bool k7_triggered = false;
static uint16_t k6_tap_timer = 0;
static bool k6_held = false;
static bool k6_triggered = false;

// Variables for Thumb Toggle Keys
static uint16_t ent_tg4_timer = 0;
static bool ent_tg4_held = false;
static bool ent_tg4_triggered = false;
static uint16_t ent_tg2_timer = 0;
static bool ent_tg2_held = false;
static bool ent_tg2_triggered = false;
static uint16_t spc_tg2_timer = 0;
static bool spc_tg2_held = false;
static bool spc_tg2_triggered = false;
static uint16_t pmns_tg4_timer = 0;
static bool pmns_tg4_held = false;
static bool pmns_tg4_triggered = false;

static uint16_t f12_tap_timer = 0;
static bool f12_held = false;
static bool f12_triggered = false;

#define MY_TAPPING_TERM 225

bool is_fast_mouse = false;
bool is_scroll_mode = false;

static uint32_t last_key_time = 0;

layer_state_t layer_state_set_user(layer_state_t state) {
  uint32_t now = timer_read32();
  uint32_t sec = now / 1000;
  uint32_t ms = now % 1000;

  uint8_t layer = get_highest_layer(state);
  uint8_t prev_layer = get_highest_layer(layer_state);

  uprintf("[%lu.%03lu] Layer change: %u -> %u (state=%lu)\n", sec, ms,
          prev_layer, layer, (unsigned long)state);
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

  switch (keycode) {
  case KC_EXIT:
    if (record->event.pressed) {
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
  case KC_MS_FAST_UP:
    if (record->event.pressed) {
      tap_code(MS_ACL2); // Speed up
      register_code(MS_UP);
    } else {
      unregister_code(MS_UP);
      tap_code(MS_ACL0); // Reset speed
    }
    return false;
  case KC_MS_FAST_DOWN:
    if (record->event.pressed) {
      tap_code(MS_ACL2);
      register_code(MS_DOWN);
    } else {
      unregister_code(MS_DOWN);
      tap_code(MS_ACL0);
    }
    return false;
  case KC_MS_FAST_LEFT:
    if (record->event.pressed) {
      tap_code(MS_ACL2);
      register_code(MS_LEFT);
    } else {
      unregister_code(MS_LEFT);
      tap_code(MS_ACL0);
    }
    return false;
  case KC_MS_FAST_RIGHT:
    if (record->event.pressed) {
      tap_code(MS_ACL2);
      register_code(MS_RGHT);
    } else {
      unregister_code(MS_RGHT);
      tap_code(MS_ACL0);
    }
    return false;
  case KC_MS_DIAG_UL:
    if (record->event.pressed) {
      register_code(MS_UP);
      register_code(MS_LEFT);
    } else {
      unregister_code(MS_UP);
      unregister_code(MS_LEFT);
    }
    return false;
  case KC_MS_DIAG_UR:
    if (record->event.pressed) {
      register_code(MS_UP);
      register_code(MS_RGHT);
    } else {
      unregister_code(MS_UP);
      unregister_code(MS_RGHT);
    }
    return false;
  case KC_MS_DIAG_DL:
    if (record->event.pressed) {
      register_code(MS_DOWN);
      register_code(MS_LEFT);
    } else {
      unregister_code(MS_DOWN);
      unregister_code(MS_LEFT);
    }
    return false;
  case KC_MS_DIAG_DR:
    if (record->event.pressed) {
      register_code(MS_DOWN);
      register_code(MS_RGHT);
    } else {
      unregister_code(MS_DOWN);
      unregister_code(MS_RGHT);
    }
    return false;
  case KC_SCR_MODE:
    if (record->event.pressed) {
      is_scroll_mode = true;
    } else {
      is_scroll_mode = false;
    }
    return false;
  case KC_ENT_MO4:
    if (record->event.pressed) {
      ent_mo_held = true;
      ent_mo_triggered = false;
      ent_mo_timer = timer_read();
    } else {
      ent_mo_held = false;
      if (!ent_mo_triggered) {
        tap_code(KC_ENT);
      } else {
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
      layer_state_set(0); // Clear all layers (peek at base)
      rgb_matrix_indicators_user();
      ent_mo_timer = timer_read();
    } else {
      uprintf("KC_ENT_EXIT Released. Elapsed: %u. Action: %s\n",
              timer_elapsed(ent_mo_timer),
              (timer_elapsed(ent_mo_timer) < MY_TAPPING_TERM) ? "Tap (Exit)"
                                                              : "Hold (Exit)");
      if (timer_elapsed(ent_mo_timer) < MY_TAPPING_TERM) {
        tap_code(KC_ENT);
        // Tap = Permanent Exit. We stay at layer 0.
      } else {
        // Hold = Momentary Peek. Restore the full layer state.
        // layer_state_set(layer_state_to_restore);
      }
      rgb_matrix_indicators_user();
    }
    return false;
  case KC_SPC_EXIT:
    if (record->event.pressed) {
      layer_state_to_restore = layer_state;
      layer_state_set(0);
      rgb_matrix_indicators_user();
      ent_mo_timer = timer_read();
    } else {
      if (timer_elapsed(ent_mo_timer) < MY_TAPPING_TERM) {
        tap_code(KC_SPC);
      } else {
        // layer_state_set(layer_state_to_restore);
      }
      rgb_matrix_indicators_user();
    }
    return false;
  case KC_BSPC_EXIT:
    if (record->event.pressed) {
      layer_state_to_restore = layer_state;
      layer_state_set(0);
      rgb_matrix_indicators_user();
      ent_mo_timer = timer_read();
    } else {
      if (timer_elapsed(ent_mo_timer) < MY_TAPPING_TERM) {
        tap_code(KC_BSPC);
      } else {
        // layer_state_set(layer_state_to_restore);
      }
      rgb_matrix_indicators_user();
    }
    return false;
  case KC_L_TG1:
    if (record->event.pressed) {
      if (get_highest_layer(layer_state) > 0) {
        layer_move(0);
      } else {
        layer_invert(1);
      }
      rgb_matrix_indicators_user();
    }
    return false;
  case KC_R_TG2:
    if (record->event.pressed) {
      if (get_highest_layer(layer_state) > 0) {
        layer_move(0);
      } else {
        layer_invert(2);
      }
      rgb_matrix_indicators_user();
    }
    return false;
  case KC_P_TO0:
    if (record->event.pressed) {
      p_held = true;
      p_triggered = false;
      p_tap_timer = timer_read();
    } else {
      p_held = false;
      if (!p_triggered) {
        tap_code(KC_P);
      }
    }
    return false;
  case KC_PGUP_TO0:
    if (record->event.pressed) {
      pgup_held = true;
      pgup_triggered = false;
      pgup_tap_timer = timer_read();
    } else {
      pgup_held = false;
      if (!pgup_triggered) {
        tap_code(KC_PGUP);
      }
    }
    return false;
  case KC_HOME_TO0:
    if (record->event.pressed) {
      home_held = true;
      home_triggered = false;
      home_tap_timer = timer_read();
    } else {
      home_held = false;
      if (!home_triggered) {
        tap_code(KC_HOME);
      }
    }
    return false;
  case KC_RAINBOW:
    if (record->event.pressed) {
      rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_LEFT_RIGHT);
      start_show_mode();
    }
    return false;
  case RM_NEXT:
    if (record->event.pressed) {
      rgb_matrix_step_noeeprom();
      start_show_mode();
    }
    return false;
  case RM_PREV:
    if (record->event.pressed) {
      rgb_matrix_step_reverse_noeeprom();
      start_show_mode();
    }
    return false;
  case KC_REACTIVE:
    if (record->event.pressed) {
      rgb_matrix_mode_noeeprom(RGB_MATRIX_SPLASH);
      start_show_mode();
    }
    return false;
  case KC_MOUSE_LOCK:
    if (record->event.pressed) {
      mouse_is_locked = !mouse_is_locked;
      sync_needed = true;
      if (mouse_is_locked) {
        layer_on(3);
      } else {
        layer_off(3);
      }
    }
    return false;
  case KC_SNIPE:
    if (record->event.pressed) {
      is_sniping_active = !is_sniping_active;
      sync_needed = true;
      if (is_sniping_active) {
        pointing_device_set_cpi(250);
        uprintf("[%lu.%03lu] Snipe ON. CPI -> 250\n", sec, ms);
      } else {
        pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
        uprintf("[%lu.%03lu] Snipe OFF. CPI -> Default (%u)\n", sec, ms,
                pointing_device_get_cpi());
      }
    }
    return false;
  case KC_ENT_L2_EXIT:
    if (record->event.pressed) {
      uprintf("KC_ENT_L2_EXIT Pressed. Exiting Layer 2.\n");
      layer_off(2);
      tap_code(KC_ENT);
    }
    return false;
  case KC_1_TG1:
    if (record->event.pressed) {
      k1_held = true;
      k1_triggered = false;
      k1_tap_timer = timer_read();
    } else {
      k1_held = false;
      if (!k1_triggered) {
        uint8_t layer = get_highest_layer(layer_state);
        if (layer == 2)
          tap_code(KC_F1);
        else if (layer == 3) {
          rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_LEFT_RIGHT);
          start_show_mode();
        } else if (layer == 4)
          tap_code(KC_0);
        else
          tap_code(KC_1);
      }
    }
    return false;
  case KC_2_TG2:
    if (record->event.pressed) {
      k2_held = true;
      k2_triggered = false;
      k2_tap_timer = timer_read();
    } else {
      k2_held = false;
      if (!k2_triggered) {
        uint8_t layer = get_highest_layer(layer_state);
        if (layer == 2)
          tap_code(KC_F2);
        else if (layer == 3) {
          rgb_matrix_step_noeeprom();
          start_show_mode();
        } else if (layer == 4)
          tap_code(KC_9);
        else
          tap_code(KC_2);
      }
    }
    return false;
  case KC_3_TG3:
    if (record->event.pressed) {
      k3_held = true;
      k3_triggered = false;
      k3_tap_timer = timer_read();
    } else {
      k3_held = false;
      if (!k3_triggered) {
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
      k4_held = true;
      k4_triggered = false;
      k4_tap_timer = timer_read();
    } else {
      k4_held = false;
      if (!k4_triggered) {
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
  case KC_JELLY:
    if (record->event.pressed) {
      rgb_matrix_mode_noeeprom(RGB_MATRIX_JELLYBEAN_RAINDROPS);
      start_show_mode();
    }
    return false;
  case KC_SPIRAL:
    if (record->event.pressed) {
      rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_SPIRAL);
      start_show_mode();
    }
    return false;
  case KC_CHEVRON:
    if (record->event.pressed) {
      rgb_matrix_mode_noeeprom(RGB_MATRIX_RAINBOW_MOVING_CHEVRON);
      start_show_mode();
    }
    return false;
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
        rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
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
  case KC_MINS_TO0:
    if (record->event.pressed) {
      mins_held = true;
      mins_triggered = false;
      mins_tap_timer = timer_read();
    } else {
      mins_held = false;
      if (!mins_triggered) {
        tap_code(KC_MINS);
      }
    }
    return false;
  case KC_0_TG1:
    if (record->event.pressed) {
      k0_held = true;
      k0_triggered = false;
      k0_tap_timer = timer_read();
    } else {
      k0_held = false;
      if (!k0_triggered) {
        tap_code(KC_0);
      }
    }
    return false;
  case KC_9_TG2:
    if (record->event.pressed) {
      k9_held = true;
      k9_triggered = false;
      k9_tap_timer = timer_read();
    } else {
      k9_held = false;
      if (!k9_triggered) {
        tap_code(KC_9);
      }
    }
    return false;
  case KC_8_TG3:
    if (record->event.pressed) {
      k8_held = true;
      k8_triggered = false;
      k8_tap_timer = timer_read();
    } else {
      k8_held = false;
      if (!k8_triggered) {
        tap_code(KC_8);
      }
    }
    return false;
  case KC_7_TO0:
    if (record->event.pressed) {
      k7_held = true;
      k7_triggered = false;
      k7_tap_timer = timer_read();
    } else {
      k7_held = false;
      if (!k7_triggered) {
        tap_code(KC_7);
      }
    }
    return false;
  case KC_6_TO0:
    if (record->event.pressed) {
      k6_held = true;
      k6_triggered = false;
      k6_tap_timer = timer_read();
    } else {
      k6_held = false;
      if (!k6_triggered) {
        tap_code(KC_6);
      }
    }
    return false;
  case KC_ENT_TG2:
    if (record->event.pressed) {
      uint32_t now = timer_read32();
      uint32_t sec = now / 1000;
      uint32_t ms = now % 1000;
      uprintf("[%lu.%03lu] KC_ENT_TG2 Pressed\n", sec, ms);
      ent_tg2_held = true;
      ent_tg2_triggered = false;
      ent_tg2_timer = timer_read();
    } else {
      uint32_t now = timer_read32();
      uint32_t sec = now / 1000;
      uint32_t ms = now % 1000;
      uprintf("[%lu.%03lu] KC_ENT_TG2 Released. Duration: %u ms. Held: %d, "
              "Triggered: %d. Action: %s\n",
              sec, ms, timer_elapsed(ent_tg2_timer), ent_tg2_held,
              ent_tg2_triggered,
              (!ent_tg2_triggered) ? "Tap (Enter)" : "None (Handled by Timer)");
      ent_tg2_held = false;
      if (!ent_tg2_triggered) {
        tap_code(KC_ENT);
      }
    }
    return false;
  case KC_ENT_TG4:
    if (record->event.pressed) {
      uint32_t now = timer_read32();
      uint32_t sec = now / 1000;
      uint32_t ms = now % 1000;
      uprintf("[%lu.%03lu] KC_ENT_TG4 Pressed\n", sec, ms);
      ent_tg4_held = true;
      ent_tg4_triggered = false;
      ent_tg4_timer = timer_read();
    } else {
      uint32_t now = timer_read32();
      uint32_t sec = now / 1000;
      uint32_t ms = now % 1000;
      uprintf("[%lu.%03lu] KC_ENT_TG4 Released. Duration: %u ms. Held: %d, "
              "Triggered: %d. Action: %s\n",
              sec, ms, timer_elapsed(ent_tg4_timer), ent_tg4_held,
              ent_tg4_triggered,
              (!ent_tg4_triggered) ? "Tap (Enter)" : "None (Handled by Timer)");
      ent_tg4_held = false;
      if (!ent_tg4_triggered) {
        tap_code(KC_ENT);
      }
    }
    return false;
  case KC_SPC_TG2:
    if (record->event.pressed) {
      uint32_t now = timer_read32();
      uint32_t sec = now / 1000;
      uint32_t ms = now % 1000;
      uprintf("[%lu.%03lu] KC_SPC_TG2 Pressed\n", sec, ms);
      spc_tg2_held = true;
      spc_tg2_triggered = false;
      spc_tg2_timer = timer_read();
    } else {
      uint32_t now = timer_read32();
      uint32_t sec = now / 1000;
      uint32_t ms = now % 1000;
      uprintf("[%lu.%03lu] KC_SPC_TG2 Released. Duration: %u ms. Held: %d, "
              "Triggered: %d. Action: %s\n",
              sec, ms, timer_elapsed(spc_tg2_timer), spc_tg2_held,
              spc_tg2_triggered,
              (!spc_tg2_triggered) ? "Tap (Space)" : "None (Handled by Timer)");
      spc_tg2_held = false;
      if (!spc_tg2_triggered) {
        tap_code(KC_SPC);
      }
    }
    return false;
  case KC_PMNS_TG4:
    if (record->event.pressed) {
      pmns_tg4_held = true;
      pmns_tg4_triggered = false;
      pmns_tg4_timer = timer_read();
    } else {
      pmns_tg4_held = false;
      if (!pmns_tg4_triggered) {
        tap_code(KC_PMNS);
      }
    }
    return false;
  case KC_MINS_TG4:
    if (record->event.pressed) {
      pmns_tg4_held = true;
      pmns_tg4_triggered = false;
      pmns_tg4_timer = timer_read();
    } else {
      pmns_tg4_held = false;
      if (!pmns_tg4_triggered) {
        tap_code(KC_MINS);
      }
    }
    return false;
  case KC_F12_EXIT:
    if (record->event.pressed) {
      f12_held = true;
      f12_triggered = false;
      f12_tap_timer = timer_read();
    } else {
      f12_held = false;
      if (!f12_triggered) {
        tap_code(KC_F12);
      }
    }
    return false;
  case KC_FIRE:
    if (record->event.pressed) {
      rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_fire);
      start_show_mode();
    }
    return false;
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
        LAYOUT(QK_GESC, KC_1_TG1, KC_2_TG2, KC_3_TG3, KC_4_TG4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS,
               KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,
               KC_LSFT, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_PLUS_COLON, KC_QUOT,
               KC_LCTL, TD(TD_Z_LAYER), KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, LT(3, KC_SLSH), KC_RSFT,
               KC_SPC_TG2, KC_ENT_TG4, KC_L_TG1, KC_DEL, KC_ENT_TG2, KC_LALT, KC_BSPC, KC_BSPC),
               //QK_BOOT, KC_PSCR  , KC_SET_LEFT, KC_SET_RIGHT, KC_EXIT, KC_EXIT, KC_SET_LEFT, KC_SET_RIGHT, KC_EXIT, KC_EXIT   , KC_PSCR   , QK_BOOT,
    [1] = LAYOUT(KC_PSCR, KC_EXIT, TO(2), TO(3), TO(4), KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT   , KC_PSCR   , QK_BOOT,
                 KC_TAB , KC_MINS  , KC_7   , KC_8   , KC_9   , S(KC_8), RM_NEXT, KC_LBRC, KC_RBRC, S(KC_LBRC), S(KC_RBRC), KC_TRNS,
                 KC_EXIT, S(KC_EQL), KC_4   , KC_5   , KC_6   , KC_SLSH, RM_PREV, KC_LEFT, KC_UP  , KC_DOWN   , KC_RGHT   , KC_EQL,
                 KC_LCTL, KC_0, KC_1, KC_2, KC_3, KC_EQL,
                 RM_HUEU, RM_HUED, RM_SATU, RM_SATD, RM_VALU, RM_VALD,
                 KC_SPC_EXIT, KC_ENT_EXIT, KC_L_TG1, KC_R_TG2, KC_ENT_EXIT,
                 KC_LALT, KC_BSPC_EXIT, KC_BSPC_EXIT),
    [2] = LAYOUT(QK_BOOT, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, QK_BOOT,
                 KC_PSCR, KC_LBRC, KC_RBRC, S(KC_LBRC), S(KC_RBRC), KC_EXIT, KC_FIRE, KC_LBRC, KC_RBRC, S(KC_LBRC), S(KC_RBRC), KC_F11,
                 KC_LSFT, KC_LEFT, KC_UP, KC_DOWN, KC_RGHT, KC_EXIT, KC_RSFT, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_F12,
                 KC_EXIT, LT(3, KC_HOME), KC_PGUP, KC_PGDN, KC_END, KC_EXIT, KC_RGB_AUTO, KC_HOME, KC_PGUP, KC_PGDN, KC_END, KC_NO,
                 KC_SPC_EXIT, KC_ENT_EXIT, KC_L_TG1, KC_R_TG2, KC_ENT_EXIT,
                 KC_DEL, KC_BSPC_EXIT, KC_BSPC_EXIT),
    [3] = LAYOUT(QK_GESC, QK_CLEAR_EEPROM, KC_MS_FAST_UP, KC_3_TG3     , KC_4_TG4, QK_BOOT      , KC_EXIT, KC_RCTL, KC_RALT, KC_RGUI, QK_CLEAR_EEPROM, QK_BOOT,
                 KC_EXIT, KC_MS_DIAG_UL  , MS_UP        , KC_MS_DIAG_UR, KC_EXIT , KC_MS_TMO_INC, KC_EXIT, DPI_MOD, DPI_RMOD, KC_SNIPE, KC_EXIT, KC_TRNS,
                 KC_MS_FAST_LEFT, MS_LEFT, MS_BTN1  , MS_RGHT  , KC_MS_FAST_RIGHT, KC_MS_TMO_DEC, KC_EXIT, MS_BTN1, DRGSCRL, KC_MOUSE_LOCK, MS_BTN2, KC_EXIT,
                 KC_EXIT, KC_MS_DIAG_DL  , MS_DOWN , KC_MS_DIAG_DR,      KC_EXIT , KC_EXIT      , KC_EXIT, KC_EXIT, MS_BTN3, MS_BTN3, MS_BTN3, KC_RSFT,
                 MS_BTN1, MS_BTN2, MS_BTN3,
                 KC_EXIT, MS_BTN1, KC_EXIT, KC_EXIT, MS_BTN2),
    [4] = LAYOUT(KC_MINS_TO0, KC_0_TG1, KC_9_TG2, KC_8_TG3, KC_7_TO0, KC_6_TO0,
                 KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_BSLS, KC_P_TO0, KC_O,
                 KC_I, KC_U, KC_Y, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,
                 KC_QUOT, KC_PLUS_COLON, KC_L, KC_K, KC_J, KC_H, KC_H, KC_J,
                 KC_K, KC_L, KC_PLUS_COLON, KC_QUOT,
                 KC_LCTL, LT(3, KC_SLSH), KC_DOT, KC_COMM, KC_M, KC_N, KC_N, KC_M, KC_COMM, KC_DOT, LT(3, KC_SLSH), KC_RCTL,
                 KC_SPC, KC_ENT_EXIT, KC_LSFT,
                 KC_R_TG2, KC_ENT_EXIT,
                 KC_LALT, KC_BSPC, KC_BSPC),
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
    if (is_keyboard_left()) {
      static const uint8_t left[] = {6,  9,  14, 17, 21, 5,  10, 13,
                                     18, 22, 4,  11, 12, 19, 23};
      for (int i = 0; i < sizeof(left); i++)
        rgb_matrix_set_color(left[i], 0, 0, 255);
    }
    if (!is_keyboard_left()) {
      static const uint8_t right[] = {22, 2};
      for (int i = 0; i < sizeof(right); i++)
        rgb_matrix_set_color(right[i], 0, 0, 255);
    }
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
    // Left Side (Pink)
    if (is_keyboard_left()) {
      static const uint8_t left[] = {0,  7,  8,  15, 16, 20, 1,  6,  9, 14,
                                     17, 21, 2,  5,  10, 13, 18, 22, 3, 4,
                                     11, 12, 19, 23, 26, 27, 28, 25, 24};
      for (int i = 0; i < sizeof(left); i++)
        rgb_matrix_set_color(left[i], 255, 0, 255);
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

  // Flash number key logic for show_mode (Master Only) would go here
  if (show_mode_active) {
    uint8_t digit = show_mode_digits[show_mode_current_digit];
    uint8_t led_index =
        (digit == 0) ? number_key_leds[9] : number_key_leds[digit - 1];
    uint8_t letter_index =
        (digit == 0) ? letter_key_leds[9] : letter_key_leds[digit - 1];
    bool am_i_left = is_keyboard_left();
    uint8_t val = (show_mode_phase == 1) ? 255 : 0;
    // Local addressing fix:
    if (am_i_left) {
      if (led_index < 29)
        rgb_matrix_set_color(led_index, val, val, val);
      if (letter_index < 29)
        rgb_matrix_set_color(letter_index, val, val, val);
    } else {
      if (led_index >= 29)
        rgb_matrix_set_color(led_index - 29, val, val, val);
      if (letter_index >= 29)
        rgb_matrix_set_color(letter_index - 29, val, val, val);
    }
  }
  return false;
}

void keyboard_post_init_user(void) {
  transaction_register_rpc(USER_SYNC_INFO, user_sync_info_slave_handler);

  // Start handedness reporting timer
  handedness_print_timer = timer_read32();
  handedness_print_count = 0;

  if (!eeconfig_is_enabled()) {
    uprintf("Init: EEPROM not enabled, initializing...\n");
    eeconfig_init();
  }

  // Read user config from EEPROM
  // Lower 8 bits: Theme Index
  // Upper 8 bits: Start Hue
  uint16_t user_config = eeconfig_read_user();
  uint8_t saved_theme = (uint8_t)(user_config & 0xFF);
  uint8_t saved_hue = (uint8_t)((user_config >> 8) & 0xFF);
  uint8_t max_effects = RGB_MATRIX_EFFECT_MAX;

  uprintf("Init: EEPROM Read=%u (Theme %d, Hue %d)\n", user_config, saved_theme,
          saved_hue);

  // Increment Theme (Cycle 1 to MAX-1)
  uint8_t next_theme = saved_theme + 1;
  if (next_theme >= max_effects || next_theme == 0) {
    next_theme = 1;
  }

  // Increment Hue (Shift by ~42 for distinct color steps)
  // 0=Red, 42=Yellow, 84=Green, 126=Cyan, 168=Blue, 210=Magenta
  uint8_t next_hue = saved_hue + 42;

  // Save Combined
  uint16_t new_config = ((uint16_t)next_hue << 8) | next_theme;
  eeconfig_update_user(new_config);

  // Initialize the tracker with the OLD hue, so the first increment
  // in start_show_mode() lands exactly on 'next_hue'.
  automatic_hue_tracker = saved_hue;

  uprintf("Init: Next Theme=%d, Next Hue=%d. Saved=%u\n", next_theme, next_hue,
          new_config);

  // Defer RGB mode application to matrix_scan for timing reliability
  master_rgb_init_mode = next_theme;
  master_rgb_init_pending = true;

  rgb_auto_cycle = true;
  rgb_auto_timer = timer_read();
}

void housekeeping_task_user(void) {
  if (is_keyboard_master()) {
    static uint32_t last_sync = 0;
    bool needs_periodic = timer_elapsed32(last_sync) > 250;

    // Update caps lock state on master
    is_caps_lock_on = host_keyboard_led_state().caps_lock;

    // Print handedness 3 times after boot with 3 second delays
    if (handedness_print_count < 3) {
      if (timer_elapsed32(handedness_print_timer) > 3000) {
        bool master_is_left = is_keyboard_left();
        bool slave_is_left = !master_is_left;
        uint8_t ee_hands_val = eeconfig_read_handedness();
        uprintf("\n=== HANDEDNESS REPORT (%d/3) ===\n", handedness_print_count + 1);
        uprintf("EEPROM handedness byte: %d\n", ee_hands_val);
        uprintf("is_keyboard_left(): %s\n", master_is_left ? "true" : "false");
        uprintf("Master (USB): %s\n", master_is_left ? "LEFT" : "RIGHT");
        uprintf("Slave (TRRS): %s (inferred)\n", slave_is_left ? "LEFT" : "RIGHT");
        uprintf("================================\n\n");
        handedness_print_count++;
        handedness_print_timer = timer_read32();
      }
    }

    if (sync_needed || needs_periodic) {
      user_sync_info_t sync_data = {
          .is_flashlight = is_flashlight,
          .is_sniping_active = is_sniping_active,
          .mouse_is_locked = mouse_is_locked,
          .show_mode_active = show_mode_active,
          .is_caps_lock_on = is_caps_lock_on,
          .show_mode_digits = {show_mode_digits[0], show_mode_digits[1]},
          .show_mode_digit_count = show_mode_digit_count,
          .show_mode_current_digit = show_mode_current_digit,
          .show_mode_phase = show_mode_phase,
          .rgb_mode = rgb_matrix_get_mode(),
          .is_left_hand = is_keyboard_left()};

      user_sync_info_response_t response_data = {0};

      static uint8_t last_synced_mode = 0;
      if (transaction_rpc_exec(USER_SYNC_INFO, sizeof(sync_data), &sync_data,
                               sizeof(response_data), &response_data)) {
        last_sync = timer_read32();
        if (response_data.did_rgb_sync && sync_data.rgb_mode != last_synced_mode) {
          uprintf("\033[92mMaster: Slave synced RGB to mode %d\033[0m\n", sync_data.rgb_mode);
          last_synced_mode = sync_data.rgb_mode;
        }
        sync_needed = false;
      }
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

  if (x != 0 || y != 0) {
    if (auto_mouse_on) {
      // Any movement resets the timer to allow fine precision without timeout
      auto_mouse_timer = timer_read();
    } else if (x > threshold || x < -threshold || y > threshold ||
               y < -threshold) {
      movement_streak++;
      if (movement_streak > 1) { // Require 2 consecutive movement events
        // Only activate layer for significant movement
        uprintf("Mouse: Activated (x=%d, y=%d)\n", x, y);
        layer_on(3); // Switch to Mouse Layer (3)
        auto_mouse_on = true;
        auto_mouse_timer = timer_read();
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
  // Deferred RGB init - apply after matrix is fully ready
  if (master_rgb_init_pending) {
    rgb_matrix_mode_noeeprom(master_rgb_init_mode);
    master_rgb_init_pending = false;
    if (is_keyboard_master()) {
      start_show_mode();
    }
    uprintf("Deferred RGB init: mode %d applied\n", master_rgb_init_mode);
  }

  if (rgb_auto_cycle && timer_elapsed(rgb_auto_timer) > 30000) {
    rgb_matrix_step_noeeprom();
    rgb_auto_timer = timer_read();
    if (is_keyboard_master()) {
      start_show_mode();
    }
  }

  // Handle show mode flash sequence
  if (is_keyboard_master()) {
    if (show_mode_active && timer_elapsed(show_mode_timer) > 500) {
      if (show_mode_phase == 1) {
        // Flash was on, turn off
        show_mode_phase = 0;
        show_mode_timer = timer_read();
        sync_needed = true;
      } else {
        // Flash was off, move to next digit or end
        show_mode_current_digit++;
        if (show_mode_current_digit >= show_mode_digit_count) {
          show_mode_active = false;
          rgb_matrix_mode_noeeprom(
              rgb_matrix_get_mode()); // Final nudge to clear overrides
        } else {
          show_mode_phase = 1;
          show_mode_timer = timer_read();
        }
        sync_needed = true;
      }
    }
  }

  if (auto_mouse_on && !mouse_is_locked &&
      timer_elapsed(auto_mouse_timer) > auto_mouse_timeout) {
    layer_off(3);
    auto_mouse_on = false;
  }

  if (pgup_held && !pgup_triggered &&
      timer_elapsed(pgup_tap_timer) > MY_TAPPING_TERM) {
    layer_move(0);
    pgup_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (home_held && !home_triggered &&
      timer_elapsed(home_tap_timer) > MY_TAPPING_TERM) {
    layer_move(0);
    home_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (p_held && !p_triggered && timer_elapsed(p_tap_timer) > MY_TAPPING_TERM) {
    layer_move(0);
    p_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (ent_mo_held && !ent_mo_triggered &&
      timer_elapsed(ent_mo_timer) > MY_TAPPING_TERM) {
    layer_on(4);
    ent_mo_triggered = true;
    rgb_matrix_indicators_user();
  }

  if (k1_held && !k1_triggered &&
      timer_elapsed(k1_tap_timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 0) {
      layer_move(1);
    } else {
      layer_move(0);
    }
    k1_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (k2_held && !k2_triggered &&
      timer_elapsed(k2_tap_timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 0) {
      layer_move(2);
    } else {
      layer_move(0);
    }
    k2_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (k3_held && !k3_triggered &&
      timer_elapsed(k3_tap_timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 0) {
      layer_move(3);
    } else {
      layer_move(0);
    }
    k3_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (k4_held && !k4_triggered &&
      timer_elapsed(k4_tap_timer) > MY_TAPPING_TERM) {
    if (get_highest_layer(layer_state) == 0) {
      layer_move(4);
    } else {
      layer_move(0);
    }
    k4_triggered = true;
    rgb_matrix_indicators_user();
  }

  // New Layer 4 Keys Logic
  if (mins_held && !mins_triggered &&
      timer_elapsed(mins_tap_timer) > MY_TAPPING_TERM) {
    layer_move(0);
    mins_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (k0_held && !k0_triggered &&
      timer_elapsed(k0_tap_timer) > MY_TAPPING_TERM) {
    layer_move(1);
    k0_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (k9_held && !k9_triggered &&
      timer_elapsed(k9_tap_timer) > MY_TAPPING_TERM) {
    layer_move(2);
    k9_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (k8_held && !k8_triggered &&
      timer_elapsed(k8_tap_timer) > MY_TAPPING_TERM) {
    layer_move(3);
    k8_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (k7_held && !k7_triggered &&
      timer_elapsed(k7_tap_timer) > MY_TAPPING_TERM) {
    layer_move(0);
    k7_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (k6_held && !k6_triggered &&
      timer_elapsed(k6_tap_timer) > MY_TAPPING_TERM) {
    layer_move(0);
    k6_triggered = true;
    rgb_matrix_indicators_user();
  }

  // Thumb Toggle Logic
  if (ent_tg2_held && !ent_tg2_triggered &&
      timer_elapsed(ent_tg2_timer) > MY_TAPPING_TERM) {
    uprintf(">> Thumb Hold Triggered: ENT_TG2 -> Toggle Layer 2\n");
    if (get_highest_layer(layer_state) == 2) {
      layer_move(0);
    } else {
      layer_move(2);
    }
    ent_tg2_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (ent_tg4_held && !ent_tg4_triggered &&
      timer_elapsed(ent_tg4_timer) > MY_TAPPING_TERM) {
    uprintf(">> Thumb Hold Triggered: ENT_TG4 -> Toggle Layer 4\n");
    if (get_highest_layer(layer_state) == 4) {
      layer_move(0);
    } else {
      layer_move(4);
    }
    ent_tg4_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (spc_tg2_held && !spc_tg2_triggered &&
      timer_elapsed(spc_tg2_timer) > MY_TAPPING_TERM) {
    uprintf(">> Thumb Hold Triggered: SPC_TG2 -> Toggle Layer 2\n");
    if (get_highest_layer(layer_state) == 2) {
      layer_move(0);
    } else {
      layer_move(2);
    }
    spc_tg2_triggered = true;
    rgb_matrix_indicators_user();
  }
  if (f12_held && !f12_triggered &&
      timer_elapsed(f12_tap_timer) > MY_TAPPING_TERM) {
    uprintf(">> Thumb Hold Triggered: F12 -> Exit to Base\n");
    layer_move(0);
    f12_triggered = true;
    rgb_matrix_indicators_user();
  }
}
