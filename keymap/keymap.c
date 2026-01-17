#include QMK_KEYBOARD_H
#include <stdio.h>
#include "transactions.h"
#include <lib/lib8tion/lib8tion.h>
#include "features/sync.h"
#include "features/mouse.h"
#include "features/rgb.h"
#include "features/tap_hold.h"
#include "features/logging.h"
#include "features/statistics.h"

const char *layer_change_reason = NULL;
#define REASON_BUFFER_SIZE 128
static char layer_reason_buffer[REASON_BUFFER_SIZE];

#include "features/keycodes.h"

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



// Helper Functions for Duplicate Code Consolidation
// Simple tap-hold key table: maps keycode -> (th_index, tap_keycode) - Removed (in features/tap_hold.c)

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

// =============================================================================

// Deferred EEPROM update to avoid USB timeout on boot
static uint16_t pending_eeprom_config = 0;
static bool eeprom_update_pending = false;
static uint32_t eeprom_defer_timer = 0;

static uint8_t master_rgb_init_mode = 0;
static bool master_rgb_init_pending = false;

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

// RGB mode name lookup table - Removed (in features/rgb.c)

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

// Right Hand Number Combos
const uint16_t PROGMEM jcol_combo[] = {KC_J, KC_PLUS_COLON, COMBO_END}; // 6 (Mirrors 5: A+F)
const uint16_t PROGMEM jk_combo[]   = {KC_J, KC_K, COMBO_END};          // 7 (Mirrors 4: D+F)
const uint16_t PROGMEM kl_combo[]   = {KC_K, KC_L, COMBO_END};          // 8 (Mirrors 3: S+D)
const uint16_t PROGMEM lcol_combo[] = {KC_L, KC_PLUS_COLON, COMBO_END}; // 9 (Mirrors 2: A+S)

