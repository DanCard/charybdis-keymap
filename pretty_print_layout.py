import json
import re
import sys
import os

def parse_keymap(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    # Extract the keymaps array
    keymaps_match = re.search(r'const uint16_t PROGMEM keymaps\[\]\[MATRIX_ROWS\]\[MATRIX_COLS\] = \{(.*?)\};', content, re.DOTALL)
    if not keymaps_match:
        print("Could not find keymaps array.")
        return {}

    keymaps_str = keymaps_match.group(1)
    
    layers = {}
    
    # Iterate through the string to find "LAYOUT(" and balance parens
    search_start = 0
    while True:
        # Find start of a layer definition, e.g., [0] = LAYOUT(
        match = re.search(r'\[(\d+)\]\s*=\s*LAYOUT\(', keymaps_str[search_start:])
        if not match:
            break
        
        layer_num = int(match.group(1))
        start_index = search_start + match.end()
        
        # Find the matching closing parenthesis for LAYOUT
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
            
        # Clean up the layout string
        # Remove comments (C style)
        layout_str = re.sub(r'//.*', '', layout_content)
        layout_str = re.sub(r'/\*.*?\*/', '', layout_str, flags=re.DOTALL)
        # Remove newlines and extra spaces
        layout_str = " ".join(layout_str.split())
        
        # Split by comma, respecting parentheses
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

def parse_info_json(file_path):
    with open(file_path, 'r') as f:
        data = json.load(f)
    return data['layouts']['LAYOUT']['layout']

def print_layer(layer_keys, layout_info, layer_name):
    # Determine grid size
    max_x = 0
    max_y = 0
    for key_info in layout_info:
        # Check if x or y are floats, convert if necessary (though usually ints in info.json)
        x = float(key_info['x'])
        y = float(key_info['y'])
        if x > max_x: max_x = x
        if y > max_y: max_y = y
    
    # Scale for ASCII art
    scale_x = 12  # Width of a key entry
    scale_y = 3   # Height of a key entry (line spacing)
    
    # +1 for buffer
    width = int((max_x + 1) * scale_x) + scale_x
    height = int((max_y + 1) * scale_y) + scale_y
    
    grid = [[' ' for _ in range(width)] for _ in range(height)]
    
    print(f"\nLayer: {layer_name}")
    print("-" * 40)

    for i, key_code in enumerate(layer_keys):
        if i >= len(layout_info):
            break
        
        info = layout_info[i]
        x_pos = int(float(info['x']) * scale_x)
        y_pos = int(float(info['y']) * scale_y)
        
        # Truncate key_code if too long
        display_text = key_code[:scale_x-2]
        
        # Place text in grid
        for idx, char in enumerate(display_text):
            if x_pos + idx < width and y_pos < height:
                grid[y_pos][x_pos + idx] = char

    # Print grid
    # Filter empty lines to make it more compact if needed, but keeping geometry is better
    for row in grid:
        line = "".join(row).rstrip()
        if line:
            print(line)

def main():
    keymap_path = '/home/dcar/projects/mech-keyboard/qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/keymap.c'
    info_path = '/home/dcar/projects/mech-keyboard/qmk_firmware/keyboards/bastardkb/charybdis/4x6/info.json'
    
    if not os.path.exists(keymap_path):
        print(f"Error: keymap file not found at {keymap_path}")
        return
    if not os.path.exists(info_path):
        print(f"Error: info.json file not found at {info_path}")
        return

    layers = parse_keymap(keymap_path)
    layout_info = parse_info_json(info_path)
    
    if not layers:
        print("No layers found in keymap file.")
        return

    # Sort layer indices
    layer_indices = sorted(layers.keys())
    
    for idx in layer_indices:
        print_layer(layers[idx], layout_info, f"Layer {idx}")

if __name__ == "__main__":
    main()
