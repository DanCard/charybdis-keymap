#!/usr/bin/env python3
"""
Generate a PDF of the Charybdis 4x6 keyboard layout.
Requires: pip install reportlab
"""

import json
import re
import os
from reportlab.lib import colors
from reportlab.lib.pagesizes import LETTER
from reportlab.lib.units import inch
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

# Layer colors matching the RGB settings in keymap.c
LAYER_COLORS = {
    0: (0.9, 0.9, 0.9),      # White/Light gray (Base)
    1: (0.6, 0.9, 1.0),      # Light Blue (Symbols + Numpad)
    2: (0.7, 1.0, 0.7),      # Green (Media)
    3: (1.0, 1.0, 0.7),      # Yellow (Mouse)
    4: (0.7, 1.0, 1.0),      # Cyan (One-Hand)
}

LAYER_NAMES = {
    0: "BASE",
    1: "SYMBOLS + NUMPAD",
    2: "MEDIA",
    3: "MOUSE",
    4: "ONE-HAND",
}

# Key label simplifications
KEY_LABELS = {
    'KC_ESC': 'Escape',
    'KC_TAB': 'Tab',
    'KC_LSFT': 'Shift',
    'KC_RSFT': 'Shift',
    'KC_LCTL': 'Ctrl',
    'KC_RCTL': 'Ctrl',
    'KC_LALT': 'Alt',
    'KC_RALT': 'Alt',
    'KC_LGUI': 'Win',
    'KC_RGUI': 'Win',
    'KC_SPC': 'Space',
    'KC_BSPC': 'Back\nSpace',
    'KC_ENT': 'Enter',
    'KC_DEL': 'Delete',
    'KC_MINS': '-',
    'KC_EQL': '=',
    'KC_LBRC': '[',
    'KC_RBRC': ']',
    'KC_BSLS': '\\',
    'KC_SCLN': ';',
    'KC_QUOT': "'",
    'KC_GRV': '`',
    'KC_COMM': ',',
    'KC_DOT': '.',
    'KC_SLSH': '/',
    'KC_CAPS': 'Caps\nLock',
    'KC_HOME': 'Home',
    'KC_END': 'End',
    'KC_PGUP': 'Page\nUp',
    'KC_PGDN': 'Page\nDown',
    'KC_UP': 'Up',
    'KC_DOWN': 'Down',
    'KC_LEFT': 'Left',
    'KC_RGHT': 'Right',
    'KC_TRNS': '',
    'KC_NO': '',
    'KC_MPLY': 'Play',
    'KC_MNXT': 'Next',
    'KC_MPRV': 'Prev',
    'KC_VOLU': 'Vol\nUp',
    'KC_VOLD': 'Vol\nDown',
    'KC_MUTE': 'Mute',
    'KC_PPLS': 'Num +',
    'KC_PMNS': 'Num -',
    'KC_PAST': 'Num *',
    'KC_PSLS': 'Num /',
    'KC_PEQL': 'Num =',
    'KC_PDOT': 'Num .',
    'KC_P0': 'Num 0',
    'KC_P1': 'Num 1',
    'KC_P2': 'Num 2',
    'KC_P3': 'Num 3',
    'KC_P4': 'Num 4',
    'KC_P5': 'Num 5',
    'KC_P6': 'Num 6',
    'KC_P7': 'Num 7',
    'KC_P8': 'Num 8',
    'KC_P9': 'Num 9',
    'QK_BOOT': 'BOOT',
    'QK_CLEAR_EEPROM': 'EE\nCLR',
    'MS_BTN1': 'Left\nClick',
    'MS_BTN2': 'Right\nClick',
    'MS_BTN3': 'Middle\nClick',
    'MS_UP': 'Mouse\nUp',
    'MS_DOWN': 'Mouse\nDown',
    'MS_LEFT': 'Mouse\nLeft',
    'MS_RGHT': 'Mouse\nRight',
    'SNIPING': 'Snipe',
    'DRGSCRL': 'Scroll',
    'DPI_MOD': 'DPI',
    'S_D_MOD': 'S-DPI',
    'RM_NEXT': 'RGB >',
}

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


def parse_info_json(file_path):
    with open(file_path, 'r') as f:
        data = json.load(f)
    layout = data['layouts']['LAYOUT']['layout']

    # Adjust layout to minimize whitespace and center thumbs
    for key in layout:
        # Right Main (rows 0-3, x >= 11) -> Shift Left by 4
        if key['y'] < 4 and key['x'] >= 11:
            key['x'] -= 4.0
        
        # Left Thumbs (y >= 4, x < 9) -> Shift Left by 2 (Center under Left Main)
        elif key['y'] >= 4 and key['x'] < 9:
            key['x'] -= 2.0
            
        # Right Thumbs (y >= 4, x >= 9) -> Shift Left by 1 (Center under Right Main)
        elif key['y'] >= 4 and key['x'] >= 9:
            key['x'] -= 1.0

    return layout


