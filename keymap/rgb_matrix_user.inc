RGB_MATRIX_EFFECT(fire)
RGB_MATRIX_EFFECT(wildfire)
RGB_MATRIX_EFFECT(campfire)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// =============================================================================
// Fire Effect Constants
// =============================================================================

// LED matrix geometry (from Charybdis 4x6 layout)
#define MATRIX_HEIGHT           64      // Y range: 0 (top) to 64 (bottom/thumb)
#define MATRIX_CENTER_X         112     // Horizontal center point
#define MAX_COLOR_VAL           255     // Maximum RGB/HSV value

// Phase multipliers for flicker randomization (using primes for variation)
#define PHASE_VAL_MULT          37      // Brightness phase offset per LED
#define PHASE_HUE_MULT          53      // Hue phase offset per LED

// Fire effect: flame shape and intensity
#define FIRE_SWAY_MAX           25      // Maximum horizontal sway at flame tips
#define FIRE_EDGE_FADE_FACTOR   80      // How quickly flames fade at edges
#define FIRE_INTENSITY_DIV      200     // Base intensity divisor (lower = brighter)
#define FIRE_MIN_BRIGHTNESS     15      // Below this, LED is turned off
#define FIRE_HUE_RANGE          35      // Base hue range (red to orange)
#define FIRE_HUE_MAX            45      // Cap at yellow (don't go green)
#define FIRE_SAT_REDUCTION      150     // Saturation reduction at top (white-hot)
#define FIRE_FLICKER_RANGE      80      // Brightness flicker modulo range

// Wildfire effect: chaos generator multipliers (primes for pseudo-randomness)
#define CHAOS_MULT_1            47
#define CHAOS_MULT_2            113
#define CHAOS_MULT_3            61
#define CHAOS_MULT_4            173

// Wildfire effect: zone thresholds
#define WILDFIRE_ZONE1_BASE     48      // Bottom zone (deep reds)
#define WILDFIRE_ZONE2_BASE     25      // Middle zone (reds/oranges)
#define WILDFIRE_TURBULENCE     16      // Y-axis turbulence range

// Wildfire effect: special triggers
#define WILDFIRE_HEATWAVE_THRESH 200    // Heat wave activation threshold
#define WILDFIRE_CORE_THRESH    248     // Hot core flare threshold
#define WILDFIRE_EMBER_THRESH   252     // Ember pop threshold
#define WILDFIRE_HUE_BOOST      8       // Hue shift during heat waves

// Campfire effect: heat zones
#define CAMPFIRE_HEAT_BASE      130     // Minimum heat at top
#define CAMPFIRE_SWAY_Y_THRESH  70      // Y threshold for sway calculation
#define CAMPFIRE_SPARK_THRESH   100     // Heat threshold for spark chance
#define CAMPFIRE_CORE_HEAT      210     // Pure red zone threshold
#define CAMPFIRE_BODY_HEAT      140     // Red-to-orange zone threshold
#define CAMPFIRE_TIPS_HEAT      60      // Orange-to-purple zone threshold
#define CAMPFIRE_MIN_EMBER_VAL  40      // Minimum brightness for embers

// Hue values (0-255 HSV wheel)
#define HUE_RED                 0
#define HUE_ORANGE              25
#define HUE_YELLOW              38
#define HUE_BLUE                155
#define HUE_PURPLE              190
#define HUE_PINK                235

// =============================================================================

