// Archived RGB Matrix Effect: Left Right Toggle
// Alternates lighting between left and right halves with cycling colors

// To restore: Add this line to rgb_matrix_user.inc declarations section:
// RGB_MATRIX_EFFECT(left_right_toggle)

// And add this implementation to the #ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS section:

static bool left_right_toggle(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Toggle interval based on speed
    uint32_t period = (256 - rgb_matrix_config.speed) * 8;
    if (period < 250) period = 250;

    // Determine which side is active
    bool left_active = (g_rgb_timer % period) < (period / 2);

    // Calculate a pulse ID to change color every swap
    // Dividing by (period/2) gives us a unique index for every "half-period"
    uint32_t pulse_id = g_rgb_timer / (period / 2);

    // Calculate hue: Base hue + (pulse_id * step)
    // Using a step of 32 ensures distinct colors each jump
    uint8_t dynamic_hue = rgb_matrix_config.hsv.h + (uint8_t)(pulse_id * 32);

    HSV hsv = { dynamic_hue, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v };
    RGB rgb = hsv_to_rgb(hsv);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        bool is_left_led = (i < 29);

        if ((left_active && is_left_led) || (!left_active && !is_left_led)) {
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}
