#include "tap_hold.h"
#include "rgb.h"
#include "sync.h"
#include "keycodes.h"
#include "logging.h"
#include <lib/lib8tion/lib8tion.h>

// Tap/Hold Key State
tap_hold_t th[TH_COUNT] = {0};

// Simple tap-hold key table
static const simple_tap_hold_t simple_tap_holds[] = {
    {KC_PGUP_TO0, TH_PGUP, KC_PGUP},
    {KC_HOME_TO0, TH_HOME, KC_HOME},
    {KC_MINS_TO0, TH_MINS, KC_MINS},
    {KC_0_L1, TH_K0, KC_0},
    {KC_9_L2, TH_K9, KC_9},
    {KC_8_L3, TH_K8, KC_8},
    {KC_7_L7, TH_K7_L7, KC_7},
    {KC_6_TO0, TH_K6, KC_6},
    {KC_PMNS_L4, TH_PMNS_TG4, KC_PMNS},
    {KC_F12_EXIT, TH_F12, KC_F12},
    {KC_0_TO0, TH_K0_TO0, KC_0},
    {KC_6_L6, TH_K6_TO6, KC_6},
};
#define SIMPLE_TAP_HOLD_COUNT (sizeof(simple_tap_holds) / sizeof(simple_tap_holds[0]))

bool process_tap_hold_key(uint16_t keycode, keyrecord_t *record) {

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
            return true; // Handled
        }
    }
    return false; // Not handled
}

// Exit key helper (SPC_EXIT, ENT_EXIT, BSPC_EXIT)
bool handle_exit_key(uint16_t tap_key, bool pressed) {
    if (pressed) {
        // layer_change_reason = "Exit Key: L0"; // Handled by layer_history_pop
        layer_history_pop();
        housekeeping_rgb_indicators(); // Call rgb indicator update
        th[TH_ENT_MO].timer = timer_read();
    } else {
        if (timer_elapsed(th[TH_ENT_MO].timer) < LONG_PRESS_TIMEOUT) {
            tap_code(tap_key);
        }
        housekeeping_rgb_indicators();
    }
    return false;
}

// Layer 3 thumb key helper (tap=switch layer, hold=switch layer)
bool handle_l3_thumb(uint8_t th_idx, uint8_t tap_layer, uint8_t hold_layer, bool pressed) {
    if (pressed) {
        th[th_idx].held = true;
        th[th_idx].timer = timer_read();
    } else {
        th[th_idx].held = false;
        if (timer_elapsed(th[th_idx].timer) < LONG_PRESS_TIMEOUT) {
            layer_change_reason = "L3 Thumb Tap";
            layer_move(tap_layer);
        } else {
            layer_change_reason = "L3 Thumb Hold";
            layer_move(hold_layer);
        }
        housekeeping_rgb_indicators();
    }
    return false;
}

// Verbose thumb toggle helper (ENT_TG2, ENT_TG4, SPC_TG2, SPC_TG4)
bool handle_thumb_toggle(uint8_t th_idx, uint16_t tap_key, const char* name, bool pressed) {
    if (pressed) {
        TH_PRESS(th_idx);
    } else {
        uint32_t now = timer_read32();
        uprintf("[%lu.%03lu] %s Released. Duration: %u ms. Action: %s\n",
                (unsigned long)(now/1000), (unsigned long)(now%1000), name,
                timer_elapsed(th[th_idx].timer),
                !th[th_idx].triggered ? "Tap" : "Hold handled");
        th[th_idx].held = false;
        if (!th[th_idx].triggered) tap_code(tap_key);
    }
    return false;
}

