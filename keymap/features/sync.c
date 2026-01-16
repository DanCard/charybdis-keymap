#include "sync.h"
#include <lib/lib8tion/lib8tion.h>

// Sync statistics tracking
uint32_t sync_success_count = 0;
uint32_t sync_fail_count = 0;
uint32_t last_sync_time = 0;
uint32_t last_heartbeat_time = 0;

// Sync state variables
bool sync_needed = false;
bool slave_first_sync = true;
uint16_t current_random_seed = 0;
uint16_t slave_task_counter = 0;
bool slave_mouse_active = false;
bool is_caps_lock_on = false;

// External function required for logging
extern const char *get_rgb_mode_name(uint8_t mode);

// Helper for logging time (copied from keymap.c to avoid dependency on it for now)
#define LOG_TIME() do { \
    uint32_t _t = timer_read32(); \
    uprintf("[%lu.%03lu] ", (unsigned long)(_t / 1000), (unsigned long)(_t % 1000)); \
} while(0)

void user_sync_info_slave_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
  const user_sync_info_t *sync_data = (const user_sync_info_t *)in_data;
  user_sync_info_response_t *response = (user_sync_info_response_t *)out_data;

  // Increment slave liveness counter
  slave_task_counter++;

  is_flashlight = sync_data->is_flashlight;
  is_sniping_active = sync_data->is_sniping_active;
  is_fast_mode_active = sync_data->is_fast_mode_active;
  mouse_is_locked = sync_data->mouse_is_locked;
  is_jitter_filter_active = sync_data->is_jitter_filter_active;
  is_caps_lock_on = sync_data->is_caps_lock_on;

  // Send slave's handedness back to master via response (not used currently, just stored locally)

  bool synced = false;
  uint8_t current_mode = rgb_matrix_get_mode();
  bool mode_changed = current_mode != sync_data->rgb_mode || slave_first_sync;
  bool seed_changed = current_random_seed != sync_data->random_seed;

  // Sync random seed for effects like PIXEL_RAIN/PIXEL_FLOW
  if (seed_changed) {
    current_random_seed = sync_data->random_seed;
    random16_set_seed(current_random_seed);
  }

  if (mode_changed) {
    LOG_TIME();
    uprintf("\033[93mSlave: Syncing RGB %d -> %d%s\033[0m\n", current_mode, sync_data->rgb_mode, slave_first_sync ? " (first sync)" : "");
    rgb_matrix_mode_noeeprom(sync_data->rgb_mode);
    slave_first_sync = false;
    // Verify the mode was applied
    uint8_t new_mode = rgb_matrix_get_mode();
    if (new_mode != sync_data->rgb_mode) {
      LOG_TIME();
      uprintf("\033[91mSlave: RGB sync FAILED! Expected %d, got %d\033[0m\n", sync_data->rgb_mode, new_mode);
      sync_fail_count++;
    } else {
      sync_success_count++;
    }
    last_sync_time = timer_read32();
    synced = true;
  }

  if (out_buflen >= sizeof(user_sync_info_response_t)) {
    response->did_rgb_sync = synced;
    response->slave_rgb_mode = rgb_matrix_get_mode();
    response->slave_task_counter = slave_task_counter;
    response->mouse_active = slave_mouse_active;
    slave_mouse_active = false; // Clear flag after reporting
  }
}