static bool fire(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Speed controls flicker rate
    uint32_t time = g_rgb_timer / (256 - rgb_matrix_config.speed);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        // Get LED position (y: 0=top, MATRIX_HEIGHT=bottom thumb cluster)
        uint8_t y = g_led_config.point[i].y;
        uint8_t x = g_led_config.point[i].x;

        // Two phase offsets for independent hue and brightness flicker
        uint8_t phase_val = (time + i * PHASE_VAL_MULT) % 256;
        uint8_t phase_hue = (time / 2 + i * PHASE_HUE_MULT) % 256;

        // Base intensity from vertical position (bottom = bright, top = dim)
        uint8_t y_intensity = (y * MAX_COLOR_VAL) / MATRIX_HEIGHT;

        // Horizontal sway: flames drift left/right over time
        // Sway is stronger at top (tips move more), weaker at bottom (base is stable)
        uint8_t sway_amount = ((MATRIX_HEIGHT - y) * FIRE_SWAY_MAX) / MATRIX_HEIGHT;
        int16_t sway_offset = ((int16_t)((time / 3) % 256) - 128) * sway_amount / 128;

        // Edge fade: center is MATRIX_CENTER_X (shifted by sway)
        int16_t shifted_center = MATRIX_CENTER_X + sway_offset;
        int16_t x_center_dist = (x > shifted_center) ? (x - shifted_center) : (shifted_center - x);
        if (x_center_dist < 0) x_center_dist = 0;
        uint8_t edge_fade = MAX_COLOR_VAL - (x_center_dist * FIRE_EDGE_FADE_FACTOR / MATRIX_CENTER_X);
        if (edge_fade > MAX_COLOR_VAL) edge_fade = MAX_COLOR_VAL; // clamp overflow from uint8

        // Combine position-based intensity (boosted ~25% for brighter flames)
        uint16_t base_val = ((uint16_t)y_intensity * edge_fade) / FIRE_INTENSITY_DIV;

        // Brightness flicker (+-40%)
        int16_t flicker = ((phase_val % FIRE_FLICKER_RANGE) - (FIRE_FLICKER_RANGE / 2)) * 2;
        int16_t val = base_val + flicker;
        if (val < 0) val = 0;
        if (val > MAX_COLOR_VAL) val = MAX_COLOR_VAL;

        // Per-key hue variation within warm range (0-FIRE_HUE_MAX: red to yellow)
        // Base hue from position (deep red at bottom, yellow toward top)
        uint8_t base_hue = ((MATRIX_HEIGHT - y) * FIRE_HUE_RANGE) / MATRIX_HEIGHT;
        // Add per-key hue flicker within warm range
        uint8_t hue_variation = (phase_hue % 15);
        uint8_t hue = base_hue + hue_variation;
        if (hue > FIRE_HUE_MAX) hue = FIRE_HUE_MAX; // cap at yellow, don't go green

        // Saturation: high at bottom (deep red), low at top (white-hot tips)
        uint8_t sat = MAX_COLOR_VAL - ((MATRIX_HEIGHT - y) * FIRE_SAT_REDUCTION / MATRIX_HEIGHT);

        // Skip very dim LEDs (top edges)
        if (val < FIRE_MIN_BRIGHTNESS) {
            rgb_matrix_set_color(i, 0, 0, 0);
        } else {
            HSV hsv = { hue, sat, (uint8_t)((val * rgb_matrix_config.hsv.v) / MAX_COLOR_VAL) };
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool wildfire(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Slower, more dramatic animation
    uint32_t time = (g_rgb_timer / 2) / (256 - rgb_matrix_config.speed);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        uint8_t y = g_led_config.point[i].y;
        uint8_t x = g_led_config.point[i].x;

        // --- CHAOS GENERATORS (multiple overlapping waves) ---
        // Each LED gets unique, time-varying offsets using prime multipliers
        uint8_t chaos1 = sin8(time * 3 + i * CHAOS_MULT_1);
        uint8_t chaos2 = sin8(time * 5 + i * CHAOS_MULT_2 + x * 7);
        uint8_t chaos3 = sin8(time * 2 + i * CHAOS_MULT_3 + y * 11);
        uint8_t chaos4 = sin8(time * 7 + i * CHAOS_MULT_4);

        // Per-key turbulence that shifts zone boundaries
        int8_t y_turbulence = ((chaos1 + chaos2) / 16) - WILDFIRE_TURBULENCE;
        int16_t effective_y = y + y_turbulence;
        if (effective_y < 0) effective_y = 0;
        if (effective_y > MATRIX_HEIGHT) effective_y = MATRIX_HEIGHT;

        // --- DYNAMIC ZONING with turbulent boundaries ---
        uint8_t hue, sat;
        int16_t val;

        // Zone thresholds shift per-key based on chaos
        int16_t zone1_thresh = WILDFIRE_ZONE1_BASE + (chaos3 / 16) - 8;
        int16_t zone2_thresh = WILDFIRE_ZONE2_BASE + (chaos4 / 20) - 6;

        if (effective_y >= zone1_thresh) {
            // BOTTOM: Deep reds and occasional brown
            uint8_t bottom_choice = chaos1 % 100;
            if (bottom_choice < 85) {
                hue = (chaos2 / 50);  // 0-5, deep red
                sat = 255;
                val = 200 + (chaos2 / 8);
            } else {
                hue = 15 + (chaos2 / 40);  // Brown (dark orange)
                sat = 255;
                val = 120 + (chaos3 / 8);  // Darker for brown
            }
        } else if (effective_y >= zone2_thresh) {
            // MIDDLE: Mostly reds and oranges, some accent colors
            uint8_t mid_choice = (chaos1 + chaos4) % 100;
            if (mid_choice < 50) {
                hue = (chaos2 / 30);  // Red (0-8)
                sat = 255;
                val = 220 + (chaos3 / 12);
            } else if (mid_choice < 75) {
                hue = 10 + (chaos2 / 20);  // Orange (10-22)
                sat = 255;
                val = 230 + (chaos2 / 12);
            } else if (mid_choice < 85) {
                hue = 220 + (chaos2 / 15);  // Pink (220-237)
                sat = 255;
                val = 200 + (chaos3 / 10);
            } else if (mid_choice < 95) {
                hue = 180 + (chaos2 / 20);  // Purple-blue (180-192)
                sat = 255;
                val = 180 + (chaos3 / 10);
            } else {
                hue = 15 + (chaos2 / 30);  // Brown
                sat = 255;
                val = 100 + (chaos3 / 10);
            }
        } else {
            // TOP: Varied colors - less yellow, more variety
            uint8_t color_choice = (chaos1 + chaos3) % 100;
            if (color_choice < 15) {
                hue = 38 + (chaos2 / 50);  // Yellow (38-43)
                sat = 255;
            } else if (color_choice < 35) {
                hue = 0 + (chaos2 / 40);  // Red (0-6)
                sat = 255;
            } else if (color_choice < 55) {
                hue = 235 + (chaos2 / 15);  // Pink (235-252)
                sat = 255;
            } else if (color_choice < 75) {
                hue = 195 + (chaos2 / 12);  // Purple (195-216)
                sat = 255;
            } else if (color_choice < 90) {
                hue = 155 + (chaos2 / 15);  // Blue (155-172)
                sat = 255;
            } else {
                hue = 12 + (chaos2 / 30);  // Brown/dark orange
                sat = 255;
            }
            val = 140 + (chaos2 / 4) - ((zone2_thresh - effective_y) * 3);
        }

        // --- FLAME TENDRILS: Horizontal waves that break uniformity ---
        uint8_t tendril = sin8(x * 4 + time * 2 + y * 3);
        int16_t tendril_mod = (tendril / 8) - 16;  // +-16
        val += tendril_mod;

        // --- ROLLING HEAT WAVES: Diagonal patterns ---
        uint8_t heat_wave = sin8(x * 2 + y * 3 + time * 4);
        if (heat_wave > WILDFIRE_HEATWAVE_THRESH) {
            val += 40;
            if (hue < FIRE_HUE_MAX) hue += WILDFIRE_HUE_BOOST;  // Push toward yellow during heat wave
        }

        // --- HOT CORES: Random bright flares ---
        uint8_t core_chance = (chaos1 + chaos4) / 2;
        if (core_chance > WILDFIRE_CORE_THRESH) {
            uint8_t flare_type = chaos2 % 100;
            if (flare_type < 40) {
                hue = 0 + (chaos3 / 50);  // Deep red
            } else if (flare_type < 60) {
                hue = 235 + (chaos3 / 20);  // Pink
            } else if (flare_type < 80) {
                hue = 200 + (chaos3 / 20);  // Purple
            } else {
                hue = 160 + (chaos3 / 25);  // Blue
            }
            sat = 255;
            val = 255;
        }

        // --- EMBER POPS: Varied color sparks ---
        if (chaos4 > WILDFIRE_EMBER_THRESH) {
            uint8_t spark_type = chaos1 % 100;
            if (spark_type < 30) {
                hue = 0 + (chaos2 / 40);  // Red
            } else if (spark_type < 50) {
                hue = 38 + (chaos2 / 60);  // Yellow
            } else if (spark_type < 70) {
                hue = 240 + (chaos2 / 20);  // Pink
            } else if (spark_type < 85) {
                hue = 195 + (chaos2 / 20);  // Purple
            } else {
                hue = 165 + (chaos2 / 25);  // Blue
            }
            sat = 255;
            val = 255;
        }

        // --- INTENSITY FLICKER (triple-layered) ---
        int16_t flicker = ((chaos1 / 3) - 42) + ((chaos2 / 4) - 32) + ((chaos3 / 5) - 25);
        val += flicker;

        // Clamp final value
        if (val > 255) val = 255;
        if (val < 0) val = 0;

        // Apply color
        if (val < 10) {
            rgb_matrix_set_color(i, 0, 0, 0);
        } else {
            HSV hsv = { hue, sat, (uint8_t)((val * rgb_matrix_config.hsv.v) / 255) };
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

        // --- INFERNO HEAT MAP ---

        // 1. Vertical Heat:
        // Base heat higher overall to avoid black corners.
        // y=MATRIX_HEIGHT (bottom) -> 258. y=0 (top) -> CAMPFIRE_HEAT_BASE.
        int16_t heat = CAMPFIRE_HEAT_BASE + (y * 2);

        // 2. Chaotic Turbulence (Explosive)
        // High frequency noise for roaring fire
        uint8_t chaos1 = sin8(time * 4 + i * 23);
        uint8_t chaos2 = sin8(time * 6 + x * 9 + y * 13);
        uint8_t chaos3 = sin8(time * 2 + y * 41); // Slow rise

        // Turbulence: -40 to +40 range
        int16_t turbulence = ((chaos1 + chaos2) / 6) - 42;
        heat += turbulence;

        // 3. Horizontal Sway (Wide Column)
        uint8_t sway_mag = (CAMPFIRE_SWAY_Y_THRESH - y) / 4;
        int8_t sway = (sin8(time / 2) - 128) * sway_mag / 128;
        int16_t center_x = MATRIX_CENTER_X + sway;
        int16_t dist_x = (x > center_x) ? (x - center_x) : (center_x - x);

        // Reduced cooling factor (0.8 instead of 1.5) to widen flame
        heat -= (dist_x * 4 / 5);

        // 4. Embers / Sparks (Chaos Injection)
        // Occasional intense pops in low-heat areas
        if (heat < CAMPFIRE_SPARK_THRESH) {
            // If chaos3 (slow rise) lines up with chaos1 (fast), trigger spark
            if (chaos1 > 240 && chaos3 > WILDFIRE_HEATWAVE_THRESH) {
                heat = CAMPFIRE_CORE_HEAT + 10; // Instant hot spark
            }
        }

        // Clamp Heat
        if (heat < 0) heat = 0;
        if (heat > MAX_COLOR_VAL) heat = MAX_COLOR_VAL;

        // 5. Heat -> Color Mapping (Deep Red Focus)
        uint8_t hue;
        uint8_t sat = MAX_COLOR_VAL;
        uint8_t val = heat;

        if (heat > CAMPFIRE_CORE_HEAT) {
            // CORE: Pure Deep Red
            hue = HUE_RED;
            sat = MAX_COLOR_VAL;
        } else if (heat > CAMPFIRE_BODY_HEAT) {
            // BODY: Red -> Orange
            hue = ((CAMPFIRE_CORE_HEAT - heat) * HUE_ORANGE) / (CAMPFIRE_CORE_HEAT - CAMPFIRE_BODY_HEAT);
            // Invert logic: CAMPFIRE_CORE_HEAT is Red(0), CAMPFIRE_BODY_HEAT is Orange(25)
            hue = HUE_ORANGE - hue;
        } else if (heat > CAMPFIRE_TIPS_HEAT) {
            // TIPS: Orange -> Purple
            hue = HUE_ORANGE + ((CAMPFIRE_BODY_HEAT - heat) * (HUE_PURPLE - HUE_ORANGE) / (CAMPFIRE_BODY_HEAT - CAMPFIRE_TIPS_HEAT));
        } else {
            // ASH/FADE: Purple -> Blue/Black
            hue = HUE_PURPLE + ((CAMPFIRE_TIPS_HEAT - heat));
            if (val < CAMPFIRE_MIN_EMBER_VAL) val = CAMPFIRE_MIN_EMBER_VAL; // Maintain some visibility for dim embers
        }

        if (val < FIRE_MIN_BRIGHTNESS) {
             rgb_matrix_set_color(i, 0, 0, 0);
        } else {
             HSV hsv = { hue, sat, (uint8_t)((val * rgb_matrix_config.hsv.v) / MAX_COLOR_VAL) };
             RGB rgb = hsv_to_rgb(hsv);
             rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif