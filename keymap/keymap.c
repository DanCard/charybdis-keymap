#include QMK_KEYBOARD_H

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

enum {
    TD_Z_LAYER = 0
};

// Custom Keycodes
enum custom_keycodes {
    KC_RAINBOW = QK_USER_0,
    KC_REACTIVE,
    KC_MOUSE_LOCK,
    KC_X_TG2,
    KC_ENT_L2_EXIT,
    KC_PGUP_TO0,
    KC_Q_TG4,
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
    KC_PLUS_COLON
};

uint8_t cur_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        if (state->interrupted || !state->pressed) return SINGLE_TAP;
        else return SINGLE_HOLD;
    } else if (state->count == 2) {
        return DOUBLE_TAP;
    } else return 0;
}

static tap_state_t z_tap_state = { .is_press_action = true, .state = 0 };
static bool is_flashlight = false;
static bool mouse_is_locked = false;
static uint8_t saved_rgb_mode;
static uint8_t saved_rgb_h, saved_rgb_s, saved_rgb_v;
static bool rgb_auto_cycle = false;
static uint16_t rgb_auto_timer = 0;

// Show mode state
static bool show_mode_active = false;
static uint8_t show_mode_digits[2];
static uint8_t show_mode_digit_count = 0;
static uint8_t show_mode_current_digit = 0;
static uint16_t show_mode_timer = 0;
static uint8_t show_mode_phase = 0; // 0=off, 1=on

// LED indices for number keys 1-0 (may need adjustment for your board)
// These are typically the matrix positions for the number row
static const uint8_t number_key_leds[] = {36, 37, 44, 45, 49, 20, 16, 15, 8, 7};

void start_show_mode(void) {
    uint8_t mode = rgb_matrix_get_mode();
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
}

void z_finished(tap_dance_state_t *state, void *user_data) {
    z_tap_state.state = cur_dance(state);
    switch (z_tap_state.state) {
        case SINGLE_TAP: 
            {
                uint8_t layer = get_highest_layer(layer_state);
                if (layer == 1) tap_code(KC_P0);
                else if (layer == 2) tap_code(KC_HOME);
                else if (layer == 4) tap_code(KC_SLSH);
                else register_code(KC_Z); 
            }
            break;
        case SINGLE_HOLD: 
            if (get_highest_layer(layer_state) == 3) {
                layer_move(0);
            } else {
                layer_move(3);
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
            break;
    }
}

void z_reset(tap_dance_state_t *state, void *user_data) {
    switch (z_tap_state.state) {
        case SINGLE_TAP: 
            {
                uint8_t layer = get_highest_layer(layer_state);
                if (layer != 1 && layer != 2 && layer != 4) {
                    unregister_code(KC_Z); 
                }
            }
            break;
        case SINGLE_HOLD: 
            break;
    }
    z_tap_state.state = 0;
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_Z_LAYER] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, z_finished, z_reset)
};

// Combo Definitions
const uint16_t PROGMEM copy_combo[] = {KC_A, KC_S, COMBO_END};
const uint16_t PROGMEM paste_combo[] = {KC_S, KC_D, COMBO_END};
const uint16_t PROGMEM paste_special_combo[] = {KC_D, KC_F, COMBO_END};
const uint16_t PROGMEM copy_special_combo[] = {KC_A, KC_F, COMBO_END};
const uint16_t PROGMEM delete_combo[] = {KC_J, KC_K, COMBO_END};

combo_t key_combos[] = {
    COMBO(copy_combo, LCTL(KC_C)),
    COMBO(paste_combo, LCTL(KC_V)),
    COMBO(paste_special_combo, C(S(KC_V))),
    COMBO(copy_special_combo, C(S(KC_C))),
    COMBO(delete_combo, KC_DEL),
};

static uint16_t x_tap_timer = 0;
static bool x_held = false;
static bool x_triggered = false;
static uint16_t pgup_tap_timer = 0;
static bool pgup_held = false;
static bool pgup_triggered = false;
static uint16_t q_tap_timer = 0;
static bool q_held = false;
static bool q_triggered = false;
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

#define MY_TAPPING_TERM 175

bool is_fast_mouse = false;
bool is_scroll_mode = false;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (is_scroll_mode && record->event.pressed) {
        switch (keycode) {
            case KC_MS_FAST_UP:
            case MS_UP: tap_code(MS_WHLU); return false;
            case KC_MS_FAST_DOWN:
            case MS_DOWN: tap_code(MS_WHLD); return false;
            case KC_MS_FAST_LEFT:
            case MS_LEFT: tap_code(MS_WHLL); return false;
            case KC_MS_FAST_RIGHT:
            case MS_RGHT: tap_code(MS_WHLR); return false;
        }
    }

    switch (keycode) {
        case KC_EXIT:
            if (record->event.pressed) {
                layer_move(0);
                rgb_matrix_indicators_user();
            }
        case KC_TURBO:
            if (record->event.pressed) {
                pointing_device_set_cpi(3000);
            } else {
                pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
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
            if (record->event.pressed) { register_code(MS_UP); register_code(MS_LEFT); }
            else { unregister_code(MS_UP); unregister_code(MS_LEFT); }
            return false;
        case KC_MS_DIAG_UR:
            if (record->event.pressed) { register_code(MS_UP); register_code(MS_RGHT); }
            else { unregister_code(MS_UP); unregister_code(MS_RGHT); }
            return false;
        case KC_MS_DIAG_DL:
            if (record->event.pressed) { register_code(MS_DOWN); register_code(MS_LEFT); }
            else { unregister_code(MS_DOWN); unregister_code(MS_LEFT); }
            return false;
        case KC_MS_DIAG_DR:
            if (record->event.pressed) { register_code(MS_DOWN); register_code(MS_RGHT); }
            else { unregister_code(MS_DOWN); unregister_code(MS_RGHT); }
            return false;
        case KC_SCR_MODE:
             if (record->event.pressed) { is_scroll_mode = true; } else { is_scroll_mode = false; }
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
                layer_state_to_restore = layer_state;
                layer_state_set(0); // Clear all layers (peek at base)
                rgb_matrix_indicators_user();
                ent_mo_timer = timer_read();
            } else {
                if (timer_elapsed(ent_mo_timer) < MY_TAPPING_TERM) {
                    tap_code(KC_ENT);
                    // Tap = Permanent Exit. We stay at layer 0.
                } else {
                    // Hold = Momentary Peek. Restore the full layer state.
                    layer_state_set(layer_state_to_restore);
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
                    layer_state_set(layer_state_to_restore);
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
                    layer_state_set(layer_state_to_restore);
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
        case KC_Q_TG4:
            if (record->event.pressed) {
                q_held = true;
                q_triggered = false;
                q_tap_timer = timer_read();
            } else {
                q_held = false;
                if (!q_triggered) {
                    tap_code(KC_Q);
                }
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
        case KC_X_TG2:
            if (record->event.pressed) {
                x_held = true;
                x_triggered = false;
                x_tap_timer = timer_read();
            } else {
                x_held = false;
                if (!x_triggered) {
                    uint8_t layer = get_highest_layer(layer_state);
                    if (layer == 1) tap_code(KC_P1);
                    else if (layer == 2) tap_code(KC_PGUP);
                    else if (layer == 4) tap_code(KC_DOT);
                    else tap_code(KC_X);
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
        case KC_REACTIVE:
            if (record->event.pressed) {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_SPLASH);
                start_show_mode();
            }
            return false;
        case KC_MOUSE_LOCK:
            if (record->event.pressed) {
                mouse_is_locked = !mouse_is_locked;
                if (mouse_is_locked) {
                    layer_on(3);
                } else {
                    layer_off(3);
                }
            }
            return false;
        case KC_ENT_L2_EXIT:
            if (record->event.pressed) {
                layer_off(2);
                tap_code(KC_ENT);
            }
            return false;
        case KC_1_TG1:
            if (record->event.pressed) {
                k1_held = true; k1_triggered = false; k1_tap_timer = timer_read();
            } else {
                k1_held = false;
                if (!k1_triggered) {
                    uint8_t layer = get_highest_layer(layer_state);
                    if (layer == 2) tap_code(KC_F1);
                    else if (layer == 3) { rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_LEFT_RIGHT); start_show_mode(); }
                    else if (layer == 4) tap_code(KC_0);
                    else tap_code(KC_1);
                }
            }
            return false;
        case KC_2_TG2:
            if (record->event.pressed) {
                k2_held = true; k2_triggered = false; k2_tap_timer = timer_read();
            } else {
                k2_held = false;
                if (!k2_triggered) {
                    uint8_t layer = get_highest_layer(layer_state);
                    if (layer == 2) tap_code(KC_F2);
                    else if (layer == 3) { rgb_matrix_step_noeeprom(); start_show_mode(); }
                    else if (layer == 4) tap_code(KC_9);
                    else tap_code(KC_2);
                }
            }
            return false;
        case KC_3_TG3:
            if (record->event.pressed) {
                k3_held = true; k3_triggered = false; k3_tap_timer = timer_read();
            } else {
                k3_held = false;
                if (!k3_triggered) {
                    uint8_t layer = get_highest_layer(layer_state);
                    if (layer == 2) tap_code(KC_F3);
                    else if (layer == 3) tap_code(KC_3); 
                    else if (layer == 4) tap_code(KC_8);
                    else tap_code(KC_3);
                }
            }
            return false;
        case KC_4_TG4:
            if (record->event.pressed) {
                k4_held = true; k4_triggered = false; k4_tap_timer = timer_read();
            } else {
                k4_held = false;
                if (!k4_triggered) {
                    uint8_t layer = get_highest_layer(layer_state);
                    if (layer == 2) tap_code(KC_F4);
                    else if (layer == 3) tap_code(KC_4); 
                    else if (layer == 4) tap_code(KC_7);
                    else tap_code(KC_4);
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
            }
            return false;
        case RGB_HUI:
        case RGB_HUD:
        case RGB_SAI:
        case RGB_SAD:
            if (record->event.pressed) {
                uint8_t mode = rgb_matrix_get_mode();
                if (mode == RGB_MATRIX_CYCLE_LEFT_RIGHT || mode == RGB_MATRIX_CYCLE_ALL || mode == RGB_MATRIX_CYCLE_SPIRAL) {
                    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
                }
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
    }
    return true;
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(KC_ESC, KC_1_TG1, KC_2_TG2, KC_3_TG3, KC_4_TG4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_TAB, KC_Q_TG4, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS, KC_LSFT, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_PLUS_COLON, KC_QUOT, KC_LCTL, TD(TD_Z_LAYER), KC_X_TG2, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, LT(3,KC_SLSH), KC_RSFT, KC_SPC, KC_ENT, KC_L_TG1, KC_R_TG2, KC_ENT, KC_LALT, KC_BSPC, KC_BSPC),
    [1] = LAYOUT(KC_ESC, KC_1_TG1, KC_2_TG2, KC_3_TG3, KC_4_TG4, KC_5, S(KC_6), S(KC_7), S(KC_8), S(KC_9), S(KC_0), S(KC_MINS), KC_TAB, KC_PMNS, KC_P7, KC_P8, KC_P9, KC_PAST, KC_LBRC, KC_LBRC, KC_RBRC, S(KC_LBRC), S(KC_RBRC), KC_NO, KC_LSFT, KC_PPLS, KC_P4, KC_P5, KC_P6, KC_PSLS, KC_PPLS, KC_LEFT, KC_UP, KC_DOWN, KC_RGHT, KC_PEQL, KC_LCTL, KC_P0, KC_P1, KC_P2, KC_P3, KC_PEQL, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RGB_VAI, RGB_VAD, KC_SPC, KC_ENT, KC_L_TG1, KC_R_TG2, KC_ENT, KC_LALT, KC_BSPC, KC_BSPC),
    [2] = LAYOUT(KC_GRV, KC_1_TG1, KC_2_TG2, KC_3_TG3, KC_4_TG4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_NO, KC_RAINBOW, KC_REACTIVE, KC_JELLY, KC_SPIRAL, KC_CHEVRON, KC_LBRC, KC_LBRC, KC_RBRC, S(KC_LBRC), S(KC_RBRC), KC_NO, KC_EXIT, KC_LEFT, KC_UP, KC_DOWN, KC_RGHT, KC_RGB_AUTO, KC_EXIT, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_NO, LT(3, KC_HOME), KC_PGUP, KC_PGDN, KC_END, KC_NO, KC_NO, KC_HOME, KC_PGUP, KC_PGDN, KC_END, KC_NO, KC_SPC_EXIT, KC_ENT_EXIT, KC_L_TG1, KC_R_TG2, KC_ENT_EXIT, KC_DEL, KC_BSPC_EXIT, KC_BSPC_EXIT),
    [3] = LAYOUT(QK_BOOT, QK_CLEAR_EEPROM, KC_MS_FAST_UP, KC_3_TG3, KC_4_TG4, RM_NEXT, KC_TRNS, KC_TRNS, KC_RAINBOW, KC_REACTIVE, QK_CLEAR_EEPROM, QK_BOOT, MS_BTN3, KC_TRNS, KC_MS_DIAG_UL, MS_UP, KC_MS_DIAG_UR, KC_SCR_MODE, DPI_MOD, S_D_MOD, KC_TURBO, DPI_MOD, KC_NO, KC_NO, KC_MS_FAST_LEFT, MS_LEFT, MS_BTN1, MS_RGHT, KC_MS_FAST_RIGHT, KC_NO, MS_BTN3, KC_RSFT, KC_RCTL, KC_RALT, KC_RGUI, KC_NO, TD(TD_Z_LAYER), KC_MS_DIAG_DL, MS_DOWN, KC_MS_FAST_DOWN, KC_MS_DIAG_DR, KC_NO, KC_NO, MS_BTN1, KC_MOUSE_LOCK, SNIPING, DRGSCRL, KC_TRNS, MS_BTN1, KC_ENT_EXIT, KC_L_TG1, KC_R_TG2, KC_ENT_EXIT, MS_BTN3, MS_BTN2, MS_BTN2),
    [4] = LAYOUT(KC_MINS, KC_1_TG1, KC_2_TG2, KC_3_TG3, KC_4_TG4, KC_6, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_BSLS, KC_P_TO0, KC_O, KC_I, KC_U, KC_Y, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS, KC_QUOT, KC_SCLN, KC_L, KC_K, KC_J, KC_H, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_RSFT, LT(3, KC_SLSH), KC_DOT, KC_COMM, KC_M, KC_N, KC_N, KC_M, KC_COMM, KC_DOT, LT(3,KC_SLSH), KC_RSFT, KC_SPC, KC_ENT_EXIT, KC_L_TG1, KC_R_TG2, KC_ENT_EXIT, KC_LALT, KC_BSPC, KC_BSPC),
};

bool rgb_matrix_indicators_user(void) {
    if (is_flashlight) {
        rgb_matrix_set_color_all(255, 255, 255);
        return false;
    }

    uint8_t layer = get_highest_layer(layer_state);
    uint8_t r = 0, g = 0, b = 0;
    bool show_layer = true;

    switch (layer) {
        case 4: r = 0; g = 255; b = 255; break; // Teal (One-Hand)
        case 3:
            if (mouse_is_locked) {
                r = 255; g = 0; b = 255; // Pink (Mouse Locked)
            } else {
                r = 255; g = 255; b = 0; // Yellow (Mouse Active)
            }
            break;
        case 2: r = 0; g = 255; b = 0; break; // Green (Function)
        case 1: r = 0; g = 0; b = 255; break; // Blue (Symbols)
        default: show_layer = false; break;
    }

    if (show_layer) {
        // Set thumb keys as layer indicators (Left: 24-28, Right: 53-57)
        for (uint8_t i = 24; i <= 28; i++) rgb_matrix_set_color(i, r, g, b);
        for (uint8_t i = 53; i <= 57; i++) rgb_matrix_set_color(i, r, g, b);
    }

    // Flash number key to show current RGB mode
    if (show_mode_active && show_mode_phase == 1) {
        uint8_t digit = show_mode_digits[show_mode_current_digit];
        // digit 1-9 maps to index 0-8, digit 0 maps to index 9
        uint8_t led_index = (digit == 0) ? number_key_leds[9] : number_key_leds[digit - 1];
        rgb_matrix_set_color(led_index, 255, 255, 255); // White flash
    }

    return true; // Allow background effects to show on non-indicator keys
}

void keyboard_post_init_user(void) {
    rgb_matrix_mode_noeeprom(RGB_MATRIX_CYCLE_LEFT_RIGHT);
    rgb_auto_cycle = true;
    rgb_auto_timer = timer_read();
    start_show_mode();
}

static uint16_t auto_mouse_timer = 0;
static bool auto_mouse_on = false;

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    int8_t x = mouse_report.x;
    int8_t y = mouse_report.y;

    if (x != 0 || y != 0) {
        if (!auto_mouse_on) {
            layer_on(3); // Switch to Mouse Layer (3)
            auto_mouse_on = true;
        }
        auto_mouse_timer = timer_read();
    }
    return mouse_report;
}

void matrix_scan_user(void) {
    if (rgb_auto_cycle && timer_elapsed(rgb_auto_timer) > 30000) {
        rgb_matrix_step_noeeprom();
        rgb_auto_timer = timer_read();
        start_show_mode();
    }

    // Handle show mode flash sequence
    if (show_mode_active && timer_elapsed(show_mode_timer) > 300) {
        if (show_mode_phase == 1) {
            // Flash was on, turn off
            show_mode_phase = 0;
            show_mode_timer = timer_read();
        } else {
            // Flash was off, move to next digit or end
            show_mode_current_digit++;
            if (show_mode_current_digit >= show_mode_digit_count) {
                show_mode_active = false;
            } else {
                show_mode_phase = 1;
                show_mode_timer = timer_read();
            }
        }
    }

    if (auto_mouse_on && !mouse_is_locked && timer_elapsed(auto_mouse_timer) > 650) { // 650ms timeout
        layer_off(3);
        auto_mouse_on = false;
    }

    if (x_held && !x_triggered && timer_elapsed(x_tap_timer) > MY_TAPPING_TERM) {
        if (get_highest_layer(layer_state) == 2) {
            layer_move(0);
        } else {
            layer_move(2);
        }
        x_triggered = true;
        rgb_matrix_indicators_user();
    }
    if (pgup_held && !pgup_triggered && timer_elapsed(pgup_tap_timer) > MY_TAPPING_TERM) {
        layer_move(0);
        pgup_triggered = true;
        rgb_matrix_indicators_user();
    }
    if (home_held && !home_triggered && timer_elapsed(home_tap_timer) > MY_TAPPING_TERM) {
        layer_move(0);
        home_triggered = true;
        rgb_matrix_indicators_user();
    }
    if (q_held && !q_triggered && timer_elapsed(q_tap_timer) > MY_TAPPING_TERM) {
        if (get_highest_layer(layer_state) == 4) {
            layer_move(0);
        } else {
            layer_move(4);
        }
        q_triggered = true;
        rgb_matrix_indicators_user();
    }
    if (p_held && !p_triggered && timer_elapsed(p_tap_timer) > MY_TAPPING_TERM) {
        layer_move(0);
        p_triggered = true;
        rgb_matrix_indicators_user();
    }
    if (ent_mo_held && !ent_mo_triggered && timer_elapsed(ent_mo_timer) > MY_TAPPING_TERM) {
        layer_on(4);
        ent_mo_triggered = true;
        rgb_matrix_indicators_user();
    }

    if (k1_held && !k1_triggered && timer_elapsed(k1_tap_timer) > MY_TAPPING_TERM) {
        if (get_highest_layer(layer_state) == 0) {
            layer_move(1);
        } else {
            layer_move(0);
        }
        k1_triggered = true;
        rgb_matrix_indicators_user();
    }
    if (k2_held && !k2_triggered && timer_elapsed(k2_tap_timer) > MY_TAPPING_TERM) {
        if (get_highest_layer(layer_state) == 0) {
            layer_move(2);
        } else {
            layer_move(0);
        }
        k2_triggered = true;
        rgb_matrix_indicators_user();
    }
    if (k3_held && !k3_triggered && timer_elapsed(k3_tap_timer) > MY_TAPPING_TERM) {
        if (get_highest_layer(layer_state) == 0) {
            layer_move(3);
        } else {
            layer_move(0);
        }
        k3_triggered = true;
        rgb_matrix_indicators_user();
    }
    if (k4_held && !k4_triggered && timer_elapsed(k4_tap_timer) > MY_TAPPING_TERM) {
        if (get_highest_layer(layer_state) == 0) {
            layer_move(4);
        } else {
            layer_move(0);
        }
        k4_triggered = true;
        rgb_matrix_indicators_user();
    }
}
