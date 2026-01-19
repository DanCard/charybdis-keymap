#!/usr/bin/env python3
"""
Common constants, parsing, and key simplification for keyboard layout generation.
Shared between PDF and wallpaper generators.
"""

import json
import re

# Layer colors (RGB tuples normalized 0-1 for PDF, 0-255 for wallpaper)
LAYER_COLORS_NORM = {
    0: (0.9, 0.9, 0.9),      # Light gray (Base)
    1: (0.95, 0.95, 0.95),   # White
    2: (0.7, 1.0, 0.7),      # Green (Arrow)
    3: (1.0, 1.0, 0.7),      # Yellow (Mouse)
    4: (1.0, 0.5, 0.0),      # Orange (One-Hand)
    5: (0.7, 0.7, 1.0),      # Blue (Nav)
    6: (0.95, 0.95, 0.95),   # White (Settings)
}

LAYER_COLORS_255 = {
    0: (200, 200, 210),      # Light gray (Base)
    1: (240, 240, 245),      # White
    2: (130, 220, 130),      # Green (Arrow)
    3: (240, 220, 100),      # Yellow (Mouse)
    4: (255, 150, 0),        # Orange (One-Hand)
    5: (130, 150, 255),      # Blue (Nav)
    6: (240, 240, 245),      # White (Settings)
}

# Key border colors for wallpaper (darker versions)
KEY_BORDER_COLORS = {
    0: (150, 150, 160),
    1: (180, 180, 190),      # Gray for white keys
    2: (80, 160, 80),
    3: (180, 160, 60),
    4: (180, 80, 0),
    5: (80, 100, 180),
    6: (180, 180, 190),
}

LAYER_NAMES = {
    0: "BASE",
    1: "BASE + ARROWS",
    2: "ARROW",
    3: "MOUSE",
    4: "ONE-HAND",
    5: "NUMBER SYMBOLS",
    6: "SETTINGS",
    7: "ARROWS FOR NUMBERS",
}

# Theme colors for special layers
# Layer 6 - Police theme
L6_RED_NORM = (1.0, 0.2, 0.2)
L6_BLUE_NORM = (0.2, 0.4, 1.0)
L6_RED_255 = (255, 50, 50)
L6_BLUE_255 = (50, 100, 255)
L6_RED_BORDER = (180, 30, 30)
L6_BLUE_BORDER = (30, 60, 180)

# Layer 1 - Flash theme (pink/cyan)
L1_PINK_NORM = (1.0, 0.4, 0.7)
L1_CYAN_NORM = (0.0, 0.8, 0.6)
L1_PINK_255 = (255, 100, 180)
L1_CYAN_255 = (0, 200, 150)
L1_PINK_BORDER = (180, 70, 130)
L1_CYAN_BORDER = (0, 140, 105)

# Layer 7 - Rainbow theme
L7_RAINBOW_NORM = [
    (1.0, 0.2, 0.2),   # Red
    (1.0, 0.6, 0.0),   # Orange
    (0.0, 0.8, 0.4),   # Green
    (0.2, 0.4, 1.0),   # Blue
]
L7_RAINBOW_255 = [
    (255, 50, 50),     # Red
    (255, 150, 0),     # Orange
    (0, 200, 100),     # Green
    (50, 100, 255),    # Blue
]
L7_RAINBOW_BORDER = [
    (180, 30, 30),     # Dark red
    (180, 100, 0),     # Dark orange
    (0, 140, 70),      # Dark green
    (30, 60, 180),     # Dark blue
]

