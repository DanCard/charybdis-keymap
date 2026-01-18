#include "statistics.h"
#include <stdio.h>
#include "keycodes.h"
#include "tap_hold.h" // For LONG_PRESS_TIMEOUT (250ms)

// Track by physical matrix position
// Charybdis 4x6 has 10 rows (5 per side) and 6 columns
static uint32_t matrix_counts_short[MATRIX_ROWS][MATRIX_COLS] = {0};
static uint32_t matrix_counts_long[MATRIX_ROWS][MATRIX_COLS] = {0};
static uint16_t press_timers[MATRIX_ROWS][MATRIX_COLS] = {0};
static uint32_t last_print_time = 0;

// Layer Timing Stats
#define MAX_LAYERS 16
static uint64_t layer_times[MAX_LAYERS] = {0};
static uint32_t last_layer_switch_time = 0;
static uint8_t current_layer = 0;
static bool first_layer_update = true;

// Print every 10 minutes (600,000 ms)
#define PRINT_INTERVAL 600000 

void process_statistics(keyrecord_t *record) {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return;

    if (record->event.pressed) {
        press_timers[row][col] = timer_read();
    } else {
        // Released
        uint16_t duration = timer_elapsed(press_timers[row][col]);
        if (duration < LONG_PRESS_TIMEOUT) {
            matrix_counts_short[row][col]++;
        } else {
            matrix_counts_long[row][col]++;
        }
    }
}

void update_layer_stats(uint8_t layer) {
    uint32_t now = timer_read32();
    
    if (first_layer_update) {
        last_layer_switch_time = now;
        current_layer = layer;
        first_layer_update = false;
        return;
    }
    
    uint32_t elapsed = timer_elapsed32(last_layer_switch_time);
    
    // Add elapsed time (safe up to ~49 days per session)
    layer_times[current_layer] += elapsed;
    
    last_layer_switch_time = now;
    current_layer = layer;
}

// We need to access the keymap to show what key is at that position on Layer 0
extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];

static const char* get_key_name(uint16_t kc) {
    // Basic Alphanumeric
    if (kc >= KC_A && kc <= KC_Z) {
        static const char *alpha[] = {
            "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
            "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
        };
        return alpha[kc - KC_A];
    }
    if (kc >= KC_1 && kc <= KC_9) {
        static const char *digits[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
        return digits[kc - KC_1];
    }
    if (kc == KC_0) return "0";
    
    // Common symbols and modifiers
    switch (kc) {
        case KC_TAB:  return "TAB";
        case KC_ENT:  return "ENT";
        case KC_SPC:  return "SPC";
        case KC_BSPC: return "BSPC";
        case KC_ESC:  return "ESC";
        case KC_DEL:  return "DEL";
        case KC_LSFT: return "LSFT";
        case KC_RSFT: return "RSFT";
        case KC_LCTL: return "LCTL";
        case KC_RCTL: return "RCTL";
        case KC_LALT: return "LALT";
        case KC_RALT: return "RALT";
        case KC_LGUI: return "LGUI";
        case KC_RGUI: return "RGUI";
        case KC_LEFT: return "LEFT";
        case KC_RGHT: return "RGHT";
        case KC_UP:   return "UP";
        case KC_DOWN: return "DOWN";
        case KC_COMM: return ",";
        case KC_DOT:  return ".";
        case KC_SLSH: return "/";
        case KC_SCLN: return ";";
        case KC_QUOT: return "'";
        case KC_MINS: return "-";
        case KC_BSLS: return "\\";
        
        // Custom Keycodes from features/keycodes.h
        case KC_Q_L4:      return "Q(L4)";
        case KC_1_L1:      return "1(L1)";
        case KC_2_L2:      return "2(L2)";
        case KC_3_L3:      return "3(L3)";
        case KC_4_L4:      return "4(L4)";
        case KC_5_L5:      return "5(L5)";
        case KC_ENT_L2:    return "ENT(L2)";
        case KC_ENT_L4:    return "ENT(L4)";
        case KC_SPC_L2:    return "SPC(L2)";
        case KC_SPC_L4:    return "SPC(L4)";
        case KC_L1_L3:     return "L1_L3";
        case KC_R_L2:      return "R_TG2";
        case KC_PLUS_COLON: return "+/:";
        case QK_GESC:       return "GESC";
    }
    
    // Tap Dance handling (if possible to match hex)
    if (kc == 0x5700) return "Z(TD)"; // TD(0) 

    return "???";
}

typedef struct {
    uint8_t row;
    uint8_t col;
    uint16_t kc;
    const char* name;
    uint32_t tap_count;
    uint32_t hold_count;
} key_stat_t;

void print_statistics_now(void) {
    // Update current layer time first so it's accurate
    update_layer_stats(current_layer);
    
    uprintf("\n--- Layer Usage Statistics ---\n");
    uprintf("Layer | Time (m:s) | %% Total\n");
    uprintf("---------------------------\n");
    
    uint64_t total_time = 0;
    for (int i = 0; i < MAX_LAYERS; i++) {
        total_time += layer_times[i];
    }
    
    if (total_time > 0) {
        for (int i = 0; i < MAX_LAYERS; i++) {
            if (layer_times[i] > 0) {
                // Calculate minutes and remaining seconds
                uint32_t total_seconds = (uint32_t)(layer_times[i] / 1000);
                uint32_t minutes = total_seconds / 60;
                uint32_t seconds = total_seconds % 60;
                
                // Calculate percentage using 64-bit math
                uint32_t percent = (uint32_t)((layer_times[i] * 100) / total_time);
                uprintf("%5d | %4lu:%02lu | %6lu%%\n", i, (unsigned long)minutes, (unsigned long)seconds, (unsigned long)percent);
            }
        }
    } else {
        uprintf("No layer time recorded yet.\n");
    }
    uprintf("---------------------------\n");

    uprintf("\n--- Physical Key Usage List ---\n");
    uprintf("Row Col | Short (<%dms) | Long (>=%dms) | Key (L0) | Code\n", LONG_PRESS_TIMEOUT, LONG_PRESS_TIMEOUT);
    uprintf("------------------------------------------------------------\n");
    
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            uint16_t kc = pgm_read_word(&keymaps[0][r][c]);
            if (kc != KC_NO) {
                uprintf("%2d  %2d  | %13lu | %12lu | %-8s | 0x%04X\n", 
                    r, c, 
                    matrix_counts_short[r][c], 
                    matrix_counts_long[r][c], 
                    get_key_name(kc), 
                    kc);
            }
        }
    }
    uprintf("------------------------------------------------------------\n");

    // Print sorted by tap usage (least used first)
    uprintf("\n--- Physical Key Usage (Sorted by Tap Count, Least First) ---\n");
    uprintf("Tap Count | Hold Count | Key (L0) | Row Col | Code\n");
    uprintf("------------------------------------------------------------\n");
    
    key_stat_t stats[MATRIX_ROWS * MATRIX_COLS];
    uint8_t count = 0;
    
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            uint16_t kc = pgm_read_word(&keymaps[0][r][c]);
            if (kc != KC_NO) {
                stats[count].row = r;
                stats[count].col = c;
                stats[count].kc = kc;
                stats[count].name = get_key_name(kc);
                stats[count].tap_count = matrix_counts_short[r][c];
                stats[count].hold_count = matrix_counts_long[r][c];
                count++;
            }
        }
    }
    
    // Bubble sort by tap count (ascending)
    for (uint8_t i = 0; i < count - 1; i++) {
        for (uint8_t j = 0; j < count - i - 1; j++) {
            if (stats[j].tap_count > stats[j + 1].tap_count) {
                key_stat_t temp = stats[j];
                stats[j] = stats[j + 1];
                stats[j + 1] = temp;
            }
        }
    }
    
    for (uint8_t i = 0; i < count; i++) {
        uprintf("%9lu | %10lu | %-8s | %2d   %2d | 0x%04X\n", 
            stats[i].tap_count, 
            stats[i].hold_count, 
            stats[i].name, 
            stats[i].row, 
            stats[i].col,
            stats[i].kc);
    }
    uprintf("------------------------------------------------------------\n");
}

