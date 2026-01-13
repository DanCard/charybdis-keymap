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
    1: (0.7, 0.7, 1.0),      # Blue (Numpad)
    2: (0.7, 1.0, 0.7),      # Green (Arrow)
    3: (1.0, 1.0, 0.7),      # Yellow (Mouse)
    4: (1.0, 0.5, 0.0),      # Orange (One-Hand)
    5: (1.0, 0.43, 0.59),    # Hot Pink (Settings)
}

LAYER_NAMES = {
    0: "BASE",
    1: "NUMPAD",
    2: "ARROW",
    3: "MOUSE",
    4: "ONE-HAND",
    5: "SETTINGS",
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
    'QK_GESC': 'Esc\n~\n`',
    'MS_BTN1': 'Left\nClick',
    'MS_BTN2': 'Right\nClick',
    'MS_BTN3': 'Middle\nClick',
    'MS_UP': 'Mouse\nUp',
    'MS_DOWN': 'Mouse\nDown',
    'MS_LEFT': 'Mouse\nLeft',
    'MS_RGHT': 'Mouse\nRight',
    'SNIPING': 'Snipe',
    'DRGSCRL': 'Scroll',
    'DPI_MOD': 'DPI+',
    'DPI_RMOD': 'DPI-',
    'S_D_MOD': 'S-DPI',
    'RM_NEXT': 'rgb +',
    'RM_PREV': 'rgb -',
    'RM_HUEU': 'Hue +',
    'RM_HUED': 'Hue -',
    'RM_SATU': 'Sat +',
    'RM_SATD': 'Sat -',
    'RM_VALU': 'Val +',
    'RM_VALD': 'Val -',
}

# Will be populated dynamically + manual entries
COMBOS = []

MANUAL_ACTIONS = [
    ("Z (Tap)", "Z"),
    ("Z (Hold)", "Layer 4"),
    ("Z (Dbl-Tap)", "Flashlight"),
    ("Snipe Active", "Right Top Black"),
    ("Snipe Active", "R-Col Rainbow"),
]

def readable_key(k):
    """Convert a single keycode to a readable short string for combo listing."""
    k = k.strip()
    if k.startswith('KC_'):
        k = k[3:]
    
    # Handle shifted keys like S(KC_V)
    if 'S(' in k:
        match = re.search(r'S\((?:KC_)?(\w+)\)', k)
        if match:
            return f"Shift+{match.group(1)}"
    
    # Handle Ctrl+Shift like C(S(KC_V))
    if 'C(S(' in k:
        match = re.search(r'C\(S\((?:KC_)?(\w+)\)\)', k)
        if match:
            return f"Ctrl+Shift+{match.group(1)}"
            
    # Handle Tap Dance
    if 'TD(' in k:
        return 'Z' # Special case for TD(TD_Z_LAYER)

    # Dictionary for common abbreviations
    lookup = {
        'LSFT': 'LShift', 'RSFT': 'RShift',
        'LCTL': 'Ctrl', 'RCTL': 'Ctrl',
        'LALT': 'Alt', 'RALT': 'Alt',
        'LGUI': 'Win', 'RGUI': 'Win',
        'BSPC': 'Bksp', 'DEL': 'Del',
        'PGUP': 'PgUp', 'PGDN': 'PgDn',
        'MINS': '-', 'EQL': '=',
        'LBRC': '[', 'RBRC': ']',
        'BSLS': '\\', 'SCLN': ';',
        'QUOT': "'", 'GRV': '`',
        'COMM': ',', 'DOT': '.',
        'SLSH': '/', 'SPC': 'Space',
        'ENT': 'Enter'
    }
    return lookup.get(k, k)

def extract_combos_from_keymap(file_path):
    """Parse keymap.c to find all combos defined in key_combos array."""
    try:
        with open(file_path, 'r') as f:
            content = f.read()

        # 1. Parse PROGMEM combo definitions: const uint16_t PROGMEM name[] = {K1, K2, COMBO_END};
        combo_defs = {}
        # Regex handles multiline definitions if needed, assuming brace closure
        # Fix: Escape brackets \[\]
        matches = re.finditer(r'const uint16_t PROGMEM (\w+)\[\]\s*=\s*\{([^}]+)\};', content)
        for match in matches:
            name = match.group(1)
            keys_str = match.group(2)
            # Filter out COMBO_END and whitespace
            keys = [k.strip() for k in keys_str.split(',') if 'COMBO_END' not in k and k.strip()]
            combo_defs[name] = keys

        # 2. Parse combo_t array: combo_t key_combos[] = { COMBO(name, action), ... };
        found_combos = []
        
        # Find the block
        match_block = re.search(r'combo_t\s+key_combos\[\]\s*=\s*\{([^;]+)\};', content, re.DOTALL)
        if match_block:
            block = match_block.group(1)
            
            # Simple parser for COMBO(name, action)
            # We iterate through the block string to handle nested parentheses in 'action'
            idx = 0
            while True:
                # Find next COMBO(
                start = block.find('COMBO(', idx)
                if start == -1:
                    break
                
                idx = start + 6 # Skip 'COMBO('
                depth = 1
                args_start = idx
                args_end = -1
                
                while idx < len(block) and depth > 0:
                    if block[idx] == '(':
                        depth += 1
                    elif block[idx] == ')':
                        depth -= 1
                        if depth == 0:
                            args_end = idx
                    idx += 1
                
                if args_end != -1:
                    args_str = block[args_start:args_end]
                    # args_str should be "name, action"
                    # Split by comma. name shouldn't have parens, so simple split is safe enough
                    # providing we strip whitespace
                    parts = args_str.split(',', 1)
                    if len(parts) == 2:
                        name = parts[0].strip()
                        action = parts[1].strip()
                        
                        if name in combo_defs:
                            # Convert keys to readable string
                            readable_keys = " + ".join([readable_key(k) for k in combo_defs[name]])
                            
                            # Convert action to readable string
                            readable_action = readable_key(action)
                            
                            # Heuristic for Ctrl+Shift+V
                            if 'Paste' in readable_action:
                                readable_action = 'Ctrl+Shift+V'
                            elif ('V' in readable_action or 'v' in readable_action) and \
                                 ('Shift' in readable_action or 'S(' in action) and \
                                 ('Ctrl' in readable_action or 'C(' in action):
                                 readable_action = 'Ctrl+Shift+V'

                            found_combos.append((readable_keys, readable_action))
        
        return found_combos

    except Exception as e:
        print(f"Error extracting combos: {e}")
        return []

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
    # Tap Dance Z-Layer Handling
    if 'TD_Z_LAYER' in key_code:
        if layer_num == 0: return 'Z\nL4\nLight'
        if layer_num == 1: return 'Num 0\nExit\nLight'
        if layer_num == 2: return 'Home\nExit\nLight'
        if layer_num == 3: return 'Z\nExit\nLight'
        if layer_num == 4: return '/\nExit\nLight'
        return 'Z\nTD'

    # Special handling for Layer 0 Long Press
    if layer_num == 0:
        if key_code == 'KC_1_TG1': return '1\nL1'
        if key_code == 'KC_2_TG2': return '2\nL2'
        if key_code == 'KC_3_TG3': return '3\nL3'
        if key_code == 'KC_4_TG4': return '4\nL4'
        if key_code == 'KC_5_TG5': return '5\nL5'

    # Special handling for Layer 1
    if layer_num == 1:
        if key_code == 'KC_1_TG1': return '1\nL0'
        if key_code == 'KC_2_TG2': return '2\nL0'
        if key_code == 'KC_3_TG3': return '3\nL0'
        if key_code == 'KC_4_TG4': return '4\nL0'

    # Special handling for Layer 2
    if layer_num == 2:
        if 'KC_X_TG2' == key_code: return 'Page\nUp'

    if layer_num is not None and layer_num > 0:
        if key_code == 'KC_L_TG1':
            return 'Exit\nL3'

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
            return f'{label}\nExit'
        if key_code == 'KC_4_TG4':
            label = 'F4' if layer_num == 2 else '4'
            return f'{label}\nExit'

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
    'KC_X': 'X',
        'KC_P_TO0': 'P\nExit',
        'KC_X_TG2': 'X\nTG2',
        'KC_V_TG5': 'V\nTG5',
        'KC_Q_TG4': 'Q\nL4',
        'KC_L_TG1': 'L3\nL1',
        'KC_R_TG2': 'L2\nTgl',
        'KC_ENT_MO4': 'Enter\nL4',
        'KC_ENT_EXIT': 'Enter\nExit',
        'KC_SPC_EXIT': 'Space\nExit',
        'KC_BSPC_EXIT': 'Back\nExit',
        'KC_EXIT': 'Exit',
        'KC_TURBO': 'Temp\nTurbo',
        'KC_RAINBOW': 'Rain\nbow',
        'KC_REACTIVE': 'Reac\nctive',
        'KC_MOUSE_LOCK': 'Layer\nLock',
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
        'KC_5_TG5': '5\nL5',
        'RM_TOGG': 'RGB\nToggle',
        'KC_JELLY': 'Jelly',
        'KC_SPIRAL': 'Spiral',
        'KC_CHEVRON': 'Chev\nron',
        'KC_LR_TOGGLE': 'LR\nToggle',
        'KC_FLASH': 'Flash',
        'KC_RAINBOW': 'Rain\nbow',
        'KC_SNIPE': 'Snipe',
        'KC_FAST': 'Fast',
        'KC_SCR_LOCK': 'Scroll\nLock',
        'KC_RGB_AUTO': 'RGB\nAuto',
        'KC_PLUS_COLON': '+ \n:',
        'RM_HUEU': 'Hue\n+',
        'RM_HUED': 'Hue\n-',
        'RM_SATU': 'Sat\n+',
        'RM_SATD': 'Sat\n-',
        'RM_VALU': 'Brt\n+',
        'RM_VALD': 'Brt\n-',
        'KC_MINS_TO0': '- \nExit',
        'KC_0_TG1': '0\nL1',
        'KC_9_TG2': '9\nL2',
        'KC_8_TG3': '8\nL3',
        'KC_P_FRAC': 'Pixel\nFrac',
        'KC_PINWHEEL': 'Pin\nwheel',
        'KC_7_TO0': '7\nExit',
        'KC_6_TO0': '6\nExit',
        'KC_SPC_TG2': 'Space\nL2',
        'KC_SPC_TG4': 'Space\nL4',
        'KC_ENT_TG2': 'Enter\nL2',
        'KC_ENT_TG4': 'Enter\nL4',
        'KC_PMNS_TG4': 'Num -\nL4',
        'KC_F12_EXIT': 'F12\nExit',
        'KC_MS_TMO_INC': 'Time\nout +',
        'KC_MS_TMO_DEC': 'Time\nout -',
        'KC_P_FRAC': 'Pixel\nFractal',
        'HYPR(KC_N)': 'New\nGdoc',
        'KC_JITTER': 'Jitter',
        'JITTER': 'Jitter',
        'PSCR': 'Save\nScreen',
        'KC_PSCR': 'Save\nScreen',
        'LALT(KC_HOME)': 'Alt\nHome',
        'KC_DEBUG_SYNC': 'Sync\nDbg',
        'KC_FIRE': 'Fire',
        'KC_DAY': 'Day\nBright',
        'KC_NIGHT': 'Night\nDim',
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
        # Split on underscore and join with newline for multi-part names
        if '_' in rest:
            return '\n'.join(rest.split('_'))
        return rest

    # Handle any remaining codes with underscores
    if '_' in key_code:
        return '\n'.join(key_code.split('_'))
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
    
    # Calculate settings for each line
    line_settings = []
    total_text_height = 0
    
    for line in lines:
        l = len(line)
        if l <= 1:
            font_size = 16
        elif l <= 3:
            font_size = 14
        elif l <= 4:
            font_size = 10
        elif l <= 7:
            font_size = 9
        else:
            font_size = 7
            
        line_height = font_size + 2  # Add some spacing
        line_settings.append((line, font_size, line_height))
        total_text_height += line_height

    # Remove spacing from last line for tighter bounding box
    if line_settings:
        total_text_height -= 2

    # Vertical centering
    # Start drawing from the top of the text block
    # Canvas y is bottom-up
    # Center of key is y + height/2
    # Top of text block is center + total/2
    
    current_y = y + (height + total_text_height) / 2

    for line, font_size, line_height in line_settings:
        c.setFont("Helvetica-Bold", font_size)
        text_width = c.stringWidth(line, "Helvetica-Bold", font_size)
        text_x = x + (width - text_width) / 2
        
        # Draw at baseline. A rough approximation for baseline shift is needed.
        # current_y is the top of this line's space.
        # We descend by font_size mostly.
        draw_y = current_y - font_size
        
        c.drawString(text_x, draw_y, line)
        
        current_y -= line_height