# Key label simplifications for standard QMK keycodes
KEY_LABELS = {
    "KC_ESC": "Escape",
    "KC_TAB": "Tab",
    "KC_LSFT": "Shift",
    "KC_RSFT": "Shift",
    "KC_LCTL": "Ctrl",
    "KC_RCTL": "Ctrl",
    "KC_LALT": "Alt",
    "KC_RALT": "Alt",
    "KC_LGUI": "Win",
    "KC_RGUI": "Win",
    "KC_SPC": "Space",
    "KC_BSPC": "Back\nSpace",
    "KC_ENT": "Enter",
    "KC_DEL": "Delete",
    "KC_MINS": "-",
    "KC_EQL": "=",
    "KC_LBRC": "[",
    "KC_RBRC": "]",
    "KC_BSLS": "\\",
    "KC_SCLN": ";",
    "KC_QUOT": "'",
    "KC_GRV": "`",
    "KC_COMM": ",",
    "KC_DOT": ".",
    "KC_SLSH": "/",
    "KC_CAPS": "Caps\nLock",
    "KC_HOME": "Home",
    "KC_END": "End",
    "KC_PGUP": "Page\nUp",
    "KC_PGDN": "Page\nDown",
    "KC_UP": "Up",
    "KC_DOWN": "Down",
    "KC_LEFT": "Left",
    "KC_RGHT": "Right",
    "KC_TRNS": "",
    "KC_NO": "",
    "KC_MPLY": "Play",
    "KC_MNXT": "Next",
    "KC_MPRV": "Prev",
    "KC_VOLU": "Vol\nUp",
    "KC_VOLD": "Vol\nDown",
    "KC_MUTE": "Mute",
    "KC_PPLS": "Num +",
    "KC_PMNS": "Num -",
    "KC_PAST": "Num *",
    "KC_PSLS": "Num /",
    "KC_PEQL": "Num =",
    "KC_PDOT": "Num .",
    "KC_P0": "Num 0",
    "KC_P1": "Num 1",
    "KC_P2": "Num 2",
    "KC_P3": "Num 3",
    "KC_P4": "Num 4",
    "KC_P5": "Num 5",
    "KC_P6": "Num 6",
    "KC_P7": "Num 7",
    "KC_P8": "Num 8",
    "KC_P9": "Num 9",
    "QK_BOOT": "BOOT",
    "QK_CLEAR_EEPROM": "EE\nCLR",
    "QK_GESC": "Esc\n~",
    "MS_BTN1": "Left\nClick",
    "MS_BTN2": "Right\nClick",
    "MS_BTN3": "Middle\nClick",
    "MS_UP": "Mouse\nUp",
    "MS_DOWN": "Mouse\nDown",
    "MS_LEFT": "Mouse\nLeft",
    "MS_RGHT": "Mouse\nRight",
    "SNIPING": "Snipe",
    "DRGSCRL": "Scroll",
    "DPI_MOD": "DPI+",
    "DPI_RMOD": "DPI-",
    "S_D_MOD": "S-DPI",
    "RM_NEXT": "rgb +",
    "RM_PREV": "rgb -",
    "RM_HUEU": "Hue +",
    "RM_HUED": "Hue -",
    "RM_SATU": "Sat +",
    "RM_SATD": "Sat -",
    "RM_VALU": "Val +",
    "RM_VALD": "Val -",
}

