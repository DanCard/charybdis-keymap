#pragma once

#include QMK_KEYBOARD_H

// Custom Keycodes
enum custom_keycodes {
  KC_RAINBOW = QK_USER_0,
  KC_REACTIVE,
  KC_MOUSE_LOCK,
  KC_ENT_L2_EXIT,
  KC_PGUP_TO0,
  KC_P_TO0,
  KC_HOME_TO0,
  KC_L_L1,
  KC_R_L2,
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
  KC_1_L1,
  KC_2_L2,
  KC_3_L3,
  KC_4_L4,
  KC_Q_L4,
  KC_JELLY,
  KC_SPIRAL,
  KC_CHEVRON,
  KC_RGB_AUTO,
  KC_PLUS_COLON,
  KC_MINS_TO0,
  KC_0_L1,
  KC_9_L2,
  KC_8_L3,
  KC_7_TO0,
  KC_6_TO0,
  KC_ENT_L2,
  KC_ENT_L4,
  KC_SPC_L2,
  KC_SPC_L4,
  KC_PMNS_L4,
  KC_MINS_L4,
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
  KC_DAY,         // Set brightness to Day level
  KC_NIGHT,       // Set brightness to Night level
  KC_FLASHLIGHT,  // Toggle Flashlight mode
  KC_SLSH_TO0,    // Tap: /, Hold: Temporary Layer 0
  KC_5_L5,        // Tap: 5, Hold: Layer 5 (Settings)
  KC_L3_EXT_TO4,  // Layer 3 Left Thumb: Tap Exit, Hold TO(4)
  KC_L3_EXT_TO2,  // Layer 3 Left Thumb: Tap Exit, Hold TO(2)
  KC_L3_EXT_TO1,  // Layer 3 Left Thumb: Tap Exit, Hold TO(1)
  KC_PRINT_STATS, // Print statistics histogram immediately
  KC_PRINT_STATS_GRID, // Print statistics in a 2D grid format
  KC_2_LEFT,      // Tap: Left, Shift+Tap: 2
  KC_3_UP,        // Tap: Up,   Shift+Tap: 3
  KC_4_DOWN,      // Tap: Down, Shift+Tap: 4
  KC_5_RIGHT,     // Tap: Right, Shift+Tap: 5
  KC_6_LEFT,      // Tap: Left, Shift+Tap: 6
  KC_UP_7,        // Tap: Up,   Shift+Tap: 7
  KC_DOWN_8,      // Tap: Down, Shift+Tap: 8
  KC_RIGHT_9,     // Tap: Right, Shift+Tap: 9
  KC_LEFT_2,      // Tap: Left,  Shift+Tap: 2, Hold: TO(2)
  KC_UP_3,        // Tap: Up,    Shift+Tap: 3, Hold: TO(3)
  KC_DOWN_4,      // Tap: Down,  Shift+Tap: 4, Hold: TO(4)
  KC_RIGHT_5,     // Tap: Right, Shift+Tap: 5, Hold: TO(5)
  KC_LEFT_6,      // Tap: Left,  Shift+Tap: 6, Hold: TO(6)
  KC_6_L6,        // Tap: 6, Hold: Layer 6
  KC_0_TO0        // Tap: 0, Hold: TO(0)
};
