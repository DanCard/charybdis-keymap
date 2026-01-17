#!/usr/bin/env python3
"""
Generate a desktop wallpaper of the Charybdis 4x6 keyboard layout.
Requires: pip install pillow
"""

import json
import re
import os
from PIL import Image, ImageDraw, ImageFont

# Layer colors matching the RGB settings in keymap.c
LAYER_COLORS = {
    0: (200, 200, 210),  # Light gray (Base)
    1: (150, 230, 230),  # Cyan (Original)
    2: (130, 220, 130),  # Green (Arrow)
    3: (240, 220, 100),  # Yellow (Mouse)
    4: (255, 150, 0),  # Orange (One-Hand)
    5: (130, 150, 255),  # Blue (Nav)
    6: (255, 110, 150),  # Hot Pink (Settings)
}

LAYER_NAMES = {
    0: "BASE",
    1: "ORIGINAL",
    2: "ARROW",
    3: "MOUSE",
    4: "ONE-HAND",
    5: "NUMBER SYMBOLS",
    6: "SETTINGS",
}

# Key border colors (darker versions)
KEY_BORDER_COLORS = {
    0: (150, 150, 160),
    1: (100, 170, 170),  # Cyan (Original)
    2: (80, 160, 80),
    3: (180, 160, 60),
    4: (180, 80, 0),
    5: (80, 100, 180),  # Blue (Nav)
    6: (180, 60, 90),  # Hot Pink (Settings)
}

# Key label simplifications
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
    "QK_GESC": "Esc\n~\n`",
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

COMBOS = []
MANUAL_ACTIONS = [
    ("Thumb (Tap)", "Top Function"),
    ("Thumb (Hold)", "Mode Change"),
    ("Z (Tap)", "Z"),
    ("Z (Hold)", "Layer 4"),
    ("Snipe Active", "Right Top Black"),
    ("Snipe Active", "R-Col Rainbow"),
]

# Wallpaper settings
WALLPAPER_WIDTH = 3840
WALLPAPER_HEIGHT = 2160
BACKGROUND_COLOR = (26, 26, 46)  # Dark blue-gray


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

    # Handle Tap Dance
    if "TD(" in k:
        return "Z"  # Special case for TD(TD_Z_LAYER)

    # Dictionary for common abbreviations
    lookup = {
        "LSFT": "LShift",
        "RSFT": "RShift",
        "LCTL": "Ctrl",
        "RCTL": "Ctrl",
        "LALT": "Alt",
        "RALT": "Alt",
        "LGUI": "Win",
        "RGUI": "Win",
        "BSPC": "Bksp",
        "DEL": "Del",
        "PGUP": "PgUp",
        "PGDN": "PgDn",
        "MINS": "-",
        "EQL": "=",
        "LBRC": "[",
        "RBRC": "]",
        "BSLS": "\\",
        "SCLN": ";",
        "QUOT": "'",
        "GRV": "`",
        "COMM": ",",
        "DOT": ".",
        "SLSH": "/",
        "SPC": "Space",
        "ENT": "Enter",
    }
    return lookup.get(k, k)


def extract_combos_from_keymap(file_path):
    """Parse keymap.c to find all combos defined in key_combos array."""
    try:
        with open(file_path, "r") as f:
            content = f.read()

        # 1. Parse PROGMEM combo definitions: const uint16_t PROGMEM name[] = {K1, K2, COMBO_END};
        combo_defs = {}
        # Regex handles multiline definitions if needed, assuming brace closure
        matches = re.finditer(
            r"const uint16_t PROGMEM (\w+)\[\]\s*=\s*\{([^}]+)\};", content
        )
        for match in matches:
            name = match.group(1)
            keys_str = match.group(2)
            # Filter out COMBO_END and whitespace
            keys = [
                k.strip()
                for k in keys_str.split(",")
                if "COMBO_END" not in k and k.strip()
            ]
            combo_defs[name] = keys

        # 2. Parse combo_t array: combo_t key_combos[] = { COMBO(name, action), ... };
        found_combos = []

        # Find the block
        match_block = re.search(
            r"combo_t\s+key_combos\[\]\s*=\s*\{([^;]+)\};", content, re.DOTALL
        )
        if match_block:
            block = match_block.group(1)

            # Simple parser for COMBO(name, action)
            idx = 0
            while True:
                # Find next COMBO(
                start = block.find("COMBO(", idx)
                if start == -1:
                    break

                idx = start + 6  # Skip 'COMBO('
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
                    # args_str should be "name, action"
                    parts = args_str.split(",", 1)
                    if len(parts) == 2:
                        name = parts[0].strip()
                        action = parts[1].strip()

                        if name in combo_defs:
                            # Convert keys to readable string
                            readable_keys = " + ".join(
                                [readable_key(k) for k in combo_defs[name]]
                            )

                            # Convert action to readable string
                            readable_action = readable_key(action)

                            # Heuristic for Ctrl+Shift+V
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


