#include "mouse.h"
#include "sync.h"
#include "logging.h"
#include <lib/lib8tion/lib8tion.h>

// Mouse state definitions
bool is_sniping_active = false;
bool is_fast_mode_active = false;
bool mouse_is_locked = false;
bool is_jitter_filter_active = false;
bool layer3_auto_activated = false;
uint16_t auto_mouse_timer = 0;
uint16_t auto_mouse_timeout = 1500; // Default (matches AUTO_MOUSE_TIME in config.h)

// Internal timers
static uint16_t snipe_timer = 0;
static uint16_t fast_mode_timer = 0;
uint8_t mouse_buttons_held = 0;
bool is_selection_locked = false;

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
    default: return;
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
        mk_max_speed = 12; // Restore default
        mk_interval = 16;
        sync_needed = true;
        uprintf("Snipe Mode Timeout. CPI -> Default\n");
    }

    // Timeout for Fast Mode
    if (is_fast_mode_active && timer_elapsed(fast_mode_timer) > FAST_MODE_TIMEOUT) {
        is_fast_mode_active = false;
        pointing_device_set_cpi(charybdis_get_pointer_default_dpi());
        mk_max_speed = 12; // Restore default
        mk_interval = 16;
        sync_needed = true;
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
    if (charybdis_get_pointer_dragscroll_enabled()) lock_layer = true;
    if (host_keyboard_led_state().scroll_lock) lock_layer = true;

    if (lock_layer && layer3_auto_activated) {
        auto_mouse_timer = timer_read();
    }

    if (layer3_auto_activated && !mouse_is_locked &&
        timer_elapsed(auto_mouse_timer) > auto_mouse_timeout) {
        snprintf(mouse_reason_buffer, sizeof(mouse_reason_buffer), "Auto Mouse Timeout (%u ms)", auto_mouse_timeout);
        layer_change_reason = mouse_reason_buffer;
        layer_off(3);
        layer3_auto_activated = false;
        mouse_buttons_held = 0; // Failsafe: clear held status if we force layer off
    }
}

// Main mouse processing task (called from pointing_device_task_user)
#define MOUSE_JITTER_STREAK_INTERVAL  10  // Max ms between small movements to count as a streak
#define MOUSE_JITTER_STREAK_THRESHOLD 60  // Number of small movements to establish a streak

report_mouse_t housekeeping_mouse_task(report_mouse_t mouse_report) {
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

  uint8_t movement = abs(x) + abs(y);
  bool is_small_movement = movement == 1;
  static uint8_t jitter_streak = 0;

  if (is_jitter_filter_active) {
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
        static uint32_t mouse_streak_reset_log = 0;
        if (jitter_streak > 0 && timer_elapsed32(mouse_streak_reset_log) > 1000) {
          uprintf("\033[90mMouse: Streak Reset (was %d)\033[0m\n", jitter_streak);
          mouse_streak_reset_log = now;
        }
        jitter_streak = 0;
      }
      last_jitter_time = now;

      // We only suppress the movement if it hasn't established a "streak".
      // A streak of MOUSE_JITTER_STREAK_THRESHOLD+ events means the user is consistently moving the ball slowly,
      // so we stop filtering and let the movement through to the host.
      if (jitter_streak < MOUSE_JITTER_STREAK_THRESHOLD) {
        static uint32_t last_jitter_log = 0;
        if (timer_elapsed32(last_jitter_log) > 500) {
          LOG_TIME();
          uprintf("\033[95mMouse: Jitter Filtered (x=%d, y=%d, streak=%d)\033[0m\n", x, y, jitter_streak);
          last_jitter_log = now;
        }
        // Zero out the report so it's ignored by the host AND by the auto-layer logic below
        mouse_report.x = 0;
        mouse_report.y = 0;
        x = 0;
        y = 0;
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

  bool did_trackball_move = (!is_jitter_filter_active &&  movement > 0) ||
                            ( is_jitter_filter_active && (movement > 1 || (is_small_movement && jitter_streak > MOUSE_JITTER_STREAK_THRESHOLD)));
  // Only auto activate layer for significant movement
  if (did_trackball_move) {
    if (layer3_auto_activated) {
      auto_mouse_timer = timer_read();      // Movement extends the timer
    }
    static uint32_t last_logged_movement = 0;
    if (timer_elapsed32(last_logged_movement) > 500) {
      uprintf("movement: %d, jitter Streak: %d (x=%d, y=%d). Master: %d. L3: %d\n",
               movement, MOUSE_JITTER_STREAK_THRESHOLD, x, y, is_keyboard_master(), layer_state_is(3));
      last_logged_movement = timer_read32();
    }
    if (is_keyboard_master()) {
      // Only activate Auto Mouse if Layer 3 isn't already active (e.g. manually locked)
      if (!layer_state_is(3)) {
        uprintf("Mouse: Activated (x=%d, y=%d), jitter streak: %d, movement: %d, is jitter filter active: %d, is small movement: %d\n",
                x, y, jitter_streak, movement, is_jitter_filter_active, is_small_movement);
        layer_change_reason = "Auto Mouse Movement";
        layer_on(3); // Switch to Mouse Layer (3)
        layer3_auto_activated = true;
        auto_mouse_timer = timer_read();
      }
    } else {
      slave_mouse_active = true;          // Slave side: Signal master
    }
  }
  return mouse_report;
}

// Handlers for Keycode-triggered mouse actions (SNIPE, FAST)
// Note: These helpers update state variables that are now extern
void activate_snipe_mode(void) {
    uint32_t now = timer_read32();
    is_sniping_active = true;
    is_fast_mode_active = false;
    snipe_timer = timer_read();
    sync_needed = true;
    pointing_device_set_cpi(250);
    mk_max_speed = 1; // Very slow mouse keys
    mk_interval = 24;
    uprintf("[%lu.%03lu] Snipe ON (2.5s). CPI -> 250, MK -> Slow(1)\n", now/1000, now%1000);
}

void activate_fast_mode(void) {
    uint32_t now = timer_read32();
    is_fast_mode_active = true;
    is_sniping_active = false;
    fast_mode_timer = timer_read();
    sync_needed = true;
    pointing_device_set_cpi(3000);
    mk_max_speed = 30; // Fast mouse keys
    mk_interval = 10;
    uprintf("[%lu.%03lu] Fast Mode ON (2.5s). CPI -> 3000, MK -> Fast\n", now/1000, now%1000);
}