# Custom keycodes specific to this keymap
CUSTOM_KEY_MAP = {
    "KC_X": "X",
    "KC_P_TO0": "P\nL0",
    "KC_SLSH_TO0": "/\nL0",
    "KC_X_TG2": "X\nTG2",
    "KC_V_TG5": "V\nTG5",
    "KC_Q_L4": "Q\nL4",
    "KC_Q_Z": "Q\nZ",
    "KC_L1_L3": "L1\nL3",
    "KC_R_L2": "L2\nTgl",
    "KC_ENT_MO4": "Enter\nL4",
    "KC_ENT_EXIT": "Enter\nExit",
    "KC_SPC_EXIT": "Space\nExit",
    "KC_BSPC_EXIT": "Back\nExit",
    "KC_EXIT": "Exit",
    "KC_TURBO": "Temp\nTurbo",
    "KC_RAINBOW": "Rain\nbow",
    "KC_REACTIVE": "Reac\ntive",
    "KC_MOUSE_LOCK": "Layer\nLock",
    "KC_MS_FAST_UP": "Mouse\nUp+",
    "KC_MS_FAST_DOWN": "Mouse\nDown+",
    "KC_MS_FAST_LEFT": "Mouse\nLeft+",
    "KC_MS_FAST_RIGHT": "Mouse\nRight+",
    "KC_MS_DIAG_UL": "Up\nLeft",
    "KC_MS_DIAG_UR": "Up\nRight",
    "KC_MS_DIAG_DL": "Down\nLeft",
    "KC_MS_DIAG_DR": "Down\nRight",
    "KC_SCR_MODE": "Scr\nMod",
    "KC_1_L1": "1\nL1",
    "KC_2_L2": "2\nL2",
    "KC_3_L3": "3\nL3",
    "KC_4_L4": "4\nL4",
    "KC_5_L5": "5\nL5",
    "KC_L3_EXT_TO4": "Exit\nL4",
    "KC_L3_EXT_TO2": "Exit\nL2",
    "KC_L3_EXT_TO1": "Exit\nL1",
    "RM_TOGG": "RGB\nToggle",
    "KC_JELLY": "Jelly",
    "KC_SPIRAL": "Spiral",
    "KC_CHEVRON": "Chev\nron",
    "KC_LR_TOGGLE": "LR\nToggle",
    "KC_FLASH": "Flash",
    "KC_FLASHLIGHT": "Light",
    "KC_SNIPE": "Snipe",
    "KC_FAST": "Fast",
    "KC_SCR_LOCK": "Scroll\nLock",
    "KC_RGB_AUTO": "RGB\nAuto",
    "KC_PLUS_COLON": "+ \n:",
    "RM_HUEU": "Hue\n+",
    "RM_HUED": "Hue\n-",
    "RM_SATU": "Sat\n+",
    "RM_SATD": "Sat\n-",
    "RM_VALU": "Brt\n+",
    "RM_VALD": "Brt\n-",
    "KC_MINS_TO0": "- \nExit",
    "KC_0_L1": "0\nL1",
    "KC_9_L2": "9\nL2",
    "KC_8_L3": "8\nL3",
    "KC_P_FRAC": "Pixel\nFractal",
    "KC_PINWHEEL": "Pin\nwheel",
    "KC_7_TO0": "7\nExit",
    "KC_6_TO0": "6\nExit",
    "KC_0_TO0": "0\nL0",
    "KC_2_LEFT": "Left\n2",
    "KC_3_UP": "Up\n3",
    "KC_4_DOWN": "Down\n4",
    "KC_5_RIGHT": "Right\n5",
    "KC_LEFT_2": "Left\n2",
    "KC_UP_3": "Up\n3",
    "KC_DOWN_4": "Down\n4",
    "KC_RIGHT_5": "Right\n5",
    "KC_6_TO6": "Left\n6",
    "KC_LEFT_6": "Left\n6",
    "KC_7_UP": "Up\n7",
    "KC_UP_7": "Up\n7",
    "KC_8_DOWN": "Down\n8",
    "KC_DOWN_8": "Down\n8",
    "KC_9_RIGHT": "Right\n9",
    "KC_RIGHT_9": "Right\n9",
    "KC_SPC_L2": "Space\nL2",
    "KC_SPC_L4": "Space\nL4",
    "KC_ENT_L2": "Enter\nL2",
    "KC_ENT_L4": "Enter\nL4",
    "KC_PMNS_L4": "Num -\nL4",
    "KC_F12_EXIT": "F12\nExit",
    "KC_MS_TMO_INC": "Time\nout +",
    "KC_MS_TMO_DEC": "Time\nout -",
    "HYPR(KC_N)": "New\nGdoc",
    "KC_JITTER": "Jitter",
    "JITTER": "Jitter",
    "PSCR": "Save\nScreen",
    "KC_PSCR": "Save\nScreen",
    "LALT(KC_HOME)": "Alt\nHome",
    "KC_DUMP_LOG": "Dump\nLog",
    "KC_FILT_LOG": "Filt\nSync",
    "KC_FIRE": "Fire",
    "KC_DAY": "Day\nBright",
    "KC_NIGHT": "Night\nDim",
    "KC_QUES_SLSH": "?\n/",
}

# Shift key mappings
SHIFT_MAP = {
    "1": "!", "2": "@", "3": "#", "4": "$", "5": "%",
    "6": "^", "7": "&", "8": "*", "9": "(", "0": ")",
    "MINS": "_", "EQL": "+", "GRV": "~",
    "LBRC": "{", "RBRC": "}", "BSLS": "|",
    "SCLN": ":", "QUOT": '"', "COMM": "<", "DOT": ">", "SLSH": "?",
}

# Combo manual actions
MANUAL_ACTIONS = [
    ("Thumb (Tap)", "Top Function"),
    ("Thumb (Hold)", "Mode Change"),
    ("Z (Tap)", "Z"),
    ("Z (Hold)", "Layer 4"),
    ("Snipe Active", "Right Top Black"),
    ("Snipe Active", "R-Col Rainbow"),
]


def readable_key(k):
    """Convert a single keycode to a readable short string for combo listing."""
    k = k.strip()
    if k.startswith("KC_"):
        k = k[3:]

    # Handle shifted keys like S(KC_V)
    if "S(" in k:
        match = re.search(r"S\((?:KC_)?(\w+)\)", k)
        if match:
            return f"Shift+{match.group(1)}"

    # Handle Ctrl+Shift like C(S(KC_V))
    if "C(S(" in k:
        match = re.search(r"C\(S\((?:KC_)?(\w+)\)\)", k)
        if match:
            return f"Ctrl+Shift+{match.group(1)}"

    # Handle Alt+key like LALT(KC_HOME)
    if "LALT(" in k:
        match = re.search(r"LALT\((?:KC_)?(\w+)\)", k)
        if match:
            return f"Alt+{match.group(1).capitalize()}"

    # Handle Tap Dance
    if "TD(" in k:
        return "Z"  # Special case for TD(TD_Z_LAYER)

    # Dictionary for common abbreviations
    lookup = {
        "LSFT": "LShf", "RSFT": "RShf",
        "LCTL": "Ctrl", "RCTL": "Ctrl",
        "LALT": "Alt", "RALT": "Alt",
        "LGUI": "Win", "RGUI": "Win",
        "BSPC": "Bksp", "DEL": "Del",
        "PGUP": "PgUp", "PGDN": "PgDn",
        "MINS": "-", "EQL": "=",
        "LBRC": "[", "RBRC": "]", "BSLS": "\\",
        "SCLN": ";", "QUOT": "'", "GRV": "`",
        "COMM": ",", "DOT": ".", "SLSH": "/",
        "SPC": "Space", "ENT": "Enter",
    }
    return lookup.get(k, k)


def simplify_key(key_code, layer_num=None):
    """Convert QMK keycode to readable label."""
    # Tap Dance Z-Layer Handling
    if "TD_Z_LAYER" in key_code:
        if layer_num == 0:
            return "Z\nL4"
        if layer_num == 1:
            return "Z\nL4"
        if layer_num == 2:
            return "Home\nL4"
        if layer_num == 3:
            return "Z\nL4"
        if layer_num == 4:
            return "/\nL0"
        return "Z\nL4"

    # Special handling for Layer 0 and Layer 6 Long Press
    if layer_num == 0 or layer_num == 6:
        if key_code == "KC_1_L1":
            return "1\nL1"
        if key_code == "KC_2_L2":
            return "2\nL2"
        if key_code == "KC_3_L3":
            return "3\nL3"
        if key_code == "KC_4_L4":
            return "4\nL4"
        if key_code == "KC_5_L5":
            return "5\nL5"
        if key_code == "KC_0_TO0":
            return "0\nL0"

    # Special handling for Layer 1
    # Note: KC_1_L1 hold on L1 goes to L0 (toggle off), but KC_2/3/4 hold
    # on L0 or L1 goes to L2/3/4 respectively (tap_hold.c checks layer_state <= 1)
    if layer_num == 1:
        if key_code == "KC_1_L1":
            return "1\nL0"
        if key_code == "KC_2_L2":
            return "2\nL2"
        if key_code == "KC_3_L3":
            return "3\nL3"
        if key_code == "KC_4_L4":
            return "4\nL4"

    # Special handling for Layer 2
    if layer_num == 2:
        if "KC_X_TG2" == key_code:
            return "Page\nUp"

    if layer_num is not None and layer_num > 0:
        if key_code == "KC_L1_L3":
            return "Exit\nL1"

    # Special handling for other layer long press returns
    if layer_num is not None and layer_num > 1:
        if key_code == "KC_1_L1":
            label = "F1" if layer_num == 2 else ("Rainb" if layer_num == 3 else "0")
            return f"{label}\nL0"
        if key_code == "KC_2_L2":
            label = "F2" if layer_num == 2 else ("Next" if layer_num == 3 else "9")
            return f"{label}\nL0"
        if key_code == "KC_3_L3":
            label = "F3" if layer_num == 2 else "3"
            return f"{label}\nExit"
        if key_code == "KC_4_L4":
            label = "F4" if layer_num == 2 else "4"
            return f"{label}\nExit"

    if key_code in KEY_LABELS:
        return KEY_LABELS[key_code]

    # Handle S(KC_X) for shifted keys
    shift_match = re.match(r"S\(KC_(\w+)\)", key_code)
    if shift_match:
        char = shift_match.group(1)
        return SHIFT_MAP.get(char, f"S-{char}")

    # Handle LT(layer, key)
    lt_match = re.match(r"LT\((\d+),\s*KC_(\w+)\)", key_code)
    if lt_match:
        layer = lt_match.group(1)
        key = lt_match.group(2)
        inner_key_code = f"KC_{key}"
        label = KEY_LABELS.get(inner_key_code, key)
        return f"{label}\nL{layer}"

    # Handle TD(...)
    td_match = re.match(r"TD\((\w+)\)", key_code)
    if td_match:
        return "TD"

    # Handle custom keycodes
    if key_code in CUSTOM_KEY_MAP:
        return CUSTOM_KEY_MAP[key_code]

    # Strip KC_ prefix for simple keys
    if key_code.startswith("KC_"):
        rest = key_code[3:]
        if len(rest) == 1:
            return rest
        if rest.startswith("F") and rest[1:].isdigit():
            return rest
        # Split on underscore and join with newline for multi-part names
        if "_" in rest:
            return "\n".join(rest.split("_"))
        return rest

    # Handle any remaining codes with underscores
    if "_" in key_code:
        return "\n".join(key_code.split("_"))
    return key_code[:10]


