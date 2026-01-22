#pragma once

#include QMK_KEYBOARD_H

#define SCROLL_MODE DRG_TOG // Toggle Drag Scroll Mode (On/Off)
// Note: DRGSCRL is the Momentary (Hold) version, defined in charybdis.h

// Custom Keycodes
enum custom_keycodes {
  KC_RAINBOW = QK_USER_0,
  KC_REACTIVE,
  KC_MOUSE_LOCK,
  KC_ENT_L2_EXIT,
  KC_PGUP_TO0,
  KC_P_TO0,
  KC_HOME_TO0,
  KC_L1_L3,
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
  KC_DUMP_LOG,       // Debug key to dump sync state
  KC_LOG_SYNC_DEBUG,   // Toggle Debug Logging
  KC_JITTER,         // Toggle Mouse Jitter Filter
  KC_DAY,            // Set brightness to Day level
  KC_NIGHT,          // Set brightness to Night level
  KC_FLASHLIGHT,     // Toggle Flashlight mode
  KC_SLSH_TO0,       // Tap: /, Hold: Temporary Layer 0
  KC_L3_EXT_TO4,     // Layer 3 Left Thumb: Tap Exit, Hold TO(4)
  KC_L3_EXT_TO2,     // Layer 3 Left Thumb: Tap Exit, Hold TO(2)
  KC_L3_EXT_TO1,     // Layer 3 Left Thumb: Tap TO(1), Hold TO(6)
  KC_EXIT_TO3,       // Layer 1 Left Thumb: Tap Exit, Hold TO(3)
  KC_PRINT_STATS,    // Print statistics histogram immediately
  KC_LOG_STATS_GRID, // Print statistics in a 2D grid format
  KC_5_L5,           // Tap: 5, Hold: Layer 5 (Settings)
  KC_0_TO0,          // Tap: 0, Hold: TO(0)
  KC_SEL_LOCK,       // Toggle Left Click Hold (Selection Lock)
  KC_Q_Z,            // Tap: Q, Hold: Z
  KC_W_X,            // Tap: W, Hold: X
  KC_7_L6,           // Tap: 7, Hold: Layer 6 (Right Arrows)
  KC_MINS_EQL,       // Tap: Minus, Hold: Equals
  KC_F1_F11,         // Tap: F1, Hold: F11
  KC_F2_F12,         // Tap: F2, Hold: F12

  // Context-aware Mouse Keys (Drag Scroll override)
  CM_MS_UP,
  CM_MS_DOWN,
  CM_MS_LEFT,
  CM_MS_RIGHT,
  CM_MS_UL, // Up-Left
  CM_MS_UR, // Up-Right
  CM_MS_DL, // Down-Left
  CM_MS_DR  // Down-Right
};
