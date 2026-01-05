import re

def parse_layout(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    # Extract the keymaps array
    keymaps_match = re.search(r'const uint16_t PROGMEM keymaps\[\]\[MATRIX_ROWS\]\[MATRIX_COLS\] = \{(.*?)\};', content, re.DOTALL)
    if not keymaps_match:
        print("Could not find keymaps array.")
        return

    keymaps_str = keymaps_match.group(1)
    
    # Extract individual layers
    layers = {}
    # Regex to find [N] = LAYOUT(...)
    layer_matches = re.finditer(r'\[(\d+)\] = LAYOUT\((.*?)\)', keymaps_str, re.DOTALL)
    
    for match in layer_matches:
        layer_num = int(match.group(1))
        # Split by comma, handling potential nested parenthesis like LT(3, KC_SLSH)
        # simplistic splitting might fail on function macros with commas, but let's try a smarter split or just manual observation
        layout_str = match.group(2)
        # Remove newlines and extra spaces
        layout_str = " ".join(layout_str.split())
        
        # Simple split by comma (approximation, might break on complex macros)
        # Better: use a simple stack to split by comma outside parens
        keys = []
        current_key = ""
        paren_depth = 0
        for char in layout_str:
            if char == '(':
                paren_depth += 1
                current_key += char
            elif char == ')':
                paren_depth -= 1
                current_key += char
            elif char == ',' and paren_depth == 0:
                keys.append(current_key.strip())
                current_key = ""
            else:
                current_key += char
        keys.append(current_key.strip())
        
        layers[layer_num] = keys

    return layers

def print_aligned(layers):
    base = layers.get(0, [])
    mouse = layers.get(3, [])

    if len(base) != len(mouse):
        print(f"Warning: Layer lengths differ! Base: {len(base)}, Mouse: {len(mouse)}")
    
    print(f"{'Index':<5} | {'Base Layer':<20} | {'Mouse Layer (3)':<20}")
    print("-" * 50)
    
    for i in range(max(len(base), len(mouse))):
        b_key = base[i] if i < len(base) else "N/A"
        m_key = mouse[i] if i < len(mouse) else "N/A"
        print(f"{i:<5} | {b_key:<20} | {m_key:<20}")

layers = parse_layout('/home/dcar/projects/mech-keyboard/qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/keymap.c')
print_aligned(layers)