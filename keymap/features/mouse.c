#include "mouse.h"
#include "sync.h"
#include <lib/lib8tion/lib8tion.h>

// Helper for logging (temporary duplication until a common logger is established)
#define LOG_TIME() do { \
    uint32_t _t = timer_read32(); \
    uprintf("[%lu.%03lu] ", (unsigned long)(_t / 1000), (unsigned long)(_t % 1000)); \
} while(0)

// Internal timers
static uint16_t snipe_timer = 0;
static uint16_t fast_mode_timer = 0;

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

    static char mouse_reason_buffer[64];
    if (auto_mouse_on && !mouse_is_locked &&
        timer_elapsed(auto_mouse_timer) > auto_mouse_timeout) {
        snprintf(mouse_reason_buffer, sizeof(mouse_reason_buffer), "Auto Mouse Timeout (%u ms)", auto_mouse_timeout);
        layer_change_reason = mouse_reason_buffer;
        layer_off(3);
        auto_mouse_on = false;
    }
}

// Main mouse processing task (called from pointing_device_task_user)
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
