#pragma once

#include QMK_KEYBOARD_H
#include "transactions.h"

// Sync statistics tracking
extern uint32_t sync_success_count;
extern uint32_t sync_fail_count;
extern uint32_t last_sync_time;
extern uint32_t last_heartbeat_time;

#define HEARTBEAT_INTERVAL 15000  // 15 seconds

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

typedef struct _user_sync_info_response_t {
  bool did_rgb_sync;
  uint8_t slave_rgb_mode;
  uint16_t slave_task_counter;
  bool mouse_active;
} user_sync_info_response_t;

// Globals needed by sync logic (defined in keymap.c or other features)
extern bool is_flashlight;
extern bool is_sniping_active;
extern bool is_fast_mode_active;
extern bool mouse_is_locked;
extern bool is_jitter_filter_active;
extern bool is_caps_lock_on;
extern bool sync_needed;
extern bool slave_first_sync;
extern uint16_t current_random_seed;
extern uint16_t slave_task_counter;
extern bool slave_mouse_active;
extern bool auto_mouse_on;
extern uint16_t auto_mouse_timer;
extern const char *layer_change_reason;

// Function prototypes
void user_sync_info_slave_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data);
void housekeeping_task_sync(void); // Renamed from housekeeping_task_user to avoid conflict
void debug_dump_sync_state(void);
