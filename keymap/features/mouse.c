#include "mouse.h"
#include "sync.h"
#include "logging.h"
#include "tap_hold.h"
#include "keycodes.h"
#include <lib/lib8tion/lib8tion.h>

// Mouse state definitions
bool     is_sniping_active       = false;
bool     is_fast_mode_active     = false;
bool     mouse_is_locked         = false;
bool     is_jitter_filter_active = false;
bool     layer3_auto_activated   = false;
uint16_t auto_mouse_timer        = 0;
uint16_t auto_mouse_timeout      = 1500; // Default (matches AUTO_MOUSE_TIME in config.h)

// Internal timers
static uint16_t snipe_timer         = 0;
static uint16_t fast_mode_timer     = 0;
uint8_t         mouse_buttons_held  = 0;
bool            is_selection_locked = false;
bool            is_drag_scroll_active = false;

void update_mouse_button_state(uint16_t keycode, bool pressed) {
  uint8_t mask = 0;
  switch (keycode) {
    case MS_BTN1: mask = 1 << 0; break;
    case MS_BTN2: mask = 1 << 1; break;
    case MS_BTN3: mask = 1 << 2; break;
    case MS_BTN4: mask = 1 << 3; break;
    case MS_BTN5: mask = 1 << 4; break;
    case MS_BTN6: mask = 1 << 5; break;
    case MS_BTN7: mask = 1 << 6; break;
    case MS_BTN8: mask = 1 << 7; break;
    default:                    return;
  }

  if (pressed) {
    mouse_buttons_held |= mask;
    // Reset timer immediately on press/hold event
    if (layer3_auto_activated) {
      auto_mouse_timer = timer_read();
    }
  } else {
    mouse_buttons_held &= ~mask;
  }
}

