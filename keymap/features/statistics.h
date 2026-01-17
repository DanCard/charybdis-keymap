#pragma once

#include QMK_KEYBOARD_H

void process_statistics(keyrecord_t *record);
void matrix_scan_statistics(void);
void update_layer_stats(uint8_t layer);
void print_statistics_now(void);
void print_statistics_grid(void);