def simplify_key(key_code, layer_num=None):
    """Convert QMK keycode to readable label."""
    # Special handling for Layer 0 Long Press
    if layer_num == 0:
        if 'TD_Z_LAYER' in key_code: return 'Z\nMouse\nLight'
        if key_code == 'KC_1_TG1': return '1\nL1'
        if key_code == 'KC_2_TG2': return '2\nL2'
        if key_code == 'KC_3_TG3': return '3\nL3'
        if key_code == 'KC_4_TG4': return '4\nL4'

    # Special handling for Layer 1
    if layer_num == 1:
        if key_code == 'KC_1_TG1': return '1\nL0'
        if key_code == 'KC_2_TG2': return '2\nL0'
        if key_code == 'KC_3_TG3': return '3\nL0'
        if key_code == 'KC_4_TG4': return '4\nL0'

    # Special handling for Layer 2
    if layer_num == 2:
        if 'TD_Z_LAYER' in key_code: return 'Home'
        if 'KC_X_TG2' == key_code: return 'Page\nUp'

    # Special handling for other layer long press returns
    if layer_num is not None and layer_num > 1:
        if key_code == 'KC_1_TG1':
            label = 'F1' if layer_num == 2 else ('Rainb' if layer_num == 3 else '0')
            return f'{label}\nL0'
        if key_code == 'KC_2_TG2':
            label = 'F2' if layer_num == 2 else ('Next' if layer_num == 3 else '9')
            return f'{label}\nL0'
        if key_code == 'KC_3_TG3':
            label = 'F3' if layer_num == 2 else '3'
            return f'{label}/L0'
        if key_code == 'KC_4_TG4':
            label = 'F4' if layer_num == 2 else '4'
            return f'{label}\nL0'

    if key_code in KEY_LABELS:
        return KEY_LABELS[key_code]

    # Handle S(KC_X) for shifted keys
    shift_match = re.match(r'S\(KC_(\w+)\)', key_code)
    if shift_match:
        char = shift_match.group(1)
        shift_map = {
            '1': '!', '2': '@', '3': '#', '4': '$', '5': '%',
            '6': '^', '7': '&', '8': '*', '9': '(', '0': ')',
            'MINS': '_', 'EQL': '+', 'GRV': '~',
            'LBRC': '{', 'RBRC': '}', 'BSLS': '|',
            'SCLN': ':', 'QUOT': '"', 'COMM': '<', 'DOT': '>', 'SLSH': '?',
        }
        return shift_map.get(char, f'S-{char}')

    # Handle LT(layer, key)
    lt_match = re.match(r'LT\((\d+),\s*KC_(\w+)\)', key_code)
    if lt_match:
        layer = lt_match.group(1)
        key = lt_match.group(2)
        # Simplify the inner key if possible
        inner_key_code = f'KC_{key}'
        label = KEY_LABELS.get(inner_key_code, key)
        return f'{label}\nL{layer}'

    # Handle TD(...)
    td_match = re.match(r'TD\((\w+)\)', key_code)
    if td_match:
        return 'TD'

    # Handle custom keycodes
    custom_map = {
        'KC_Q_TG4': 'Q\nTG4',
        'KC_P_TO0': 'P\nTO0',
        'KC_X_TG2': 'X\nTG2',
        'KC_V_TG5': 'V\nTG5',
        'KC_L_TG1': 'L1\nTgl',
        'KC_R_TG2': 'L2\nTgl',
        'KC_ENT_MO4': 'Enter\nL4',
        'KC_ENT_EXIT': 'Enter\nExit',
        'KC_SPC_EXIT': 'Space\nExit',
        'KC_BSPC_EXIT': 'Back\nExit',
        'KC_EXIT': 'Exit',
        'KC_TURBO': 'Turbo',
        'KC_RAINBOW': 'Rain\nbow',
        'KC_REACTIVE': 'Reac\ntive',
        'KC_MOUSE_LOCK': 'Mouse\nLock',
        'KC_MS_FAST_UP': 'Mouse\nUp+',
        'KC_MS_FAST_DOWN': 'Mouse\nDown+',
        'KC_MS_FAST_LEFT': 'Mouse\nLeft+',
        'KC_MS_FAST_RIGHT': 'Mouse\nRight+',
        'KC_MS_DIAG_UL': 'Up\nLeft',
        'KC_MS_DIAG_UR': 'Up\nRight',
        'KC_MS_DIAG_DL': 'Down\nLeft',
        'KC_MS_DIAG_DR': 'Down\nRight',
        'KC_SCR_MODE': 'Scr\nMod',
        'KC_1_TG1': '1\nL1',
        'KC_2_TG2': '2\nL2',
        'KC_3_TG3': '3\nL3',
        'KC_4_TG4': '4\nL4',
        'KC_JELLY': 'Jelly',
        'KC_SPIRAL': 'Spiral',
        'KC_CHEVRON': 'Chevrn',
        'KC_RGB_AUTO': 'RGB\nAuto',
        'KC_PLUS_COLON': '+\n:',
    }
    if key_code in custom_map:
        return custom_map[key_code]

    # Strip KC_ prefix for simple keys
    if key_code.startswith('KC_'):
        rest = key_code[3:]
        if len(rest) == 1:
            return rest
        if rest.startswith('F') and rest[1:].isdigit():
            return rest
        return rest

    return key_code[:10]