void housekeeping_task_sync(void) {
  if (is_keyboard_master()) {
    static uint32_t last_sync = 0;
    // Poll faster (50ms) to detect slave mouse movement for layer switching
    bool needs_periodic = timer_elapsed32(last_sync) > 50;

    // Update caps lock state on master
    is_caps_lock_on = host_keyboard_led_state().caps_lock;

    static user_sync_info_response_t last_slave_response = {0};

    if (sync_needed || needs_periodic) {
      user_sync_info_t sync_data = {
          .is_flashlight = is_flashlight,
          .is_sniping_active = is_sniping_active,
          .is_fast_mode_active = is_fast_mode_active,
          .mouse_is_locked = mouse_is_locked,
          .is_jitter_filter_active = is_jitter_filter_active,
          .is_caps_lock_on = is_caps_lock_on,
          .rgb_mode = rgb_matrix_get_mode(),
          .is_left_hand = is_keyboard_left(),
          .random_seed = current_random_seed};

      user_sync_info_response_t response_data = {0};

      static uint8_t last_synced_mode = 0;
      if (transaction_rpc_exec(USER_SYNC_INFO, sizeof(sync_data), &sync_data,
                               sizeof(response_data), &response_data)) {
        last_sync = timer_read32();
        last_sync_time = last_sync;
        sync_success_count++;
        last_slave_response = response_data; // Cache for heartbeat

        if (response_data.did_rgb_sync && sync_data.rgb_mode != last_synced_mode) {
          LOG_TIME();
          uprintf("\033[92mMaster: Slave synced RGB to mode %d (seed=%u)\033[0m\n", sync_data.rgb_mode, current_random_seed);
          last_synced_mode = sync_data.rgb_mode;
        }
        
        // Handle Slave Mouse Activity (Auto Layer 3)
        if (response_data.mouse_active) {
            if (!auto_mouse_on) {
                layer_change_reason = "Slave Mouse Movement";
                layer_on(3);
                auto_mouse_on = true;
                uprintf("Master: Activated Layer 3 due to Slave Mouse\n");
            }
            // Keep timer alive
            auto_mouse_timer = timer_read();
        }

        sync_needed = false;
      } else {
        sync_fail_count++;
      }
    }

    // Periodic heartbeat - show sync health every HEARTBEAT_INTERVAL
    if (timer_elapsed32(last_heartbeat_time) > HEARTBEAT_INTERVAL) {
      last_heartbeat_time = timer_read32();
      uint32_t since_sync = timer_elapsed32(last_sync_time);
      LOG_TIME();
      uprintf("\033[36m[Heartbeat] Mode=%d (%s) Seed=%u Syncs=%lu/%lu LastSync=%lu.%03lus ago Slave(Mode=%d Ctr=%u)\033[0m\n",
              rgb_matrix_get_mode(), get_rgb_mode_name(rgb_matrix_get_mode()),
              current_random_seed, (unsigned long)sync_success_count, (unsigned long)sync_fail_count,
              (unsigned long)(since_sync / 1000), (unsigned long)(since_sync % 1000),
              last_slave_response.slave_rgb_mode, last_slave_response.slave_task_counter);
    }
  }
}

// Debug function to dump full sync state
void debug_dump_sync_state(void) {
  uint32_t since_sync = last_sync_time > 0 ? timer_elapsed32(last_sync_time) : 0;

  uprintf("\n\033[95m========== SYNC STATE DUMP ==========\033[0m\n");
  LOG_TIME();
  uprintf("Role: %s\n", is_keyboard_master() ? "MASTER" : "SLAVE");
  uprintf("Hand: %s\n", is_keyboard_left() ? "LEFT" : "RIGHT");
  uprintf("\n\033[96m--- RGB State ---\033[0m\n");
  uprintf("Mode: %d (%s)\n", rgb_matrix_get_mode(), get_rgb_mode_name(rgb_matrix_get_mode()));
  uprintf("Random Seed: %u\n", current_random_seed);
  uprintf("HSV: H=%d S=%d V=%d\n", rgb_matrix_get_hue(), rgb_matrix_get_sat(), rgb_matrix_get_val());
  uprintf("\n\033[96m--- Sync Stats ---\033[0m\n");
  uprintf("Success: %lu\n", (unsigned long)sync_success_count);
  uprintf("Failures: %lu\n", (unsigned long)sync_fail_count);
  uprintf("Last Sync: %lu.%03lu sec ago\n", (unsigned long)(since_sync / 1000), (unsigned long)(since_sync % 1000));
  uprintf("\n\033[96m--- Flags ---\033[0m\n");
  uprintf("sync_needed: %s\n", sync_needed ? "YES" : "no");
  uprintf("slave_first_sync: %s\n", slave_first_sync ? "YES" : "no");
  uprintf("is_flashlight: %s\n", is_flashlight ? "YES" : "no");
  uprintf("is_sniping_active: %s\n", is_sniping_active ? "YES" : "no");
  uprintf("is_fast_mode_active: %s\n", is_fast_mode_active ? "YES" : "no");
  uprintf("mouse_is_locked: %s\n", mouse_is_locked ? "YES" : "no");
  uprintf("is_jitter_filter_active: %s\n", is_jitter_filter_active ? "YES" : "no");
  uprintf("is_caps_lock_on: %s\n", is_caps_lock_on ? "YES" : "no");
  uprintf("\033[95m======================================\033[0m\n\n");
}