combo_t key_combos[] = {
    COMBO(left_combo, KC_2),      COMBO(up_combo, KC_3),
    COMBO(down_combo, KC_4),      COMBO(af_combo, KC_5),
    COMBO(ad_combo, KC_DEL),
    COMBO(jcol_combo, KC_6),      COMBO(jk_combo, KC_7),
    COMBO(kl_combo, KC_8),        COMBO(lcol_combo, KC_9),
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
  process_statistics(record);

  uint32_t now = timer_read32();
  uint32_t sec = now / 1000;
  uint32_t ms = now % 1000;

  if (record->event.pressed) {
    // Shift cancels Caps Lock
    if (host_keyboard_led_state().caps_lock && (keycode == KC_LSFT || keycode == KC_RSFT)) {
      tap_code(KC_CAPS);
      return false;
    }

    uint32_t diff = timer_elapsed32(last_key_time);
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
  if (process_tap_hold_key(keycode, record)) {
    return false;
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
      layer_change_reason = "ENT_EXIT (Tap): L0";
      layer_state_set(0); // Clear all layers (peek at base)
      rgb_matrix_indicators_user();
      th[TH_ENT_MO].timer = timer_read();
    } else {
      uprintf("KC_ENT_EXIT Released. Elapsed: %u. Action: %s\n",
              timer_elapsed(th[TH_ENT_MO].timer),
              (timer_elapsed(th[TH_ENT_MO].timer) < TAPPING_TERM) ? "Tap (Exit)"
                                                              : "Hold (Exit)");
      if (timer_elapsed(th[TH_ENT_MO].timer) < TAPPING_TERM) {
        tap_code(KC_ENT);
        // Tap = Permanent Exit. We stay at layer 0.
      } else {
        // Hold = Momentary Peek. Restore the full layer state.
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
      activate_snipe_mode();
    }
    return false;
  case KC_FAST:
    if (record->event.pressed) {
      activate_fast_mode();
    }
    return false;
  case KC_ENT_L2_EXIT:
    if (record->event.pressed) {
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
        // Shift already held by user: send colon directly
        tap_code(KC_SCLN);
      } else {
        // Shift not held: send plus (shifted equals sign)
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
    }
    return true; // Let core handle the DPI change
  case DPI_RMOD:
    if (record->event.pressed) {
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
      is_day_mode = true;
      sync_needed = true;
      uprintf("Day Mode: Brightness set to 225\n");
      layer_move(0);
    }
    return false;
  case KC_NIGHT:
    if (record->event.pressed) {
      // Set Low Brightness (Night Mode)
      // Keep current Hue/Sat, set Value to 16
      rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), 16);
      is_day_mode = false;
      sync_needed = true;
      uprintf("Night Mode: Brightness set to 16\n");
      layer_move(0);
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
      layer_move(0);
    }
    return false;
  case KC_PRINT_STATS:
    if (record->event.pressed) {
      print_statistics_now();
      layer_move(0);
    }
    return false;
  case KC_PRINT_STATS_GRID:
    if (record->event.pressed) {
      print_statistics_grid();
      layer_move(0);
    }
    return false;
  // Layer 3 Thumb Logic: Tap = Exit, Hold = Switch Layer
  case KC_L3_EXT_TO4: return handle_l3_thumb(TH_L3_TO4, 4, record->event.pressed);
  case KC_L3_EXT_TO2: return handle_l3_thumb(TH_L3_TO2, 2, record->event.pressed);
  case KC_L3_EXT_TO1: return handle_l3_thumb(TH_L3_TO1, 6, record->event.pressed);
  
  // Dual-function Arrow/Number keys
  case KC_2_LEFT:
    if (record->event.pressed) {
      if (get_mods() & MOD_MASK_SHIFT) {
        uint8_t mods = get_mods();
        unregister_mods(MOD_MASK_SHIFT);
        tap_code(KC_2);
        register_mods(mods);
      } else {
        tap_code(KC_LEFT);
      }
    }
    return false;
  case KC_3_UP:
    if (record->event.pressed) {
      if (get_mods() & MOD_MASK_SHIFT) {
        uint8_t mods = get_mods();
        unregister_mods(MOD_MASK_SHIFT);
        tap_code(KC_3);
        register_mods(mods);
      } else {
        tap_code(KC_UP);
      }
    }
    return false;
  case KC_4_DOWN:
    if (record->event.pressed) {
      if (get_mods() & MOD_MASK_SHIFT) {
        uint8_t mods = get_mods();
        unregister_mods(MOD_MASK_SHIFT);
        tap_code(KC_4);
        register_mods(mods);
      } else {
        tap_code(KC_DOWN);
      }
    }
    return false;
  case KC_5_RIGHT:
    if (record->event.pressed) {
      if (get_mods() & MOD_MASK_SHIFT) {
        uint8_t mods = get_mods();
        unregister_mods(MOD_MASK_SHIFT);
        tap_code(KC_5);
        register_mods(mods);
      } else {
        tap_code(KC_LEFT);
      }
    }
    return false;
  case KC_6_LEFT:
    if (record->event.pressed) {
      if (get_mods() & MOD_MASK_SHIFT) {
        uint8_t mods = get_mods();
        unregister_mods(MOD_MASK_SHIFT);
        tap_code(KC_6);
        register_mods(mods);
      } else {
        tap_code(KC_LEFT);
      }
    }
    return false;
  case KC_2_TO2:
    if (!record->event.pressed) {
      if (!th[TH_K2_TO2].triggered) {
        if (get_mods() & MOD_MASK_SHIFT) {
          uint8_t mods = get_mods();
          unregister_mods(MOD_MASK_SHIFT);
          tap_code(KC_2);
          register_mods(mods);
        } else {
          tap_code(KC_LEFT);
        }
      }
    }
    return false;
  case KC_3_TO3:
    if (!record->event.pressed) {
      if (!th[TH_K3_TO3].triggered) {
        if (get_mods() & MOD_MASK_SHIFT) {
          uint8_t mods = get_mods();
          unregister_mods(MOD_MASK_SHIFT);
          tap_code(KC_3);
          register_mods(mods);
        } else {
          tap_code(KC_UP);
        }
      }
    }
    return false;
  case KC_4_TO4:
    if (!record->event.pressed) {
      if (!th[TH_K4_TO4].triggered) {
        if (get_mods() & MOD_MASK_SHIFT) {
          uint8_t mods = get_mods();
          unregister_mods(MOD_MASK_SHIFT);
          tap_code(KC_4);
          register_mods(mods);
        } else {
          tap_code(KC_DOWN);
        }
      }
    }
    return false;
  case KC_5_TO5:
    if (!record->event.pressed) {
      if (!th[TH_K5_TO5].triggered) {
        if (get_mods() & MOD_MASK_SHIFT) {
          uint8_t mods = get_mods();
          unregister_mods(MOD_MASK_SHIFT);
          tap_code(KC_5);
          register_mods(mods);
        } else {
          tap_code(KC_RIGHT);
        }
      }
    }
    return false;
  case KC_6_TO6:
    if (!record->event.pressed) {
      if (!th[TH_K6_TO6].triggered) {
        if (get_mods() & MOD_MASK_SHIFT) {
          uint8_t mods = get_mods();
          unregister_mods(MOD_MASK_SHIFT);
          tap_code(KC_6);
          register_mods(mods);
        } else {
          tap_code(KC_LEFT);
        }
      }
    }
    return false;
  case KC_7_UP:
    if (record->event.pressed) {
      if (get_mods() & MOD_MASK_SHIFT) {
        uint8_t mods = get_mods();
        unregister_mods(MOD_MASK_SHIFT);
        tap_code(KC_7);
        register_mods(mods);
      } else {
        tap_code(KC_UP);
      }
    }
    return false;
  case KC_8_DOWN:
    if (record->event.pressed) {
      if (get_mods() & MOD_MASK_SHIFT) {
        uint8_t mods = get_mods();
        unregister_mods(MOD_MASK_SHIFT);
        tap_code(KC_8);
        register_mods(mods);
      } else {
        tap_code(KC_DOWN);
      }
    }
    return false;
  case KC_9_RIGHT:
    if (record->event.pressed) {
      if (get_mods() & MOD_MASK_SHIFT) {
        uint8_t mods = get_mods();
        unregister_mods(MOD_MASK_SHIFT);
        tap_code(KC_9);
        register_mods(mods);
      } else {
        tap_code(KC_LEFT);
      }
    }
    return false;

  default:
    return true;
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
        LAYOUT(QK_GESC, KC_1_TG1, KC_2_TO2, KC_3_TO3, KC_4_TO4, KC_5_TO5,   KC_6_TO6, KC_7_UP, KC_8_DOWN, KC_9_RIGHT, KC_0, KC_MINS,
               KC_TAB , KC_Q_TG4, KC_W    , KC_E    , KC_R    , KC_T    ,   KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,
               KC_LSFT, KC_A    , KC_S    , KC_D    , KC_F    , KC_G    ,   KC_H, KC_J, KC_K, KC_L, KC_PLUS_COLON, KC_QUOT,
               KC_LCTL, TD(TD_Z_LAYER), KC_X, KC_C  , KC_V    , KC_B    ,   KC_N, KC_M, KC_COMM, KC_DOT, LT(3, KC_SLSH), KC_RSFT,
                                        KC_SPC_TG4, KC_ENT_TG2, KC_L_TG1,   KC_DEL, KC_ENT_TG2,
                                                        KC_LALT, KC_BSPC,   KC_BSPC),
    // Layer 1: Copy of original base layer (Original)
    [1] = LAYOUT(QK_GESC, KC_1_TG1, KC_2_TG2, KC_3_TG3, KC_4_TG4, KC_5_TG5,   KC_6, KC_7, KC_8, KC_9, KC_0_TO0, KC_MINS,
                 KC_TAB , KC_Q_TG4, KC_W    , KC_E    , KC_R    , KC_T    ,   KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,
                 KC_LSFT, KC_A    , KC_S    , KC_D    , KC_F    , KC_G    ,   KC_H, KC_J, KC_K, KC_L, KC_PLUS_COLON, KC_QUOT,
                 KC_LCTL, TD(TD_Z_LAYER), KC_X, KC_C  , KC_V    , KC_B    ,   KC_N, KC_M, KC_COMM, KC_DOT, LT(3, KC_SLSH), KC_RSFT,
                                          KC_SPC_TG4, KC_ENT_TG2, KC_L_TG1,   KC_DEL, KC_ENT_TG2,
                                                          KC_LALT, KC_BSPC,   KC_BSPC),
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
    // Layer 6: number symbol layer
    [5] = LAYOUT(KC_PSCR, KC_EXIT    , TO(2), TO(3), TO(4), KC_EXIT,   KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT   , KC_PSCR   , QK_BOOT,
                 KC_TAB , KC_MINS_TO0, KC_7 , KC_8 , KC_9 , KC_EXIT,   KC_EXIT, KC_LBRC, KC_RBRC, S(KC_LBRC), S(KC_RBRC), HYPR(KC_N),
                 KC_EXIT, S(KC_EQL)  , KC_4 , KC_5 , KC_6 , KC_EXIT,   KC_EXIT, KC_LEFT, KC_UP  , KC_DOWN   , KC_RGHT   , KC_EXIT,
                 KC_LCTL, KC_0       , KC_1 , KC_2 , KC_3 , KC_EQL ,   KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT   , KC_EXIT   , KC_EXIT,
                                 KC_SPC_EXIT, KC_ENT_EXIT, KC_L_TG1,   KC_R_TG2, KC_ENT_EXIT,
                                              KC_LALT, KC_BSPC_EXIT,   KC_BSPC_EXIT),
    // Settings Layer - RGB and Mouse configuration (accessed via Hold '5')
    [6] = LAYOUT(KC_PSCR, KC_EXIT, KC_EXIT, KC_EXIT, KC_EXIT    , KC_EXIT  ,   KC_DEBUG_SYNC, KC_PRINT_STATS, KC_PRINT_STATS_GRID, KC_EXIT  , KC_EXIT  , KC_PSCR,
                 KC_EXIT, RM_TOGG, RM_NEXT, RM_PREV, KC_RGB_AUTO, KC_P_FRAC,   KC_FIRE      , KC_EXIT       , KC_EXIT            , KC_EXIT  , KC_EXIT  , QK_CLEAR_EEPROM,
                 KC_EXIT, KC_FLASHLIGHT, RM_VALU, RM_VALD, KC_DAY, KC_NIGHT,   KC_EXIT      , DPI_MOD    , DPI_RMOD , KC_JITTER, KC_EXIT  , KC_EXIT,
                 KC_EXIT, RM_HUEU   , RM_HUED, RM_SATU , RM_SATD , KC_EXIT ,   KC_PINWHEEL, KC_MS_TMO_INC, KC_MS_TMO_DEC, KC_EXIT, KC_EXIT, KC_EXIT,
                                                  KC_EXIT, KC_EXIT, KC_EXIT,   KC_EXIT, KC_EXIT,
                                                           KC_EXIT, KC_EXIT,   KC_EXIT),
};

bool rgb_matrix_indicators_user(void) {
  return housekeeping_rgb_indicators();
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
  is_jitter_filter_active = (user_config & 0x0080);
  is_day_mode = rgb_matrix_get_val() > 100;

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
  housekeeping_task_sync();
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
  return housekeeping_mouse_task(mouse_report);
}

void matrix_scan_user(void) {
  matrix_scan_statistics();

  // Handle deferred EEPROM update (wait >1s after boot)
  if (eeprom_update_pending && timer_elapsed32(eeprom_defer_timer) > 1500) {
    eeconfig_update_user(pending_eeprom_config);
    eeprom_update_pending = false;
    uprintf("Deferred EEPROM update executed. Config=%u\n", pending_eeprom_config);
  }

  // Deferred RGB init - apply after matrix is fully ready
  if (master_rgb_init_pending) {
    handle_rgb_mode_change(master_rgb_init_mode);
    master_rgb_init_pending = false;
    uprintf("Deferred RGB init: mode %d applied\n", master_rgb_init_mode);
  }

  // Process mouse-related timeouts (Snipe, Fast Mode, Auto Mouse)
  process_mouse_timeouts();

  if (rgb_auto_cycle && timer_elapsed(rgb_auto_timer) > 30000) {
    rgb_matrix_step_noeeprom();
    rgb_auto_timer = timer_read();
    if (is_keyboard_master()) {
      handle_rgb_mode_change(rgb_matrix_get_mode());
    }
  }

  housekeeping_tap_hold();
}


