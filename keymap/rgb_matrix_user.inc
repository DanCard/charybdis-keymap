RGB_MATRIX_EFFECT(fire)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static bool fire(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Speed controls flicker rate
    uint32_t time = g_rgb_timer / (256 - rgb_matrix_config.speed);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        // Get LED position (y: 0=top, 64=bottom thumb cluster)
        uint8_t y = g_led_config.point[i].y;
        uint8_t x = g_led_config.point[i].x;

        // Two phase offsets for independent hue and brightness flicker
        uint8_t phase_val = (time + i * 37) % 256;
        uint8_t phase_hue = (time / 2 + i * 53) % 256;

        // Base intensity from vertical position (bottom = bright, top = dim)
        uint8_t y_intensity = (y * 255) / 64;

        // Edge fade: x ranges 0-224, center is 112
        int16_t x_center_dist = (x > 112) ? (x - 112) : (112 - x);
        uint8_t edge_fade = 255 - (x_center_dist * 80 / 112);

        // Combine position-based intensity
        uint16_t base_val = ((uint16_t)y_intensity * edge_fade) / 255;

        // Brightness flicker (+-40%)
        int16_t flicker = ((phase_val % 80) - 40) * 2;
        int16_t val = base_val + flicker;
        if (val < 0) val = 0;
        if (val > 255) val = 255;

        // Per-key hue variation within warm range (0-40: red to yellow)
        // Base hue from position (red at bottom, orange/yellow toward top)
        uint8_t base_hue = ((64 - y) * 25) / 64; // 0-25 based on height
        // Add per-key hue flicker within warm range
        uint8_t hue_variation = (phase_hue % 20); // 0-19 variation
        uint8_t hue = base_hue + hue_variation;
        if (hue > 45) hue = 45; // cap at yellow, don't go green

        // Skip very dim LEDs (top edges)
        if (val < 15) {
            rgb_matrix_set_color(i, 0, 0, 0);
        } else {
            HSV hsv = { hue, 255, (uint8_t)val };
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif