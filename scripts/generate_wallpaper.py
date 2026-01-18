#!/usr/bin/env python3
"""
Generate a desktop wallpaper of the Charybdis 4x6 keyboard layout.
Requires: pip install pillow
"""

import os
import sys

# Add scripts directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from PIL import Image, ImageDraw, ImageFont

from layout_common import (
    LAYER_COLORS_255,
    LAYER_NAMES,
    KEY_BORDER_COLORS,
    MANUAL_ACTIONS,
    L6_RED_BORDER,
    L6_BLUE_BORDER,
    L1_PINK_BORDER,
    L1_CYAN_BORDER,
    L7_RAINBOW_BORDER,
    parse_keymap,
    parse_info_json,
    extract_combos_from_keymap,
    simplify_key,
    get_themed_colors_for_key,
)

# Wallpaper settings
WALLPAPER_WIDTH = 3840
WALLPAPER_HEIGHT = 2160
BACKGROUND_COLOR = (26, 26, 46)  # Dark blue-gray

# Will be populated dynamically
COMBOS = []


def get_font(size, bold=False):
    """Get a font, falling back to default if custom fonts unavailable."""
    # Preferred fonts in order
    preferred_fonts = [
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",  # DejaVu Sans (regular)
    ]

    for path in preferred_fonts:
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size)
            except:
                continue

    # Fallback fonts
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
        draw.arc([x1, y1, x1 + radius * 2, y1 + radius * 2], 180, 270, fill=outline, width=width)
        draw.arc([x2 - radius * 2, y1, x2, y1 + radius * 2], 270, 360, fill=outline, width=width)
        draw.arc([x1, y2 - radius * 2, x1 + radius * 2, y2], 90, 180, fill=outline, width=width)
        draw.arc([x2 - radius * 2, y2 - radius * 2, x2, y2], 0, 90, fill=outline, width=width)
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
        (x + shadow_offset, y + shadow_offset, x + width + shadow_offset, y + height + shadow_offset),
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
        h = bbox[3] - bbox[1]
        h += 14  # Add some leading/spacing

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


def draw_layer(img, draw, layer_keys, layout_info, layer_num, start_x, start_y, key_width=88, key_height=88):
    """Draw a complete layer at the specified position."""
    layer_name = LAYER_NAMES.get(layer_num, f"Layer {layer_num}")
    bg_color = LAYER_COLORS_255.get(layer_num, (200, 200, 200))
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

        # Get themed color or default
        themed_bg, themed_border = get_themed_colors_for_key(layer_num, info["x"], info["y"], use_255=True)
        key_bg = themed_bg if themed_bg else bg_color
        key_border = themed_border if themed_border else border_color

        label = simplify_key(key_code, layer_num)
        if label:  # Only draw if there's a label
            draw_key(draw, kx, ky, key_width, key_height, label, key_bg, key_border)

    return (max_y + 1) * (key_height + key_gap) + 55


def draw_combos(draw, start_x, start_y, width):
    """Draw the combos section in 2 columns."""
    current_y = start_y

    title_font = get_font(44, bold=True)
    text_font = get_font(34)

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

    current_y += 55  # Spacing after title

    # Split combos into two columns
    n = len(COMBOS)
    half = (n + 1) // 2
    col1_combos = COMBOS[:half]
    col2_combos = COMBOS[half:]

    # Calculate column positions - use more of available width
    col_content_width = (width * 55) // 100  # 55% for each column to spread out more
    combo_start_x = start_x
    action_offset = 280  # Space between keys and action text

    line_height = 44

    max_rows = max(len(col1_combos), len(col2_combos))
    for i in range(max_rows):
        if i < len(col1_combos):
            keys, action = col1_combos[i]
            draw.text((combo_start_x + 20, current_y), f"{keys}", fill=(200, 200, 220), font=text_font)
            draw.text((combo_start_x + action_offset, current_y), f": {action}", fill=(160, 160, 180), font=text_font)

        if i < len(col2_combos):
            keys, action = col2_combos[i]
            draw.text((combo_start_x + col_content_width + 20, current_y), f"{keys}", fill=(200, 200, 220), font=text_font)
            draw.text((combo_start_x + col_content_width + action_offset, current_y), f": {action}", fill=(160, 160, 180), font=text_font)

        current_y += line_height


def generate_wallpaper(output_path, keymap_path, info_path):
    """Generate the wallpaper with all layers in a 2x3 grid."""
    global COMBOS

    layers = parse_keymap(keymap_path)
    layout_info = parse_info_json(info_path, thumb_shift_left=2.0, thumb_shift_right=1.5, right_main_shift=4.5)

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

        draw_layer(img, draw, layers[layer_num], layout_info, layer_num, x, y, key_width, key_height)

    # Draw combos in the empty space in row 2
    num_layers = len(layer_nums)
    if num_layers <= 8:  # We have empty space in row 2 (8 layers = positions 0-7, leaving col 2 empty)
        # Position combos in remaining columns of row 2
        cols_used_in_row2 = num_layers - 6  # Layers 6, 7 use cols 0, 1
        combos_col = cols_used_in_row2
        combos_x = start_x + combos_col * (layer_width + grid_gap_x)
        combos_y = start_y + 2 * (layer_height + grid_gap_y)
        combos_width = (3 - combos_col) * layer_width + (2 - combos_col) * grid_gap_x
        draw_combos(draw, combos_x, combos_y, combos_width)

    # Save
    img.save(output_path, "PNG", quality=95)
    print(f"Wallpaper saved to: {output_path}")

    # Refresh XFCE wallpaper
    refresh_xfce_wallpaper(output_path)


def refresh_xfce_wallpaper(image_path):
    """Force XFCE to refresh the wallpaper."""
    try:
        import subprocess

        cmd = ["xfconf-query", "-c", "xfce4-desktop", "-l"]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            return

        properties = [p for p in result.stdout.splitlines() if p.endswith("last-image")]

        for prop in properties:
            check_cmd = ["xfconf-query", "-c", "xfce4-desktop", "-p", prop]
            current = subprocess.run(check_cmd, capture_output=True, text=True).stdout.strip()

            if current == image_path or "mech-keyboard/wallpaper.png" in current:
                subprocess.run(
                    ["xfconf-query", "-c", "xfce4-desktop", "-p", prop, "-s", ""],
                    capture_output=True,
                )
                subprocess.run(
                    ["xfconf-query", "-c", "xfce4-desktop", "-p", prop, "-s", image_path],
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