void housekeeping_tap_hold(void) {
  if (th[TH_PGUP].held && !th[TH_PGUP].triggered &&
      timer_elapsed(th[TH_PGUP].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "PGUP(Hold): Exit to Base";
    layer_move(0);
    th[TH_PGUP].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_HOME].held && !th[TH_HOME].triggered &&
      timer_elapsed(th[TH_HOME].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "HOME(Hold): Exit to Base";
    layer_move(0);
    th[TH_HOME].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_P].held && !th[TH_P].triggered && timer_elapsed(th[TH_P].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "P(Hold): Peek at Base";
    layer_state_set(0); // Peek at base layer
    th[TH_P].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_SLSH].held && !th[TH_SLSH].triggered && timer_elapsed(th[TH_SLSH].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "/(Hold): Peek at Base";
    layer_state_set(0); // Peek at base layer
    th[TH_SLSH].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_Q].held && !th[TH_Q].triggered && timer_elapsed(th[TH_Q].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "Q(Hold): Peek at One-Hand (L4)";
    layer_move(4); // Peek at layer 4
    th[TH_Q].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_L1].held && !th[TH_L1].triggered && timer_elapsed(th[TH_L1].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "L_TG1(Hold): Layer 3";
    layer_move(3);
    th[TH_L1].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_ENT_MO].held && !th[TH_ENT_MO].triggered &&
      timer_elapsed(th[TH_ENT_MO].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "ENT(Hold): MO(4)";
    layer_on(4);
    th[TH_ENT_MO].triggered = true;
    housekeeping_rgb_indicators();
  }

  if (th[TH_K1].held && !th[TH_K1].triggered &&
      timer_elapsed(th[TH_K1].timer) > LONG_PRESS_TIMEOUT) {
    if (get_highest_layer(layer_state) == 0) {
      layer_change_reason = "K1(Hold): Toggle L1 (ON)";
      layer_move(1);
    } else {
      layer_change_reason = "K1(Hold): Toggle L1 (OFF)";
      layer_move(0);
    }
    th[TH_K1].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_K2].held && !th[TH_K2].triggered &&
      timer_elapsed(th[TH_K2].timer) > LONG_PRESS_TIMEOUT) {
    if (get_highest_layer(layer_state) <= 1) {
      layer_change_reason = "K2(Hold): Toggle L2 (ON)";
      layer_move(2);
    } else {
      layer_change_reason = "K2(Hold): Toggle L2 (OFF)";
      layer_move(0);
    }
    th[TH_K2].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_K3].held && !th[TH_K3].triggered &&
      timer_elapsed(th[TH_K3].timer) > LONG_PRESS_TIMEOUT) {
    if (get_highest_layer(layer_state) <= 1) {
      layer_change_reason = "K3(Hold): Toggle L3 (ON)";
      layer_move(3);
    } else {
      layer_change_reason = "K3(Hold): Toggle L3 (OFF)";
      layer_move(0);
    }
    th[TH_K3].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_K4].held && !th[TH_K4].triggered &&
      timer_elapsed(th[TH_K4].timer) > LONG_PRESS_TIMEOUT) {
    if (get_highest_layer(layer_state) <= 1) {
      layer_change_reason = "K4(Hold): Toggle L4 (ON)";
      layer_move(4);
    } else {
      layer_change_reason = "K4(Hold): Toggle L4 (OFF)";
      layer_move(0);
    }
    th[TH_K4].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_K5].held && !th[TH_K5].triggered &&
      timer_elapsed(th[TH_K5].timer) > LONG_PRESS_TIMEOUT) {
    if (get_highest_layer(layer_state) <= 1) {
      layer_change_reason = "K5(Hold): Toggle L5 (ON)";
      layer_move(5);
    } else {
      layer_change_reason = "K5(Hold): Toggle L5 (OFF)";
      layer_move(0);
    }
    th[TH_K5].triggered = true;
    housekeeping_rgb_indicators();
  }

  // New Layer 4 Keys Logic
  if (th[TH_MINS].held && !th[TH_MINS].triggered &&
      timer_elapsed(th[TH_MINS].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "MINS(Hold): L4->Base";
    layer_move(0);
    th[TH_MINS].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_K0].held && !th[TH_K0].triggered &&
      timer_elapsed(th[TH_K0].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "K0(Hold): L4->L1 (Jump)";
    layer_move(1);
    th[TH_K0].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_K9].held && !th[TH_K9].triggered &&
      timer_elapsed(th[TH_K9].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "K9(Hold): L4->L2 (Jump)";
    layer_move(2);
    th[TH_K9].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_K8].held && !th[TH_K8].triggered &&
      timer_elapsed(th[TH_K8].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "K8(Hold): L4->L3 (Jump)";
    layer_move(3);
    th[TH_K8].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_K7].held && !th[TH_K7].triggered &&
      timer_elapsed(th[TH_K7].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "K7(Hold): L4->Base";
    layer_move(0);
    th[TH_K7].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_K6].held && !th[TH_K6].triggered &&
      timer_elapsed(th[TH_K6].timer) > LONG_PRESS_TIMEOUT) {
    layer_change_reason = "K6(Hold): L4->Base";
    layer_move(0);
    th[TH_K6].triggered = true;
    housekeeping_rgb_indicators();
  }

  // Thumb Toggle Logic
  if (th[TH_ENT_TG2].held && !th[TH_ENT_TG2].triggered &&
      timer_elapsed(th[TH_ENT_TG2].timer) > LONG_PRESS_TIMEOUT) {
    uprintf(">>> Thumb Hold Triggered: ENT_TG2 -> Toggle Layer 2\n");
    if (get_highest_layer(layer_state) == 2) {
      layer_change_reason = "ENT(Hold): Toggle L2 (OFF)";
      layer_move(0);
    } else {
      layer_change_reason = "ENT(Hold): Toggle L2 (ON)";
      layer_move(2);
    }
    th[TH_ENT_TG2].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_ENT_TG4].held && !th[TH_ENT_TG4].triggered &&
      timer_elapsed(th[TH_ENT_TG4].timer) > LONG_PRESS_TIMEOUT) {
    uprintf(">>> Thumb Hold Triggered: ENT_TG4 -> Toggle Layer 4\n");
    if (get_highest_layer(layer_state) == 4) {
      layer_change_reason = "ENT(Hold): Toggle L4 (OFF)";
      layer_move(0);
    } else {
      layer_change_reason = "ENT(Hold): Toggle L4 (ON)";
      layer_move(4);
    }
    th[TH_ENT_TG4].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_SPC_TG2].held && !th[TH_SPC_TG2].triggered &&
      timer_elapsed(th[TH_SPC_TG2].timer) > LONG_PRESS_TIMEOUT) {
    uprintf(">>> Thumb Hold Triggered: SPC_TG2 -> Toggle Layer 2\n");
    if (get_highest_layer(layer_state) == 2) {
      layer_change_reason = "SPC(Hold): Toggle L2 (OFF)";
      layer_move(0);
    } else {
      layer_change_reason = "SPC(Hold): Toggle L2 (ON)";
      layer_move(2);
    }
    th[TH_SPC_TG2].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_SPC_TG4].held && !th[TH_SPC_TG4].triggered &&
      timer_elapsed(th[TH_SPC_TG4].timer) > LONG_PRESS_TIMEOUT) {
    uprintf(">>> Thumb Hold Triggered: SPC_TG4 -> Toggle Layer 4\n");
    if (get_highest_layer(layer_state) == 4) {
      layer_change_reason = "SPC(Hold): Toggle L4 (OFF)";
      layer_move(0);
    } else {
      layer_change_reason = "SPC(Hold): Toggle L4 (ON)";
      layer_move(4);
    }
    th[TH_SPC_TG4].triggered = true;
    housekeeping_rgb_indicators();
  }
  if (th[TH_F12].held && !th[TH_F12].triggered &&
      timer_elapsed(th[TH_F12].timer) > LONG_PRESS_TIMEOUT) {
    uprintf(">>> Thumb Hold Triggered: F12 -> Exit to Base\n");
    layer_change_reason = "F12(Hold): Exit to Base";
    layer_move(0);
    th[TH_F12].triggered = true;
    housekeeping_rgb_indicators();
  }

  if (TH_CHECK(TH_K0_TO0)) {
    uprintf(">>> KC_0_TO0 Hold Triggered -> Layer 0\n");
    layer_change_reason = "0(Hold): Layer 0";
    layer_move(0);
    TH_TRIGGER(TH_K0_TO0);
    housekeeping_rgb_indicators();
  }

  if (TH_CHECK(TH_K6_TO6)) {
    uprintf(">>> KC_6_L6 Hold Triggered -> Layer 6\n");
    layer_change_reason = "6(Hold): Layer 6";
    layer_move(6);
    TH_TRIGGER(TH_K6_TO6);
    housekeeping_rgb_indicators();
  }
  if (TH_CHECK(TH_EXIT_TO3)) {
    uprintf(">>> KC_EXIT_TO3 Hold Triggered -> Layer 3\n");
    layer_change_reason = "EXIT(Hold): Layer 3";
    layer_move(3);
    TH_TRIGGER(TH_EXIT_TO3);
    housekeeping_rgb_indicators();
  }
  if (TH_CHECK(TH_Q_Z)) {
    uprintf(">>> KC_Q_Z Hold Triggered -> Z\n");
    register_code(KC_Z);
    TH_TRIGGER(TH_Q_Z);
  }
  if (TH_CHECK(TH_W_X)) {
    uprintf(">>> KC_W_X Hold Triggered -> X\n");
    register_code(KC_X);
    TH_TRIGGER(TH_W_X);
  }
  if (TH_CHECK(TH_K7_L7)) {
    uprintf(">>> KC_7_L7 Hold Triggered -> Layer 7 (Right Arrows)\n");
    layer_change_reason = "7(Hold): Layer 7 (Right Arrows)";
    layer_move(7);
    TH_TRIGGER(TH_K7_L7);
    housekeeping_rgb_indicators();
  }
}