def parse_keymap(file_path):
    """Parse keymap.c to extract layer definitions."""
    with open(file_path, "r") as f:
        content = f.read()

    keymaps_match = re.search(
        r"const uint16_t PROGMEM keymaps\[\]\[MATRIX_ROWS\]\[MATRIX_COLS\] = \{(.*?)\};",
        content,
        re.DOTALL,
    )
    if not keymaps_match:
        return {}

    keymaps_str = keymaps_match.group(1)
    layers = {}
    search_start = 0

    while True:
        match = re.search(r"\[(\d+)\]\s*=\s*LAYOUT\(", keymaps_str[search_start:])
        if not match:
            break

        layer_num = int(match.group(1))
        start_index = search_start + match.end()

        paren_depth = 1
        current_index = start_index
        layout_content = ""

        while current_index < len(keymaps_str) and paren_depth > 0:
            char = keymaps_str[current_index]
            if char == "(":
                paren_depth += 1
            elif char == ")":
                paren_depth -= 1
            if paren_depth > 0:
                layout_content += char
            current_index += 1

        layout_str = re.sub(r"//.*", "", layout_content)
        layout_str = re.sub(r"/\*.*?\*/", "", layout_str, flags=re.DOTALL)
        layout_str = " ".join(layout_str.split())

        keys = []
        current_key = ""
        depth = 0
        for char in layout_str:
            if char == "(":
                depth += 1
                current_key += char
            elif char == ")":
                depth -= 1
                current_key += char
            elif char == "," and depth == 0:
                keys.append(current_key.strip())
                current_key = ""
            else:
                current_key += char
        if current_key:
            keys.append(current_key.strip())

        layers[layer_num] = keys
        search_start = current_index

    return layers


def parse_info_json(file_path, thumb_shift_left=2.0, thumb_shift_right=1.0, right_main_shift=4.0):
    """Parse info.json to get physical key layout positions."""
    with open(file_path, "r") as f:
        data = json.load(f)
    layout = data["layouts"]["LAYOUT"]["layout"]

    # Adjust layout to minimize whitespace and center thumbs
    for key in layout:
        # Right Main (rows 0-3, x >= 11) -> Shift Left
        if key["y"] < 4 and key["x"] >= 11:
            key["x"] -= right_main_shift

        # Left Thumbs (y >= 4, x < 9)
        elif key["y"] >= 4 and key["x"] < 9:
            key["x"] -= thumb_shift_left

        # Right Thumbs (y >= 4, x >= 9)
        elif key["y"] >= 4 and key["x"] >= 9:
            key["x"] -= thumb_shift_right

    return layout


