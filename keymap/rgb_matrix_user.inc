RGB_MATRIX_EFFECT(left_right_toggle)
RGB_MATRIX_EFFECT(police_lights)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

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

static bool police_lights(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Cycle time: 1000ms roughly at default speed
    // Split into two phases: Left (Red) and Right (Blue)
    uint32_t period = (256 - rgb_matrix_config.speed) * 4;
    if (period < 250) period = 250;
    
    uint32_t t = g_rgb_timer % period;
    bool left_phase = (t < (period / 2));
    
    // Strobe logic: Blink twice per phase
    // Phase duration = period / 2.
    // Sub-blink duration = period / 8.
    // On: 0-1, Off: 1-2, On: 2-3, Off: 3-4
    uint32_t phase_t = t % (period / 2);
    uint32_t blink_dur = period / 8;
    
    bool light_on = false;
    // Blink 1
    if (phase_t < blink_dur) light_on = true;
    // Blink 2
    else if (phase_t > (blink_dur * 2) && phase_t < (blink_dur * 3)) light_on = true;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        bool is_left_led = (i < 29);
        
        uint8_t r = 0, g = 0, b = 0;
        
        if (light_on) {
            if (left_phase && is_left_led) {
                r = 255; // Red
            } else if (!left_phase && !is_left_led) {
                b = 255; // Blue
            }
        }
        rgb_matrix_set_color(i, r, g, b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif