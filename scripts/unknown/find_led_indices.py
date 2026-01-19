import json
import re

def load_info_json(path):
    with open(path, 'r') as f:
        return json.load(f)

def parse_keymap(path):
    with open(path, 'r') as f:
        content = f.read()
    
    layers = {}
    
    # Find start of keymaps array
    start_match = re.search(r'const uint16_t PROGMEM keymaps\[\]\[MATRIX_ROWS\]\[MATRIX_COLS\] = \{', content)
    if not start_match: return {}
    
    current_pos = start_match.end()
    
    while True:
        # Find next layer definition [N] = LAYOUT(
        layer_start = re.search(r'\[(\d+)\]\s*=\s*LAYOUT\(', content[current_pos:])
        if not layer_start:
            break
            
        layer_num = int(layer_start.group(1))
        # Absolute start position of the content inside LAYOUT(...)
        content_start = current_pos + layer_start.end()
        
        # Extract balanced parenthesis content
        balance = 1
        i = content_start
        while i < len(content) and balance > 0:
            if content[i] == '(': balance += 1
            elif content[i] == ')': balance -= 1
            i += 1
            
        # The content is everything up to the last closing paren
        layout_str = content[content_start : i-1]
        current_pos = i
        
        # Clean up
        layout_str = re.sub(r'//.*', '', layout_str)
        # layout_str = re.sub(r'/\*.*?\*/', '', layout_str, flags=re.DOTALL)
        layout_str = layout_str.replace('\n', ' ').replace('\t', ' ')
        
        # Split by comma respecting nested parens
        keys = []
        paren_depth = 0
        current_key = ""
        for char in layout_str:
            if char == '(': paren_depth += 1
            elif char == ')': paren_depth -= 1
            
            if char == ',' and paren_depth == 0:
                keys.append(current_key.strip())
                current_key = ""
            else:
                current_key += char
        if current_key:
            keys.append(current_key.strip())
            
        layers[layer_num] = keys

    return layers

def main():
    info = load_info_json('qmk_firmware/keyboards/bastardkb/charybdis/4x6/info.json')
    layers = parse_keymap('keymap/keymap.c')
    led_layout = info['rgb_matrix']['layout']
    logical_layout = info['layouts']['LAYOUT']['layout']
    
    matrix_to_led = {}
    for idx, led in enumerate(led_layout):
        if 'matrix' in led: matrix_to_led[tuple(led['matrix'])] = idx
            
    logical_to_matrix = {}
    for idx, key in enumerate(logical_layout):
        logical_to_matrix[idx] = tuple(key['matrix'])

    def get_led(logical_idx):
        if logical_idx in logical_to_matrix:
            m = logical_to_matrix[logical_idx]
            return matrix_to_led.get(m)
        return None

    def print_layer_logic(layer_num, layer_name, color_r, color_g, color_b, keys_to_match):
        l_indices = []
        r_indices = []
        
        layer_keys = layers.get(layer_num, [])
        for i, k in enumerate(layer_keys):
            # Special case for L4 (Left Side Only) logic handling
            if layer_num == 4:
                # For L4, we want all valid LEDs on the left side
                # Left side global indices are 0-28
                # So we check if the LED index is < 29
                led = get_led(i)
                if led is not None:
                    if led < 29:
                        l_indices.append(led)
                continue

            if any(x in k for x in keys_to_match):
                led = get_led(i)
                if led is not None:
                    if led < 29:
                        l_indices.append(led)
                    else:
                        r_indices.append(led - 29) # Normalize for local addressing

        print(f"        case {layer_num}: {{")
        print(f"            // {layer_name}")
        if l_indices:
            print(f"            if (is_keyboard_left()) {{")
            print(f"                static const uint8_t left[] = {{ {', '.join(map(str, l_indices))} }};")
            print(f"                for (int i=0; i<sizeof(left); i++) rgb_matrix_set_color(left[i], {color_r}, {color_g}, {color_b});")
            print(f"            }}")
        
        if r_indices:
            print(f"            if (!is_keyboard_left()) {{")
            print(f"                static const uint8_t right[] = {{ {', '.join(map(str, r_indices))} }};")
            print(f"                for (int i=0; i<sizeof(right); i++) rgb_matrix_set_color(right[i], {color_r}, {color_g}, {color_b});")
            print(f"            }}")
        print("            break;")
        print("        }")

    print("bool rgb_matrix_indicators_user(void) {")
    print("    if (is_flashlight) { rgb_matrix_set_color_all(255, 255, 255); return false; }")
    print("    uint8_t layer = get_highest_layer(layer_state);")
    print("    switch (layer) {")

    print_layer_logic(1, "Numpad (Blue)", 0, 0, 255, 
                      ["KC_P0", "KC_P1", "KC_P2", "KC_P3", "KC_P4", "KC_P5", "KC_P6", "KC_P7", "KC_P8", "KC_P9", "KC_PPLS", "KC_PMNS", "KC_PAST", "KC_PSLS", "KC_PEQL", "KC_DOT", "KC_COMM"])
    
    print_layer_logic(2, "Movement (Green)", 0, 255, 0,
                      ["KC_UP", "KC_DOWN", "KC_LEFT", "KC_RGHT", "KC_HOME", "KC_END", "KC_PGUP", "KC_PGDN"])
                      
    print_layer_logic(3, "Mouse (Yellow)", 255, 255, 0,
                      ["MS_UP", "MS_DOWN", "MS_LEFT", "MS_RGHT", "KC_MS_FAST", "KC_MS_DIAG", "MS_BTN", "SNIP", "DRG", "SCR", "DPI", "TURBO", "LOCK"])

    # Layer 4 is special in the function above, keys_to_match is ignored/dummy
    print_layer_logic(4, "Left Side (Pink)", 255, 0, 255, [])

    print("        default: break;")
    print("    }")
    print("    // Flash number key logic for show_mode (Master Only) would go here")
    print("    if (show_mode_active) {")
    print("        uint8_t digit = show_mode_digits[show_mode_current_digit];")
    print("        uint8_t led_index = (digit == 0) ? number_key_leds[9] : number_key_leds[digit - 1];")
    print("        uint8_t letter_index = (digit == 0) ? letter_key_leds[9] : letter_key_leds[digit - 1];")
    print("        bool am_i_left = is_keyboard_left();")
    print("        uint8_t val = (show_mode_phase == 1) ? 255 : 0;")
    print("        // Local addressing fix:")
    print("        if (am_i_left) {")
    print("            if (led_index < 29) rgb_matrix_set_color(led_index, val, val, val);")
    print("            if (letter_index < 29) rgb_matrix_set_color(letter_index, val, val, val);")
    print("        } else {")
    print("            if (led_index >= 29) rgb_matrix_set_color(led_index - 29, val, val, val);")
    print("            if (letter_index >= 29) rgb_matrix_set_color(letter_index - 29, val, val, val);")
    print("        }")
    print("    }")
    print("    return false;")
    print("}")

if __name__ == "__main__":
    main()