def draw_layer(c, layer_keys, layout_info, layer_num, start_x_arg, start_y, key_size=28):
    """Draw a complete layer on the canvas at specified position."""
    layer_name = LAYER_NAMES.get(layer_num, f"Layer {layer_num}")
    bg_color = LAYER_COLORS.get(layer_num, (0.9, 0.9, 0.9))
    
    # Key dimensions
    key_gap = 1

    # Calculate layout bounds to determine width
    max_x = max(k['x'] for k in layout_info)
    layout_width = (max_x + 1) * (key_size + key_gap)
    
    # Calculate start_x to center on page
    page_width = LETTER[0]
    start_x = (page_width - layout_width) / 2

    # Title
    c.setFont("Helvetica-Bold", 12)
    c.setFillColor(colors.black)
    title = f"{layer_name} (L{layer_num})"
    title_width = c.stringWidth(title, "Helvetica-Bold", 12)
    title_x = (LETTER[0] - title_width) / 2
    c.drawString(title_x, start_y, title)

    # Calculate layout bounds
    max_y = max(k['y'] for k in layout_info)

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


def draw_combos(c, start_y):
    """Draw the combos and tap dance information as two side-by-side tables."""
    c.setFont("Helvetica-Bold", 12)
    c.setFillColor(colors.black)
    title = "COMBOS & SPECIAL ACTIONS"
    title_width = c.stringWidth(title, "Helvetica-Bold", 12)
    title_x = (LETTER[0] - title_width) / 2
    c.drawString(title_x, start_y, title)

    # Split combos into two columns
    mid = (len(COMBOS) + 1) // 2
    left_combos = COMBOS[:mid]
    right_combos = COMBOS[mid:]

    # Column positions for 4-column layout (two tables side by side)
    left_col1_x = 0.5 * inch   # Left table: combo keys
    left_col2_x = 1.7 * inch   # Left table: actions
    right_col1_x = 4.1 * inch  # Right table: combo keys
    right_col2_x = 5.4 * inch  # Right table: actions

    font_size = 14
    curr_y = start_y - 24

    c.setFont("Helvetica", font_size)
    for i in range(max(len(left_combos), len(right_combos))):
        # Draw left table entry
        if i < len(left_combos):
            keys, action = left_combos[i]
            c.drawString(left_col1_x, curr_y, keys)
            c.drawString(left_col2_x, curr_y, f": {action}")

        # Draw right table entry
        if i < len(right_combos):
            keys, action = right_combos[i]
            c.drawString(right_col1_x, curr_y, keys)
            c.drawString(right_col2_x, curr_y, f": {action}")

        curr_y -= 19

def generate_pdf(output_path, keymap_path, info_path):
    """Generate the PDF with all layers."""
    global COMBOS
    
    layers = parse_keymap(keymap_path)
    layout_info = parse_info_json(info_path)
    
    # Extract combos dynamically
    extracted_combos = extract_combos_from_keymap(keymap_path)
    COMBOS = extracted_combos + MANUAL_ACTIONS

    if not layers:
        print("No layers found!")
        return

    page_size = LETTER  # Portrait orientation
    c = canvas.Canvas(output_path, pagesize=page_size)
    c.setTitle("mech keyboard layout")
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

        # Check if there's room for combos on this page (last page with < 3 layers)
        layers_on_this_page = min(3, len(layer_nums) - i)
        if i + 3 >= len(layer_nums) and layers_on_this_page < 3:
            draw_combos(c, current_y - 20)

        c.showPage()

    # If last page was full (3 layers), add combos on a new page
    if len(layer_nums) % 3 == 0:
        current_y = page_height - margin
        draw_combos(c, current_y)
        c.showPage()

    c.save()
    print(f"PDF saved to: {output_path}")

def main():
    base_path = '/home/dcar/projects/mech-keyboard'
    keymap_path = f'{base_path}/keymap/keymap.c'
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