static void print_grid_matrix(uint32_t counts[MATRIX_ROWS][MATRIX_COLS], const char* title) {
    uprintf("--- %s ---\n", title);
    
    // Main 4x6 Grid
    for (uint8_t r = 0; r < 4; r++) {
        // Left Half (Rows 0-3, Cols 0-5)
        for (uint8_t c = 0; c < 6; c++) {
            uprintf("%5lu ", counts[r][c]);
        }   
        uprintf(" | ");   
        // Right Half (Rows 5-8, Cols 5-0)
        for (int8_t c = 5; c >= 0; c--) {
            uprintf("%5lu ", counts[r + 5][c]);
        }
        uprintf("\n");
    }
    // Thumb Clusters
    uprintf("\n");
    // Left Thumb Top: [4][3], [4][4], [4][1]
    uprintf("                  %5lu %5lu %5lu ",  counts[4][3], counts[4][4], counts[4][1]);        
    uprintf(" | ");
    // Right Thumb Top: [9][1], [9][3]
    uprintf("%5lu %5lu\n", counts[9][1], counts[9][3]);          
    // Left Thumb Bottom: [4][5], [4][2]
    uprintf("                        %5lu %5lu ", counts[4][5], counts[4][2]);           
    uprintf(" | ");   
    // Right Thumb Bottom: [9][5] (Approximation)
    uprintf("      %5lu\n", counts[9][5]);
}

static void print_layer_times(void) {
    uprintf("\n--- Layer Usage ---\n");

    // Update current layer time first so it's accurate
    update_layer_stats(current_layer);

    uint64_t total_time = 0;
    for (int i = 0; i < MAX_LAYERS; i++) {
        total_time += layer_times[i];
    }

    if (total_time > 0) {
        for (int i = 0; i < MAX_LAYERS; i++) {
            if (layer_times[i] > 0) {
                uint32_t total_seconds = (uint32_t)(layer_times[i] / 1000);
                uint32_t minutes = total_seconds / 60;
                uint32_t seconds = total_seconds % 60;
                uint32_t percent = (uint32_t)((layer_times[i] * 100) / total_time);
                uprintf("L%d: %lu:%02lu (%lu%%)  ", i, (unsigned long)minutes, (unsigned long)seconds, (unsigned long)percent);
            }
        }
        uprintf("\n");
    } else {
        uprintf("No layer time recorded yet.\n");
    }
}

void print_statistics_grid(void) {
    char title_buf[64];

    print_layer_times();

    snprintf(title_buf, sizeof(title_buf), "Short Press Grid (<%dms)", LONG_PRESS_TIMEOUT);
    print_grid_matrix(matrix_counts_short, title_buf);

    snprintf(title_buf, sizeof(title_buf), "Long Press Grid (>=%dms)", LONG_PRESS_TIMEOUT);
    print_grid_matrix(matrix_counts_long, title_buf);
}

void matrix_scan_statistics(void) {
    if (timer_elapsed32(last_print_time) > PRINT_INTERVAL) {
        last_print_time = timer_read32();
        static bool print_grid_next = true;
        
        if (print_grid_next) {
            print_statistics_grid();
        } else {
            print_statistics_now();
        }
        print_grid_next = !print_grid_next;
    }
}