// Mouse Key globals logic
// Fast mouse movement helper
bool handle_fast_mouse(uint16_t direction, bool pressed) {
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
bool handle_diag_mouse(uint16_t dir1, uint16_t dir2, bool pressed) {
  if (pressed) {
    register_code(dir1);
    register_code(dir2);
  } else {
    unregister_code(dir1);
    unregister_code(dir2);
  }
  return false;
}

// Process mouse timeouts (called from matrix_scan_user)
void process_mouse_timeouts(void) {
  // Timeout for Snipe Mode
  if (is_sniping_active && timer_elapsed(snipe_timer) > SNIPE_MODE_TIMEOUT) {
    is_sniping_active = false;
    pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
    mk_max_speed = MK_MAX_SPEED_DEFAULT;
    mk_interval  = MK_INTERVAL_DEFAULT;
    sync_needed  = true;
    uprintf("Snipe Mode Timeout. CPI -> Default\n");
  }

  // Timeout for Fast Mode
  if (is_fast_mode_active && timer_elapsed(fast_mode_timer) > FAST_MODE_TIMEOUT) {
    is_fast_mode_active = false;
    pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
    mk_max_speed = MK_MAX_SPEED_DEFAULT;
    mk_interval  = MK_INTERVAL_DEFAULT;
    sync_needed  = true;
    uprintf("Fast Mode Timeout. CPI -> Default\n");
  }

  static char mouse_reason_buffer[64];

  // Keep layer 3 alive if:
  // 1. Any mouse button is held
  // 2. Selection Lock is active
  // 3. Drag Scroll is active
  // 4. Scroll Lock LED is on
  bool lock_layer = false;
  if (mouse_buttons_held) lock_layer = true;
  if (is_selection_locked) lock_layer = true;
  if (is_drag_scroll_active) lock_layer = true;
  if (host_keyboard_led_state().scroll_lock) lock_layer = true;

  if (lock_layer && layer3_auto_activated) {
    auto_mouse_timer = timer_read();
  }

  if (layer3_auto_activated && !mouse_is_locked && timer_elapsed(auto_mouse_timer) > auto_mouse_timeout) {
    snprintf(mouse_reason_buffer, sizeof(mouse_reason_buffer), "Auto Mouse Timeout (%u ms)", auto_mouse_timeout);
    layer_change_reason = mouse_reason_buffer;
    layer_history_pop(); // Restore previous layer state
    layer3_auto_activated = false;
    mouse_buttons_held    = 0; // Failsafe: clear held status if we force layer off
  }
}

// Main mouse processing task (called from pointing_device_task_user)

report_mouse_t housekeeping_mouse_task(report_mouse_t mouse_report) {
  int8_t x                = mouse_report.x;
  int8_t y                = mouse_report.y;
  int8_t v                = mouse_report.v;
  int8_t h                = mouse_report.h;

  // Log state change for Scroll Mode
  static bool last_scroll_state = false;
  if (is_drag_scroll_active != last_scroll_state) {
    uprintf("\033[93mMouse: Drag Scroll %s\033[0m\n", is_drag_scroll_active ? "ENABLED" : "DISABLED");
    last_scroll_state = is_drag_scroll_active;
    if (is_drag_scroll_active) {
      pointing_device_set_cpi(100); // Low CPI for scroll
    } else {
      pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
    }
  }

  // Custom Drag Scroll Logic (Accumulated with Remainder)
  if (is_drag_scroll_active) {
    static int16_t scroll_remainder_x = 0;
    static int16_t scroll_remainder_y = 0;

    // Accumulate movement
    scroll_remainder_x += x;
    scroll_remainder_y += y;

    // Divisor: Higher = Slower/Finer, Lower = Faster
    const int16_t scroll_divisor = CHARYBDIS_DRAGSCROLL_DIVISOR;

    int8_t scroll_h = scroll_remainder_x / scroll_divisor;
    int8_t scroll_v = scroll_remainder_y / scroll_divisor;

    // Update remainders
    scroll_remainder_x -= scroll_h * scroll_divisor;
    scroll_remainder_y -= scroll_v * scroll_divisor;

    // Apply scroll (Inverted for natural feel)
    mouse_report.h = -scroll_h;
    mouse_report.v = -scroll_v; // Inverted Y

    // Zero out cursor movement
    mouse_report.x = 0;
    mouse_report.y = 0;
    x = 0;
    y = 0;
    v = mouse_report.v;
    h = mouse_report.h;
  }

// ============================================================
// PRECISION DECELERATION SYSTEM
// ============================================================
// Problem: Trackball is sensitive. When making tiny movements for precision
// (e.g., clicking a small button), the cursor can still move too fast.
//
// Solution: Detect sustained small movements and automatically slow down
// the cursor to help with precise positioning.
//
// How it works:
// 1. Track movement magnitude: small = abs(x) + abs(y) ≤ 2 pixels
// 2. Start timer when small movements begin
// 3. After 200ms of continuous small movements, halve the speed
// 4. Reset to normal speed if a larger movement (>2 pixels) occurs
//
// Example: Hovering over a checkbox. You make micro-adjustments (1-2px).
// After ~200ms, cursor slows 50%, making it easier to position precisely.
// ============================================================
#define SMALL_MOVEMENT_THRESHOLD 2  // Magnitude (abs(x)+abs(y)) considered "small"
#define SMALL_MOVEMENT_DURATION 200 // ms of consecutive small movements before decelerating

  static uint16_t small_movement_start   = 0;     // Timer for small movement duration
  static bool     in_small_movement_mode = false; // Are we in small movement mode?

  if (x != 0 || y != 0) {
    uint8_t magnitude = abs(x) + abs(y);

    if (magnitude <= SMALL_MOVEMENT_THRESHOLD) {
      // Small movement detected - track how long we've been doing this
      if (!in_small_movement_mode) {
        in_small_movement_mode = true;
        small_movement_start   = timer_read();
      }

      if (timer_elapsed(small_movement_start) > SMALL_MOVEMENT_DURATION) {
        // Been doing small movements for 200ms+ - slow down for precision
        static uint32_t last_decel_log = 0;
        if (timer_elapsed32(last_decel_log) > 1000) {
          uprintf("\033[94mMouse: Decelerating (small movements for %u ms, mag=%d)\033[0m\n", timer_elapsed(small_movement_start), magnitude);
          last_decel_log = timer_read32();
        }
        // Decelerate: halve the speed, but preserve minimum movement of 1
        // (x/2) preserves direction, abs(x) > 1 prevents 1 from becoming 0
        if (x != 0) mouse_report.x = (abs(x) > 1) ? (x / 2) : x;
        if (y != 0) mouse_report.y = (abs(y) > 1) ? (y / 2) : y;

        x = mouse_report.x;
        y = mouse_report.y;
      }
    } else {
      // Larger movement - user is intentionally moving fast, reset precision mode
      in_small_movement_mode = false;
    }
  }

  uint32_t now = timer_read32();
  uint32_t sec = now / 1000;
  uint32_t ms  = now % 1000;

  // Log movement/scroll
  if (x != 0 || y != 0 || v != 0 || h != 0) {
    static uint32_t last_move_log = 0;
    if (timer_elapsed32(last_move_log) > 1000) { // Log at most every 1s
      if (is_drag_scroll_active) {
        uprintf("[%lu.%03lu] Mouse Scroll: v=%d, h=%d\n", sec, ms, v, h);
      } else if (get_highest_layer(layer_state) == 3) {
        uint16_t current_cpi = pointing_device_get_cpi();
        uprintf("[%lu.%03lu] Mouse Move (L3): x=%d, y=%d, CPI=%u\n", sec, ms, x, y, current_cpi);
      }
      last_move_log = now;
    }
  }

  uint8_t        movement          = abs(x) + abs(y);
  bool           is_small_movement = movement == 1;
  static uint8_t jitter_streak     = 0;

  if (is_jitter_filter_active && !layer_state_is(3)) {
    if (movement == 2) {
      // uprintf("\033[90mMouse: Jitter Detected (x=%d, y=%d)\033[0m\n", x, y);
      static uint32_t last_x2_streak = 0;
      int             timer_elapsed  = timer_elapsed32(last_x2_streak);
      if (timer_elapsed > 25) {
        uprintf("\033[90mMouse: Jitter filtered for x=%d, y=%d (timer elapsed %d)\033[0m\n", x, y, timer_elapsed);
        movement          = 1;
        is_small_movement = true;
      }
      last_x2_streak = now;
    }
#define MOUSE_JITTER_STREAK_INTERVAL 10  // Max ms between small movements to count as a streak
#define MOUSE_JITTER_STREAK_THRESHOLD 60 // Number of small movements to establish a streak

    // Filter out 1-unit movements (jitter)
    // Keep diagonals (e.g. x=1, y=1) or larger movements

    if (is_small_movement) {
      static uint32_t last_jitter_time = 0;
      // Check if this small movement occurred very soon after the previous one.
      // High-frequency small movements (interval < MOUSE_JITTER_STREAK_INTERVAL) are likely intentional
      // slow tracking rather than random sensor noise.
      if (timer_elapsed32(last_jitter_time) < MOUSE_JITTER_STREAK_INTERVAL) {
        if (jitter_streak < 255) jitter_streak++;
      } else {
        // If there's a gap, reset the streak; it's considered isolated jitter again.
        /*
        static uint32_t mouse_streak_reset_log = 0;
        if (jitter_streak > 0 && timer_elapsed32(mouse_streak_reset_log) > 1000) {
          uprintf("\033[90mMouse: Streak Reset (was %d)\033[0m\n", jitter_streak);
          mouse_streak_reset_log = now;
        }
        */
        jitter_streak = 0;
      }
      last_jitter_time = now;

      // We only suppress the movement if it hasn't established a "streak".
      // A streak of MOUSE_JITTER_STREAK_THRESHOLD+ events means the user is consistently moving the ball slowly,
      // so we stop filtering and let the movement through to the host.
      if (jitter_streak < MOUSE_JITTER_STREAK_THRESHOLD) {
        static uint32_t last_jitter_log = 0;
        if (timer_elapsed32(last_jitter_log) > 1000) {
          LOG_TIME();
          uprintf("\033[95mMouse: Jitter Filtered (x=%d, y=%d, streak=%d)\033[0m\n", x, y, jitter_streak);
          last_jitter_log = now;
        }
        // Zero out the report so it's ignored by the host AND by the auto-layer logic below
        mouse_report.x = 0;
        mouse_report.y = 0;
        x              = 0;
        y              = 0;
      } else {
        // Log when we break the filter
        static uint32_t last_logged_streak = 0;
        if (jitter_streak >= MOUSE_JITTER_STREAK_THRESHOLD && timer_elapsed32(last_logged_streak) > 500) {
          uprintf("\033[92mMouse: Streak Established (%d), Filter Broken\033[0m\n", jitter_streak);
          last_logged_streak = now;
        }
      }
    }
  }

  bool trackball_significant_move = ( layer_state_is(3)       && movement > 0) ||
                                    (!is_jitter_filter_active && movement > 0) ||
                                    (                            movement > 1) ||
                                    ( is_small_movement       && jitter_streak > MOUSE_JITTER_STREAK_THRESHOLD);
  // Only auto activate layer for significant movement
  if (trackball_significant_move) {
    if (layer3_auto_activated) {
      auto_mouse_timer = timer_read(); // Movement extends the timer
    }
    static uint32_t last_logged_movement = 0;
    if (timer_elapsed32(last_logged_movement) > 500) {
      if (!layer_state_is(3)) {
        LOG_TIME();
        uprintf("jitter Streak: %d ", jitter_streak);
      }
      uprintf("movement: %d, (x=%d, y=%d). \n", movement, x, y);
      last_logged_movement = timer_read32();
    }
    if (is_keyboard_master()) {
      // Only activate Auto Mouse if Layer 3 isn't already active (e.g. manually locked)
      if (!layer_state_is(3)) {
        uprintf("Mouse: Activated (x=%d, y=%d), jitter streak: %d, movement: %d, is jitter filter active: %d, is small movement: %d\n", x, y, jitter_streak, movement, is_jitter_filter_active, is_small_movement);
        layer_change_reason = "Auto Mouse Movement";
        layer_move(3); // Switch exclusively to Mouse Layer (3)
        layer3_auto_activated = true;
        auto_mouse_timer      = timer_read();
      }
    } else {
      slave_mouse_active = true; // Slave side: Signal master
    }
  }
  return mouse_report;
}

bool process_mouse_keycodes(uint16_t keycode, keyrecord_t *record) {
  if (!record->event.pressed) {
    // Handle release for all context mouse keys by unregistering everything potential
    // This is safer than tracking state, as unregistering a key not pressed is a no-op
    switch (keycode) {
      case CM_MS_UP:
        unregister_code(MS_UP);   unregister_code(MS_WHLU); return false;
      case CM_MS_DOWN:
        unregister_code(MS_DOWN); unregister_code(MS_WHLD); return false;
      case CM_MS_LEFT:
        unregister_code(MS_LEFT); unregister_code(MS_WHLL); return false;
      case CM_MS_RIGHT:
        unregister_code(MS_RGHT); unregister_code(MS_WHLR); return false;
      // Diagonals
      case CM_MS_UL:
        unregister_code(MS_UP);   unregister_code(MS_LEFT);
        unregister_code(MS_WHLU); unregister_code(MS_WHLL); return false;
      case CM_MS_UR:
        unregister_code(MS_UP);   unregister_code(MS_RGHT);
        unregister_code(MS_WHLU); unregister_code(MS_WHLR); return false;
      case CM_MS_DL:
        unregister_code(MS_DOWN); unregister_code(MS_LEFT);
        unregister_code(MS_WHLD); unregister_code(MS_WHLL); return false;
      case CM_MS_DR:
        unregister_code(MS_DOWN); unregister_code(MS_RGHT);
        unregister_code(MS_WHLD); unregister_code(MS_WHLR); return false;
    }
    return true; // Not a CM key
  }

  // Key Pressed
  bool drag_scroll = is_drag_scroll_active;

  switch (keycode) {
    case SCROLL_MODE:
      if (record->event.pressed) {
        is_drag_scroll_active = !is_drag_scroll_active;
      }
      return false;
    case DRGSCRL:
      is_drag_scroll_active = record->event.pressed;
      return false;

    case CM_MS_UP:
      if (drag_scroll) register_code(MS_WHLU);
      else             register_code(MS_UP  ); return false;
    case CM_MS_DOWN:
      if (drag_scroll) register_code(MS_WHLD);
      else             register_code(MS_DOWN); return false;
    case CM_MS_LEFT:
      if (drag_scroll) register_code(MS_WHLL);
      else             register_code(MS_LEFT); return false;
    case CM_MS_RIGHT:
      if (drag_scroll) register_code(MS_WHLR);
      else             register_code(MS_RGHT); return false;
    // Diagonals
    case CM_MS_UL:
      if (drag_scroll) { register_code(MS_WHLU); register_code(MS_WHLL); }
      else             { register_code(MS_UP  ); register_code(MS_LEFT); } return false;
    case CM_MS_UR:
      if (drag_scroll) { register_code(MS_WHLU); register_code(MS_WHLR); }
      else             { register_code(MS_UP  ); register_code(MS_RGHT); } return false;
    case CM_MS_DL:
      if (drag_scroll) { register_code(MS_WHLD); register_code(MS_WHLL); }
      else             { register_code(MS_DOWN); register_code(MS_LEFT); } return false;
    case CM_MS_DR:
      if (drag_scroll) { register_code(MS_WHLD); register_code(MS_WHLR); }
      else             { register_code(MS_DOWN); register_code(MS_RGHT); } return false;
  }

  return true; // Not a CM key
}

// Handlers for Keycode-triggered mouse actions (SNIPE, FAST)
// Note: These helpers update state variables that are now extern
void activate_snipe_mode(void) {
  uint32_t now        = timer_read32();
  is_sniping_active   = true;
  is_fast_mode_active = false;
  snipe_timer         = timer_read();
  sync_needed         = true;
  pointing_device_set_cpi(250);
  mk_max_speed = 1; // Very slow mouse keys
  mk_interval  = 24;
  uprintf("[%lu.%03lu] Snipe ON (2.5s). CPI -> 250, MK -> Slow(1)\n", now / 1000, now % 1000);
}

void activate_fast_mode(void) {
  uint32_t now        = timer_read32();
  is_fast_mode_active = true;
  is_sniping_active   = false;
  fast_mode_timer     = timer_read();
  sync_needed         = true;
  pointing_device_set_cpi(3000);
  mk_max_speed = 30; // Fast mouse keys
  mk_interval  = 10;
  uprintf("[%lu.%03lu] Fast Mode ON (2.5s). CPI -> 3000, MK -> Fast\n", now / 1000, now % 1000);
}

// Unified function to clear all mouse-related states on layer exit
void clear_mouse_states(void) {
  uprintf("Clear Mouse States: Locked=%d, SelLock=%d, DragScroll=%d, Snipe=%d, Fast=%d\n",
          mouse_is_locked, is_selection_locked, is_drag_scroll_active, is_sniping_active, is_fast_mode_active);

  mouse_is_locked = false;
  if (is_selection_locked) {
    unregister_code(MS_BTN1);
    is_selection_locked = false;
    update_mouse_button_state(MS_BTN1, false);
  }
  
  // Clear Drag Scroll
  if (is_drag_scroll_active) {
    is_drag_scroll_active = false;
    pointing_device_set_cpi(charybdis_get_pointer_default_dpi()); // Ensure CPI resets
  }
  
  // Clear Snipe Mode
  if (is_sniping_active) {
    is_sniping_active = false;
    pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
    mk_max_speed = MK_MAX_SPEED_DEFAULT;
    mk_interval  = MK_INTERVAL_DEFAULT;
  }
  
  // Clear Fast Mode
  if (is_fast_mode_active) {
    is_fast_mode_active = false;
    pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
    mk_max_speed = MK_MAX_SPEED_DEFAULT;
    mk_interval  = MK_INTERVAL_DEFAULT;
  }
  
  layer3_auto_activated = false;
  sync_needed           = true;
}
