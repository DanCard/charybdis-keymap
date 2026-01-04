RGB_MATRIX_EFFECT(left_right_toggle)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static bool left_right_toggle(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Toggle interval. Standard speed is 0-255. 
    // We want a period of maybe 1-2 seconds.
    // map speed 128 -> 2000ms period (1s per side)
    uint32_t period = (256 - rgb_matrix_config.speed) * 16;
    if (period < 500) period = 500;

    bool left_active = (g_rgb_timer % period) < (period / 2);

    HSV hsv = { rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v };
    RGB rgb = hsv_to_rgb(hsv);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        // Charybdis 4x6 split point (Left: 0-28, Right: 29+)
        bool is_left_led = (i < 29);
        
        if ((left_active && is_left_led) || (!left_active && !is_left_led)) {
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif
