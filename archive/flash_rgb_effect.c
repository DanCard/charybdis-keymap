// Archived RGB Matrix Effect: Flash
// Previously renamed from "police" effect
// Creates alternating left/right flashing pattern with cycling colors

// To restore: Add this line to rgb_matrix_user.inc declarations section:
// RGB_MATRIX_EFFECT(flash)

// And add this implementation to the #ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS section:

static bool flash(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Cycle time for color changes
    uint32_t period = (256 - rgb_matrix_config.speed) * 4;
    if (period < 250) period = 250;

    uint32_t t = g_rgb_timer % period;
    bool left_phase = (t < (period / 2));

    // Calculate pulse ID to increment hue on each phase change
    uint32_t pulse_id = g_rgb_timer / (period / 2);

    // Increment hue on color wheel (step of 42 for distinct colors)
    uint8_t dynamic_hue = rgb_matrix_config.hsv.h + (uint8_t)(pulse_id * 42);

    // Strobe logic: Blink twice per phase
    uint32_t phase_t = t % (period / 2);
    uint32_t blink_dur = period / 8;

    bool light_on = false;
    // Blink 1
    if (phase_t < blink_dur) light_on = true;
    // Blink 2
    else if (phase_t > (blink_dur * 2) && phase_t < (blink_dur * 3)) light_on = true;

    HSV hsv = { dynamic_hue, 255, light_on ? 32 : 0 };
    RGB rgb = hsv_to_rgb(hsv);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        bool is_left_led = (i < 29);

        if ((left_phase && is_left_led) || (!left_phase && !is_left_led)) {
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}
