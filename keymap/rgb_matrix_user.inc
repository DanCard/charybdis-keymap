RGB_MATRIX_EFFECT(fire)
RGB_MATRIX_EFFECT(campfire)

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

        // Horizontal sway: flames drift left/right over time
        // Sway is stronger at top (tips move more), weaker at bottom (base is stable)
        uint8_t sway_amount = ((64 - y) * 25) / 64; // 0 at bottom, 25 at top
        int16_t sway_offset = ((int16_t)((time / 3) % 256) - 128) * sway_amount / 128;

        // Edge fade: x ranges 0-224, center is 112 (shifted by sway)
        int16_t shifted_center = 112 + sway_offset;
        int16_t x_center_dist = (x > shifted_center) ? (x - shifted_center) : (shifted_center - x);
        if (x_center_dist < 0) x_center_dist = 0;
        uint8_t edge_fade = 255 - (x_center_dist * 80 / 112);
        if (edge_fade > 255) edge_fade = 255; // clamp overflow from uint8

        // Combine position-based intensity (boosted ~25% for brighter flames)
        uint16_t base_val = ((uint16_t)y_intensity * edge_fade) / 200;

        // Brightness flicker (+-40%)
        int16_t flicker = ((phase_val % 80) - 40) * 2;
        int16_t val = base_val + flicker;
        if (val < 0) val = 0;
        if (val > 255) val = 255;

        // Per-key hue variation within warm range (0-45: red to yellow)
        // Base hue from position (deep red at bottom, yellow toward top)
        uint8_t base_hue = ((64 - y) * 35) / 64; // 0-35 based on height
        // Add per-key hue flicker within warm range
        uint8_t hue_variation = (phase_hue % 15); // 0-14 variation
        uint8_t hue = base_hue + hue_variation;
        if (hue > 45) hue = 45; // cap at yellow, don't go green

        // Saturation: high at bottom (deep red), low at top (white-hot tips)
        uint8_t sat = 255 - ((64 - y) * 150 / 64); // ~255 at bottom, ~105 at top

        // Skip very dim LEDs (top edges)
        if (val < 15) {
            rgb_matrix_set_color(i, 0, 0, 0);
        } else {
            HSV hsv = { hue, sat, (uint8_t)val };
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool campfire(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint32_t time = g_rgb_timer / (256 - rgb_matrix_config.speed);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        uint8_t y = g_led_config.point[i].y;
        uint8_t x = g_led_config.point[i].x;

        // Turbulence/Flicker phases
        // Use prime numbers for phase offsets to avoid patterns aligning too often
        uint8_t flicker_phase = (time * 3 + i * 17) % 256; 
        uint8_t spark_phase = (time + i * 137) % 256;

        uint8_t hue = 0;
        uint8_t sat = 255;
        uint16_t val = 0;

        // --- ZONING ---
        if (y >= 55) {
             // BASE: Blue/Purple Core + Deep Red
             if (y >= 60) {
                 hue = 195 + (sin8(time) / 6); // Shifting Blue/Purple/Pink
                 sat = 255;
             } else {
                 hue = 0; // Red
                 sat = 255;
             }
             val = 200 + (sin8(flicker_phase) / 6); 
        } else if (y >= 30) {
             // MID: Red to Yellow transition
             // y=55 -> 0 (Red), y=30 -> 35 (Yellow)
             hue = ((55 - y) * 35) / 25;
             sat = 255;
             val = 255; // Brightest part
        } else {
             // TOP: Embers/Smoke
             hue = 35; // Yellow/Orange
             sat = 255 - ((30 - y) * 5); // Desaturate slightly as it goes up
             val = 50; // Dim default
             
             // Random Sparks
             if (spark_phase > 250) {
                 val = 255;
                 sat = 0; // White hot spark
             }
        }

        // --- SHAPE & FLICKER ---
        // Horizontal swaying center
        uint8_t sway = sin8(time / 2) / 8; // +/- 32/8 = +/-4 sway
        int16_t center_x = 112 + (sway - 16); // Center around 112

        int16_t dist_x = (x > center_x) ? (x - center_x) : (center_x - x);
        
        // Width tapers with height
        uint8_t visible_width = 20 + (y * 40) / 64;
        
        if (dist_x > visible_width) {
             // Fade out based on distance
             uint16_t fade = (dist_x - visible_width) * 10;
             if (val > fade) val -= fade;
             else val = 0;
        }

        // Apply flickering modulation to value
        int8_t mod = (sin8(flicker_phase) / 4) - 32; // +/- 32 range
        if (val > 0) {
            int16_t new_val = val + mod;
             if (new_val < 0) new_val = 0;
             if (new_val > 255) new_val = 255;
             val = new_val;
        }

        // Apply final color
        if (val < 10) {
             rgb_matrix_set_color(i, 0, 0, 0);
        } else {
             HSV hsv = { hue, sat, (uint8_t)val };
             RGB rgb = hsv_to_rgb(hsv);
             rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif