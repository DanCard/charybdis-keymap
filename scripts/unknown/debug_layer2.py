
import re

def parse_keymap(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    keymaps_match = re.search(
        r'const uint16_t PROGMEM keymaps\[\]\[MATRIX_ROWS\]\[MATRIX_COLS\] = \{(.*?)\};',
        content, re.DOTALL
    )
    if not keymaps_match:
        return {}

    keymaps_str = keymaps_match.group(1)
    layers = {}
    search_start = 0

    while True:
        match = re.search(r'\[(\d+)\]\s*=\s*LAYOUT\(', keymaps_str[search_start:])
        if not match:
            break

        layer_num = int(match.group(1))
        start_index = search_start + match.end()

        paren_depth = 1
        current_index = start_index
        layout_content = ""

        while current_index < len(keymaps_str) and paren_depth > 0:
            char = keymaps_str[current_index]
            if char == '(': 
                paren_depth += 1
            elif char == ')':
                paren_depth -= 1
            if paren_depth > 0:
                layout_content += char
            current_index += 1

        layout_str = re.sub(r'//.*', '', layout_content)
        layout_str = re.sub(r'/\*.*?\*/', '', layout_str, flags=re.DOTALL)
        layout_str = " ".join(layout_str.split())

        keys = []
        current_key = ""
        depth = 0
        for char in layout_str:
            if char == '(': 
                depth += 1
                current_key += char
            elif char == ')':
                depth -= 1
                current_key += char
            elif char == ',' and depth == 0:
                keys.append(current_key.strip())
                current_key = ""
            else:
                current_key += char
        if current_key:
            keys.append(current_key.strip())

        layers[layer_num] = keys
        search_start = current_index

    return layers

layers = parse_keymap('/home/dcar/projects/mech-keyboard/qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/keymap.c')
if 2 in layers:
    l2 = layers[2]
    # Indices 42-47 are Right Bottom
    print("Layer 2 Right Bottom (42-47):", l2[42:48])
else:
    print("Layer 2 not found")
