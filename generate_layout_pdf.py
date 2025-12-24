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
    1: (0.7, 0.7, 1.0),      # Blue (Symbols)
    2: (0.7, 1.0, 0.7),      # Green (Media)
    3: (1.0, 1.0, 0.7),      # Yellow (Mouse)
    4: (0.7, 1.0, 1.0),      # Cyan (One-Hand)
    5: (0.9, 0.7, 0.9),      # Purple (Numpad)
}

LAYER_NAMES = {
    0: "BASE",
    1: "SYMBOLS",
    2: "MEDIA",
    3: "MOUSE",
    4: "ONE-HAND",
    5: "NUMPAD",
}

# Key label simplifications
KEY_LABELS = {
    'KC_ESC': 'Esc',
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
    'KC_BSPC': 'Bksp',
    'KC_ENT': 'Enter',
    'KC_DEL': 'Del',
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
    'KC_CAPS': 'Caps',
    'KC_HOME': 'Home',
    'KC_END': 'End',
    'KC_PGUP': 'PgUp',
    'KC_PGDN': 'PgDn',
    'KC_UP': 'Up',
    'KC_DOWN': 'Down',
    'KC_LEFT': 'Left',
    'KC_RGHT': 'Right',
    'KC_TRNS': '',
    'KC_NO': '',
    'KC_MPLY': 'Play',
    'KC_MNXT': 'Next',
    'KC_MPRV': 'Prev',
    'KC_VOLU': 'Vol+',
    'KC_VOLD': 'Vol-',
    'KC_MUTE': 'Mute',
    'KC_PPLS': 'Num+',
    'KC_PMNS': 'Num-',
    'KC_PAST': 'Num*',
    'KC_PSLS': 'Num/',
    'KC_PEQL': 'Num=',
    'KC_PDOT': 'Num.',
    'KC_P0': 'Num0',
    'KC_P1': 'Num1',
    'KC_P2': 'Num2',
    'KC_P3': 'Num3',
    'KC_P4': 'Num4',
    'KC_P5': 'Num5',
    'KC_P6': 'Num6',
    'KC_P7': 'Num7',
    'KC_P8': 'Num8',
    'KC_P9': 'Num9',
    'QK_BOOT': 'BOOT',
    'QK_CLEAR_EEPROM': 'EECLR',
    'MS_BTN1': 'LClick',
    'MS_BTN2': 'RClick',
    'MS_BTN3': 'MClick',
    'MS_UP': 'M-Up',
    'MS_DOWN': 'M-Down',
    'MS_LEFT': 'M-Left',
    'MS_RGHT': 'M-Right',
    'SNIPING': 'Snipe',
    'DRGSCRL': 'Scroll',
    'DPI_MOD': 'DPI',
    'S_D_MOD': 'S-DPI',
    'RM_NEXT': 'RGB>',
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
    return data['layouts']['LAYOUT']['layout']


def simplify_key(key_code):
    """Convert QMK keycode to readable label."""
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
        return f'{key}/L{layer}'

    # Handle TD(...)
    td_match = re.match(r'TD\((\w+)\)', key_code)
    if td_match:
        name = td_match.group(1)
        if 'Z' in name:
            return 'Z/TD'
        return 'TD'

    # Handle custom keycodes
    custom_map = {
        'KC_Q_TG4': 'Q/TG4',
        'KC_P_TO0': 'P/TO0',
        'KC_X_TG2': 'X/TG2',
        'KC_C_TG1': 'C/TG1',
        'KC_V_TG5': 'V/TG5',
        'KC_L_TG1': 'L1 Tgl',
        'KC_R_TG2': 'L2 Tgl',
        'KC_ENT_MO4': 'Ent/L4',
        'KC_ENT_EXIT': 'Ent/Ex',
        'KC_RAINBOW': 'Rainbow',
        'KC_REACTIVE': 'React',
        'KC_MOUSE_LOCK': 'M-Lock',
        'KC_MS_FAST_UP': 'M-Up+',
        'KC_MS_FAST_DOWN': 'M-Dn+',
        'KC_MS_FAST_LEFT': 'M-Lt+',
        'KC_MS_FAST_RIGHT': 'M-Rt+',
        'KC_MS_DIAG_UL': 'M-UL',
        'KC_MS_DIAG_UR': 'M-UR',
        'KC_MS_DIAG_DL': 'M-DL',
        'KC_MS_DIAG_DR': 'M-DR',
        'KC_SCR_MODE': 'ScrMod',
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

    return key_code[:8]


def draw_key(c, x, y, width, height, label, bg_color):
    """Draw a single key with label."""
    # Key background
    c.setFillColorRGB(*bg_color)
    c.roundRect(x, y, width, height, 4, fill=1, stroke=1)

    # Key label
    c.setFillColor(colors.black)

    # Adjust font size based on label length
    if len(label) <= 2:
        font_size = 10
    elif len(label) <= 4:
        font_size = 8
    elif len(label) <= 6:
        font_size = 7
    else:
        font_size = 6

    c.setFont("Helvetica-Bold", font_size)

    # Center text
    text_width = c.stringWidth(label, "Helvetica-Bold", font_size)
    text_x = x + (width - text_width) / 2
    text_y = y + (height - font_size) / 2 + 2

    c.drawString(text_x, text_y, label)


def draw_layer(c, layer_keys, layout_info, layer_num, start_x, start_y, key_size=28):
    """Draw a complete layer on the canvas at specified position."""
    layer_name = LAYER_NAMES.get(layer_num, f"Layer {layer_num}")
    bg_color = LAYER_COLORS.get(layer_num, (0.9, 0.9, 0.9))

    # Title
    c.setFont("Helvetica-Bold", 12)
    c.setFillColor(colors.black)
    title = f"{layer_name} (L{layer_num})"
    c.drawString(start_x, start_y, title)

    # Calculate layout bounds
    max_x = max(k['x'] for k in layout_info)
    max_y = max(k['y'] for k in layout_info)

    # Key dimensions
    key_gap = 3

    # Adjust vertical offset for title - increased spacing
    offset_y = start_y - 35

    c.setStrokeColor(colors.gray)
    c.setLineWidth(0.5)

    for i, key_code in enumerate(layer_keys):
        if i >= len(layout_info):
            break

        info = layout_info[i]
        kx = start_x + info['x'] * (key_size + key_gap)
        ky = offset_y - (info['y']) * (key_size + key_gap)

        label = simplify_key(key_code)
        draw_key(c, kx, ky, key_size, key_size, label, bg_color)

    # Return height used for this layer
    return (max_y + 1) * (key_size + key_gap) + 30


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
    margin = 0.4 * inch
    key_size = 24  # Smaller keys to fit 3 layers

    for i in range(0, len(layer_nums), 3):
        current_y = page_height - margin

        # Draw up to 3 layers on this page
        for j in range(3):
            if i + j >= len(layer_nums):
                break

            layer_num = layer_nums[i + j]
            height_used = draw_layer(c, layers[layer_num], layout_info, layer_num,
                                     margin, current_y, key_size=key_size)
            current_y -= height_used + 30  # Space between layers

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