def extract_combos_from_keymap(file_path):
    """Parse keymap.c to find all combos defined in key_combos array."""
    try:
        with open(file_path, "r") as f:
            content = f.read()

        # 1. Parse PROGMEM combo definitions
        combo_defs = {}
        matches = re.finditer(
            r"const uint16_t PROGMEM (\w+)\[\]\s*=\s*\{([^}]+)\};", content
        )
        for match in matches:
            name = match.group(1)
            keys_str = match.group(2)
            keys = [
                k.strip()
                for k in keys_str.split(",")
                if "COMBO_END" not in k and k.strip()
            ]
            combo_defs[name] = keys

        # 2. Parse combo_t array
        found_combos = []
        match_block = re.search(
            r"combo_t\s+key_combos\[\]\s*=\s*\{([^;]+)\};", content, re.DOTALL
        )
        if match_block:
            block = match_block.group(1)
            idx = 0
            while True:
                start = block.find("COMBO(", idx)
                if start == -1:
                    break

                idx = start + 6
                depth = 1
                args_start = idx
                args_end = -1

                while idx < len(block) and depth > 0:
                    if block[idx] == "(":
                        depth += 1
                    elif block[idx] == ")":
                        depth -= 1
                        if depth == 0:
                            args_end = idx
                    idx += 1

                if args_end != -1:
                    args_str = block[args_start:args_end]
                    parts = args_str.split(",", 1)
                    if len(parts) == 2:
                        name = parts[0].strip()
                        action = parts[1].strip()

                        if name in combo_defs:
                            readable_keys = " + ".join(
                                [readable_key(k) for k in combo_defs[name]]
                            )
                            readable_action = readable_key(action)

                            if "Paste" in readable_action:
                                readable_action = "Ctrl+Shift+V"
                            elif (
                                ("V" in readable_action or "v" in readable_action)
                                and ("Shift" in readable_action or "S(" in action)
                                and ("Ctrl" in readable_action or "C(" in action)
                            ):
                                readable_action = "Ctrl+Shift+V"

                            found_combos.append((readable_keys, readable_action))

        return found_combos

    except Exception as e:
        print(f"Error extracting combos: {e}")
        return []


def get_themed_colors_for_key(layer_num, x, y, use_255=False):
    """
    Get the background and border colors for a key based on layer themes.
    Returns (bg_color, border_color) or (None, None) if no special theme applies.
    """
    # Layer 1: Arrow keys police theme (pink/cyan)
    # Right side original x values: 7,8,9,10,11 - only x>=11 gets shifted to ~6.5
    # So rightmost is either x>=11 (unshifted) or 6<x<7 (shifted)
    if layer_num == 1:
        # Left side arrows (pink): row 3 leftmost 2 keys (LEFT, RIGHT at x<2)
        # Thumb arrows: row 4-5 specific positions for UP, DOWN
        is_left_arrow = (y == 3 and x < 2) or (y == 4 and 2 <= x <= 3) or (y == 5 and 2 <= x <= 4)

        # Right side arrows (cyan): rightmost column(s) only
        # Rightmost key: x >= 11 (unshifted) or 6 < x < 8 (shifted from x=11)
        # Row 0-1: only rightmost (UP, DOWN)
        # Row 2: rightmost 2 (LEFT at x=10, RIGHT at x=11 or shifted)
        is_rightmost = (x >= 11) or (6 < x < 8)
        is_second_rightmost = (x >= 10) or (5 < x < 6.5)

        is_right_arrow = ((y == 0 or y == 1) and is_rightmost) or (y == 2 and is_second_rightmost)

        if is_left_arrow:
            if use_255:
                return L1_PINK_255, L1_PINK_BORDER
            return L1_PINK_NORM, None
        elif is_right_arrow:
            if use_255:
                return L1_CYAN_255, L1_CYAN_BORDER
            return L1_CYAN_NORM, None
        return None, None

    if y != 0:  # Only top row gets themed colors for other layers
        return None, None

    if layer_num == 6:  # Police theme
        if x <= 1:
            if use_255:
                return L6_RED_255, L6_RED_BORDER
            return L6_RED_NORM, None
        elif x >= 10:
            if use_255:
                return L6_BLUE_255, L6_BLUE_BORDER
            return L6_BLUE_NORM, None

    elif layer_num == 7:  # Rainbow theme
        if x == 0:
            if use_255:
                return L7_RAINBOW_255[0], L7_RAINBOW_BORDER[0]
            return L7_RAINBOW_NORM[0], None
        elif x == 1:
            if use_255:
                return L7_RAINBOW_255[1], L7_RAINBOW_BORDER[1]
            return L7_RAINBOW_NORM[1], None
        elif x == 10:
            if use_255:
                return L7_RAINBOW_255[2], L7_RAINBOW_BORDER[2]
            return L7_RAINBOW_NORM[2], None
        elif x == 11:
            if use_255:
                return L7_RAINBOW_255[3], L7_RAINBOW_BORDER[3]
            return L7_RAINBOW_NORM[3], None

    return None, None