def draw_key(c, x, y, width, height, label, bg_color):
    """Draw a single key with label."""
    # Key background
    c.setFillColorRGB(*bg_color)
    c.roundRect(x, y, width, height, 4, fill=1, stroke=1)

    # Key label
    c.setFillColor(colors.black)

    # Split lines
    lines = label.split('\n')
    
    # Determine max line length for font sizing
    max_len = max(len(line) for line in lines)
    
    # Adjust font size based on label length
    if max_len <= 2:
        font_size = 10
    elif max_len <= 4:
        font_size = 8
    elif max_len <= 5:
        font_size = 7
    elif max_len <= 7:
        font_size = 6
    else:
        font_size = 5.5

    c.setFont("Helvetica-Bold", font_size)
    
    # Calculate vertical starting position to center the block of text
    line_height = font_size + 2
    total_text_height = len(lines) * line_height
    
    start_text_y = y + (height + total_text_height) / 2 - line_height + 2
    
    if len(lines) > 1:
        # Tweak for multi-line to center better visually
        start_text_y += 1 

    for i, line in enumerate(lines):
        text_width = c.stringWidth(line, "Helvetica-Bold", font_size)
        text_x = x + (width - text_width) / 2
        text_y = start_text_y - (i * line_height)
        c.drawString(text_x, text_y, line)


def draw_layer(c, layer_keys, layout_info, layer_num, start_x, start_y, key_size=28):
    """Draw a complete layer on the canvas at specified position."""
    layer_name = LAYER_NAMES.get(layer_num, f"Layer {layer_num}")
    bg_color = LAYER_COLORS.get(layer_num, (0.9, 0.9, 0.9))

    # Title
    c.setFont("Helvetica-Bold", 12)
    c.setFillColor(colors.black)
    title = f"{layer_name} (L{layer_num})"
    title_width = c.stringWidth(title, "Helvetica-Bold", 12)
    title_x = (LETTER[0] - title_width) / 2
    c.drawString(title_x, start_y, title)

    # Calculate layout bounds
    max_x = max(k['x'] for k in layout_info)
    max_y = max(k['y'] for k in layout_info)

    # Key dimensions
    key_gap = 1

    # Adjust vertical offset for title - reduced spacing
    offset_y = start_y - 40

    c.setStrokeColor(colors.gray)
    c.setLineWidth(0.5)

    for i, key_code in enumerate(layer_keys):
        if i >= len(layout_info):
            break

        info = layout_info[i]
        kx = start_x + info['x'] * (key_size + key_gap)
        ky = offset_y - (info['y']) * (key_size + key_gap)

        label = simplify_key(key_code, layer_num)
        draw_key(c, kx, ky, key_size, key_size, label, bg_color)

    # Return height used for this layer
    return (max_y + 1) * (key_size + key_gap) + 10


def generate_pdf(output_path, keymap_path, info_path):
    """Generate the PDF with all layers."""
    layers = parse_keymap(keymap_path)
    layout_info = parse_info_json(info_path)

    if not layers:
        print("No layers found!")
        return

    page_size = LETTER  # Portrait orientation
    c = canvas.Canvas(output_path, pagesize=page_size)
    page_width, page_height = page_size

    # Layer pages - fit 3 layers per page
    layer_nums = sorted(layers.keys())
    margin = 0.15 * inch
    key_size = 36  # Slightly smaller to fit

    for i in range(0, len(layer_nums), 3):
        current_y = page_height - margin

        # Draw up to 3 layers on this page
        for j in range(3):
            if i + j >= len(layer_nums):
                break

            layer_num = layer_nums[i + j]
            height_used = draw_layer(c, layers[layer_num], layout_info, layer_num,
                                     margin, current_y, key_size=key_size)
            current_y -= height_used + 15  # Space between layers

        c.showPage()

    c.save()
    print(f"PDF saved to: {output_path}")

def main():
    base_path = '/home/dcar/projects/mech-keyboard'
    keymap_path = f'{base_path}/qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/keymap.c'
    info_path = f'{base_path}/qmk_firmware/keyboards/bastardkb/charybdis/4x6/info.json'
    output_path = f'{base_path}/charybdis_layout.pdf'

    if not os.path.exists(keymap_path):
        print(f"Error: keymap not found at {keymap_path}")
        return
    if not os.path.exists(info_path):
        print(f"Error: info.json not found at {info_path}")
        return

    generate_pdf(output_path, keymap_path, info_path)


if __name__ == "__main__":
    main()