def get_font(size, bold=False):
    """Get a font, falling back to default if custom fonts unavailable."""
    font_paths = [
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
    ]

    for path in font_paths:
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size)
            except:
                continue

    return ImageFont.load_default()


def draw_rounded_rect(draw, xy, radius, fill, outline=None, width=1):
    """Draw a rounded rectangle."""
    x1, y1, x2, y2 = xy

    # Draw main rectangle
    draw.rectangle([x1 + radius, y1, x2 - radius, y2], fill=fill)
    draw.rectangle([x1, y1 + radius, x2, y2 - radius], fill=fill)

    # Draw corners
    draw.pieslice([x1, y1, x1 + radius * 2, y1 + radius * 2], 180, 270, fill=fill)
    draw.pieslice([x2 - radius * 2, y1, x2, y1 + radius * 2], 270, 360, fill=fill)
    draw.pieslice([x1, y2 - radius * 2, x1 + radius * 2, y2], 90, 180, fill=fill)
    draw.pieslice([x2 - radius * 2, y2 - radius * 2, x2, y2], 0, 90, fill=fill)

    # Draw outline if specified
    if outline:
        draw.arc(
            [x1, y1, x1 + radius * 2, y1 + radius * 2],
            180,
            270,
            fill=outline,
            width=width,
        )
        draw.arc(
            [x2 - radius * 2, y1, x2, y1 + radius * 2],
            270,
            360,
            fill=outline,
            width=width,
        )
        draw.arc(
            [x1, y2 - radius * 2, x1 + radius * 2, y2],
            90,
            180,
            fill=outline,
            width=width,
        )
        draw.arc(
            [x2 - radius * 2, y2 - radius * 2, x2, y2], 0, 90, fill=outline, width=width
        )
        draw.line([x1 + radius, y1, x2 - radius, y1], fill=outline, width=width)
        draw.line([x1 + radius, y2, x2 - radius, y2], fill=outline, width=width)
        draw.line([x1, y1 + radius, x1, y2 - radius], fill=outline, width=width)
        draw.line([x2, y1 + radius, x2, y2 - radius], fill=outline, width=width)


def draw_key(draw, x, y, width, height, label, bg_color, border_color):
    """Draw a single key with label."""
    radius = 6

    # Draw key shadow
    shadow_offset = 3
    draw_rounded_rect(
        draw,
        (
            x + shadow_offset,
            y + shadow_offset,
            x + width + shadow_offset,
            y + height + shadow_offset,
        ),
        radius,
        fill=(15, 15, 30),
    )

    # Draw key background
    draw_rounded_rect(
        draw,
        (x, y, x + width, y + height),
        radius,
        fill=bg_color,
        outline=border_color,
        width=2,
    )

    # Draw label
    lines = label.split("\n")
    text_color = (30, 30, 40)  # Dark text for contrast

    line_configs = []
    total_height = 0

    for line in lines:
        l = len(line)
        if l <= 1:
            font_size = 46
        elif l <= 3:
            font_size = 38
        elif l <= 4:
            font_size = 26
        elif l <= 6:
            font_size = 22
        else:
            font_size = 18

        # Increase cap for 3+ line labels
        if len(lines) >= 3:
            font_size = min(font_size, 32)

        font = get_font(font_size, bold=True)

        # Calculate height
        bbox = draw.textbbox((0, 0), line, font=font)
        # Height is bottom - top
        h = bbox[3] - bbox[1]
        # Add some leading/spacing
        h += 14

        line_configs.append((line, font, h))
        total_height += h

    # Remove spacing from last line
    if line_configs:
        total_height -= 14

    # Start position for vertical centering
    current_y = y + (height - total_height) // 2

    for line, font, h in line_configs:
        bbox = draw.textbbox((0, 0), line, font=font)
        text_width = bbox[2] - bbox[0]
        text_x = x + (width - text_width) // 2

        draw.text((text_x, current_y), line, fill=text_color, font=font)
        current_y += h


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
    # Special handling for Layer 2
    if layer_num == 2:
        if "KC_X_TG2" == key_code:
            return "Page\nUp"

    if layer_num is not None and layer_num > 0:
        if key_code == "KC_L_L1":
            return "Exit\nL3"

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
        shift_map = {
            "1": "!",
            "2": "@",
            "3": "#",
            "4": "$",
            "5": "%",
            "6": "^",
            "7": "&",
            "8": "*",
            "9": "(",
            "0": ")",
            "MINS": "_",
            "EQL": "+",
            "GRV": "~",
            "LBRC": "{",
            "RBRC": "}",
            "BSLS": "|",
            "SCLN": ":",
            "QUOT": '"',
            "COMM": "<",
            "DOT": ">",
            "SLSH": "?",
        }
        return shift_map.get(char, f"S-{char}")

    # Handle LT(layer, key)
    lt_match = re.match(r"LT\((\d+),\s*KC_(\w+)\)", key_code)
    if lt_match:
        layer = lt_match.group(1)
        key = lt_match.group(2)
        # Simplify the inner key if possible
        inner_key_code = f"KC_{key}"
        label = KEY_LABELS.get(inner_key_code, key)
        return f"{label}\nL{layer}"

    # Handle TD(...)
    td_match = re.match(r"TD\((\w+)\)", key_code)
    if td_match:
        return "TD"

    # Handle custom keycodes
    custom_map = {
        "KC_X": "X",
        "KC_P_TO0": "P\nL0",
        "KC_SLSH_TO0": "/\nL0",
        "KC_X_TG2": "X\nTG2",
        "KC_V_TG5": "V\nTG5",
        "KC_Q_L4": "Q\nL4",
        "KC_L_L1": "L3\nL1",
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
        "KC_RAINBOW": "Rain\nbow",
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
        "KC_P_FRAC": "Pixel\nFrac",
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
        "KC_P_FRAC": "Pixel\nFractal",
        "HYPR(KC_N)": "New\nGdoc",
        "KC_JITTER": "Jitter",
        "JITTER": "Jitter",
        "PSCR": "Save\nScreen",
        "KC_PSCR": "Save\nScreen",
        "LALT(KC_HOME)": "Alt\nHome",
        "KC_DEBUG_SYNC": "Sync\nDbg",
        "KC_FIRE": "Fire",
        "KC_DAY": "Day\nBright",
        "KC_NIGHT": "Night\nDim",
    }
    if key_code in custom_map:
        return custom_map[key_code]

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


def parse_info_json(file_path):
    with open(file_path, "r") as f:
        data = json.load(f)
    layout = data["layouts"]["LAYOUT"]["layout"]

    # Adjust layout to minimize whitespace and center thumbs
    for key in layout:
        # Right Main (rows 0-3, x >= 11) -> Shift Left by 4.5
        if key["y"] < 4 and key["x"] >= 11:
            key["x"] -= 4.5

        # Left Thumbs (y >= 4, x < 9) -> Shift Left by 2 (Center under Left Main)
        elif key["y"] >= 4 and key["x"] < 9:
            key["x"] -= 2.0

        # Right Thumbs (y >= 4, x >= 9) -> Shift Left by 1.5 (Center under Right Main)
        elif key["y"] >= 4 and key["x"] >= 9:
            key["x"] -= 1.5

    return layout


def draw_layer(
    img,
    draw,
    layer_keys,
    layout_info,
    layer_num,
    start_x,
    start_y,
    key_width=88,
    key_height=88,
):
    """Draw a complete layer at the specified position."""
    layer_name = LAYER_NAMES.get(layer_num, f"Layer {layer_num}")
    bg_color = LAYER_COLORS.get(layer_num, (200, 200, 200))
    border_color = KEY_BORDER_COLORS.get(layer_num, (150, 150, 150))

    key_gap = 4

    # Calculate layout bounds
    max_x = max(k["x"] for k in layout_info)
    max_y = max(k["y"] for k in layout_info)

    layout_width = int((max_x + 1) * (key_width + key_gap))

    # Draw title
    title_font = get_font(44, bold=True)
    title = f"{layer_name} (L{layer_num})"
    bbox = draw.textbbox((0, 0), title, font=title_font)
    title_width = bbox[2] - bbox[0]
    title_x = start_x + (layout_width - title_width) // 2
    draw.text((title_x, start_y), title, fill=(180, 180, 200), font=title_font)

    # Offset for keys below title
    keys_start_y = start_y + 55

    # Draw each key
    for i, key_code in enumerate(layer_keys):
        if i >= len(layout_info):
            break

        info = layout_info[i]
        kx = start_x + int(info["x"] * (key_width + key_gap))
        ky = keys_start_y + int(info["y"] * (key_height + key_gap))

        label = simplify_key(key_code, layer_num)
        if label:  # Only draw if there's a label
            draw_key(draw, kx, ky, key_width, key_height, label, bg_color, border_color)

    return (max_y + 1) * (key_height + key_gap) + 55


def draw_combos(draw, start_x, start_y, width):
    """Draw the combos section in 2 columns."""
    current_y = start_y

    title_font = get_font(32, bold=True)
    text_font = get_font(22)

    # Title
    title = "COMBOS & SPECIAL ACTIONS"
    bbox = draw.textbbox((0, 0), title, font=title_font)
    title_width = bbox[2] - bbox[0]
    draw.text(
        (start_x + (width - title_width) // 2, current_y),
        title,
        fill=(180, 180, 200),
        font=title_font,
    )

    current_y += 50  # Spacing after title

    # Split combos into two columns
    n = len(COMBOS)
    half = (n + 1) // 2
    col1_combos = COMBOS[:half]
    col2_combos = COMBOS[half:]

    # Calculate column positions
    col_content_width = width // 2
    combo_start_x = start_x

    line_height = 34

    max_rows = max(len(col1_combos), len(col2_combos))
    for i in range(max_rows):
        if i < len(col1_combos):
            keys, action = col1_combos[i]
            draw.text(
                (combo_start_x + 20, current_y),
                f"{keys}",
                fill=(200, 200, 220),
                font=text_font,
            )
            draw.text(
                (combo_start_x + 200, current_y),
                f": {action}",
                fill=(160, 160, 180),
                font=text_font,
            )

        if i < len(col2_combos):
            keys, action = col2_combos[i]
            draw.text(
                (combo_start_x + col_content_width + 20, current_y),
                f"{keys}",
                fill=(200, 200, 220),
                font=text_font,
            )
            draw.text(
                (combo_start_x + col_content_width + 200, current_y),
                f": {action}",
                fill=(160, 160, 180),
                font=text_font,
            )

        current_y += line_height


def generate_wallpaper(output_path, keymap_path, info_path):
    """Generate the wallpaper with all layers in a 2x3 grid."""
    global COMBOS

    layers = parse_keymap(keymap_path)
    layout_info = parse_info_json(info_path)

    # Extract combos dynamically
    extracted_combos = extract_combos_from_keymap(keymap_path)
    COMBOS = MANUAL_ACTIONS + extracted_combos

    if not layers:
        print("No layers found!")
        return

    # Create image
    img = Image.new("RGB", (WALLPAPER_WIDTH, WALLPAPER_HEIGHT), BACKGROUND_COLOR)
    draw = ImageDraw.Draw(img)

    # Calculate grid layout (3 rows x 3 columns for 7 layers)
    grid_cols = 3
    grid_rows = 3
    grid_gap_x = 60  # Horizontal gap between layers
    grid_gap_y = 40  # Vertical gap between rows

    # Key dimensions - sized to fill screen
    key_width = 94
    key_height = 94
    key_gap = 2

    # Calculate actual layer dimensions based on keyboard layout
    max_x = max(k["x"] for k in layout_info)
    max_y = max(k["y"] for k in layout_info)
    layer_width = int((max_x + 1) * (key_width + key_gap))
    layer_height = int((max_y + 1) * (key_height + key_gap)) + 60  # +60 for title

    # Calculate total content size (no combos below - they go beside L6)
    grid_height = grid_rows * layer_height + (grid_rows - 1) * grid_gap_y
    total_width = grid_cols * layer_width + (grid_cols - 1) * grid_gap_x

    # Center everything on the wallpaper
    start_x = (WALLPAPER_WIDTH - total_width) // 2
    start_y = (WALLPAPER_HEIGHT - grid_height) // 2

    # Draw each layer in grid
    layer_nums = sorted(layers.keys())
    for idx, layer_num in enumerate(layer_nums[:9]):  # Max 9 layers (3x3 grid)
        row = idx // grid_cols
        col = idx % grid_cols

        x = start_x + col * (layer_width + grid_gap_x)
        y = start_y + row * (layer_height + grid_gap_y)

        draw_layer(
            img,
            draw,
            layers[layer_num],
            layout_info,
            layer_num,
            x,
            y,
            key_width,
            key_height,
        )

    # Draw combos in the empty space next to L6 (row 2, cols 1-2)
    num_layers = len(layer_nums)
    if num_layers <= 7:  # We have empty space in row 2
        combos_x = start_x + 1 * (layer_width + grid_gap_x)
        combos_y = start_y + 2 * (layer_height + grid_gap_y)
        combos_width = 2 * layer_width + grid_gap_x
        draw_combos(draw, combos_x, combos_y, combos_width)

    # Save
    img.save(output_path, "PNG", quality=95)
    print(f"Wallpaper saved to: {output_path}")

    # Refresh XFCE wallpaper
    refresh_xfce_wallpaper(output_path)


def refresh_xfce_wallpaper(image_path):
    """Force XFCE to refresh the wallpaper."""
    try:
        # Get all properties related to last-image
        import subprocess

        cmd = ["xfconf-query", "-c", "xfce4-desktop", "-l"]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            return

        properties = [p for p in result.stdout.splitlines() if p.endswith("last-image")]

        for prop in properties:
            # Check if it's currently set to this image or if we should just update it anyway
            check_cmd = ["xfconf-query", "-c", "xfce4-desktop", "-p", prop]
            current = subprocess.run(
                check_cmd, capture_output=True, text=True
            ).stdout.strip()

            if current == image_path or "mech-keyboard/wallpaper.png" in current:
                # Setting it to the same value often doesn't trigger a refresh if the path hasn't changed.
                # We can try to toggle it or just set it.
                # Sometimes setting it to the absolute path again works.
                subprocess.run(
                    ["xfconf-query", "-c", "xfce4-desktop", "-p", prop, "-s", ""],
                    capture_output=True,
                )
                subprocess.run(
                    [
                        "xfconf-query",
                        "-c",
                        "xfce4-desktop",
                        "-p",
                        prop,
                        "-s",
                        image_path,
                    ],
                    capture_output=True,
                )
                print(f"Refreshed XFCE property: {prop}")

    except Exception as e:
        print(f"Failed to refresh XFCE wallpaper: {e}")


def main():
    base_path = "/home/dcar/projects/mech-keyboard"
    keymap_path = f"{base_path}/keymap/keymap.c"
    info_path = f"{base_path}/qmk_firmware/keyboards/bastardkb/charybdis/4x6/info.json"
    output_path = f"{base_path}/wallpaper.png"

    if not os.path.exists(keymap_path):
        print(f"Error: keymap not found at {keymap_path}")
        return
    if not os.path.exists(info_path):
        print(f"Error: info.json not found at {info_path}")
        return

    generate_wallpaper(output_path, keymap_path, info_path)


if __name__ == "__main__":
    main()
