#!/usr/bin/env python3
"""
Generate a PDF of the Charybdis 4x6 keyboard layout.
Requires: pip install reportlab
"""

import os
import sys

# Add scripts directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from reportlab.lib import colors
from reportlab.lib.pagesizes import LETTER
from reportlab.lib.units import inch
from reportlab.pdfgen import canvas

from layout_common import (
    LAYER_COLORS_NORM,
    LAYER_NAMES,
    KEY_BORDER_COLORS,
    MANUAL_ACTIONS,
    parse_keymap,
    parse_info_json,
    extract_combos_from_keymap,
    simplify_key,
    get_themed_colors_for_key,
)

# Will be populated dynamically + manual entries
COMBOS = []


def draw_key(c, x, y, width, height, label, bg_color):
    """Draw a single key with label."""
    # Key background
    c.setFillColorRGB(*bg_color)
    c.roundRect(x, y, width, height, 4, fill=1, stroke=1)

    # Key label
    c.setFillColor(colors.black)

    # Split lines
    lines = label.split("\n")

    # Calculate settings for each line
    line_settings = []
    total_text_height = 0

    spacing = 2
    if len(lines) >= 3:
        spacing = 0

    for line in lines:
        l = len(line)
        if l <= 1:
            font_size = 14
        elif l <= 3:
            font_size = 12
        elif l <= 4:
            font_size = 9
        elif l <= 7:
            font_size = 8
        else:
            font_size = 6

        # Reduce size for 3-line labels to fit
        if len(lines) >= 3:
            font_size = min(font_size, 11)

        line_height = font_size + spacing
        line_settings.append((line, font_size, line_height))
        total_text_height += line_height

    # Remove spacing from last line for tighter bounding box
    if line_settings:
        total_text_height -= spacing

    current_y = y + (height + total_text_height) / 2

    for line, font_size, line_height in line_settings:
        c.setFont("Helvetica-Bold", font_size)
        text_width = c.stringWidth(line, "Helvetica-Bold", font_size)
        text_x = x + (width - text_width) / 2
        draw_y = current_y - font_size
        c.drawString(text_x, draw_y, line)
        current_y -= line_height


def draw_layer(c, layer_keys, layout_info, layer_num, start_x_arg, start_y, key_size=28):
    """Draw a complete layer on the canvas at specified position."""
    layer_name = LAYER_NAMES.get(layer_num, f"Layer {layer_num}")
    bg_color = LAYER_COLORS_NORM.get(layer_num, (0.9, 0.9, 0.9))

    # Key dimensions
    key_gap = 1

    # Calculate layout bounds to determine width
    max_x = max(k["x"] for k in layout_info)
    layout_width = (max_x + 1) * (key_size + key_gap)

    # Calculate start_x to center on page
    page_width = LETTER[0]
    start_x = (page_width - layout_width) / 2

    # Calculate layout bounds
    max_y = max(k["y"] for k in layout_info)

    # Keys start at start_y and go downward
    offset_y = start_y

    # Title goes above the keys
    c.setFont("Helvetica-Bold", 12)
    c.setFillColor(colors.black)
    title = f"{layer_name} (L{layer_num})"
    title_width = c.stringWidth(title, "Helvetica-Bold", 12)
    title_x = (LETTER[0] - title_width) / 2
    c.drawString(title_x, start_y + key_size + 5, title)

    c.setStrokeColor(colors.gray)
    c.setLineWidth(0.5)

    for i, key_code in enumerate(layer_keys):
        if i >= len(layout_info):
            break

        info = layout_info[i]
        kx = start_x + info["x"] * (key_size + key_gap)
        ky = offset_y - (info["y"]) * (key_size + key_gap)

        # Get themed color or default
        themed_bg, _ = get_themed_colors_for_key(layer_num, info["x"], info["y"], use_255=False)
        key_color = themed_bg if themed_bg else bg_color

        label = simplify_key(key_code, layer_num)
        draw_key(c, kx, ky, key_size, key_size, label, key_color)

    # Return height used for this layer
    return (max_y + 1) * (key_size + key_gap) + 15


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
    left_col1_x = 0.5 * inch
    left_col2_x = 1.7 * inch
    right_col1_x = 4.1 * inch
    right_col2_x = 5.4 * inch

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
    COMBOS = MANUAL_ACTIONS + extracted_combos

    if not layers:
        print("No layers found!")
        return

    page_size = LETTER  # Portrait orientation
    c = canvas.Canvas(output_path, pagesize=page_size)
    c.setTitle("mech keyboard layout")
    page_width, page_height = page_size

    # Layer pages - fit 3 layers per page
    layer_nums = sorted(layers.keys())
    margin = 0.3 * inch
    key_size = 38  # Sized to fit 3 layers per page

    for i in range(0, len(layer_nums), 3):
        current_y = page_height - margin - key_size  # Account for title above keys

        # Draw up to 3 layers on this page
        for j in range(3):
            if i + j >= len(layer_nums):
                break

            layer_num = layer_nums[i + j]
            height_used = draw_layer(
                c,
                layers[layer_num],
                layout_info,
                layer_num,
                margin,
                current_y,
                key_size=key_size,
            )
            current_y -= height_used + 8  # Add spacing between layers

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
    base_path = "/home/dcar/projects/mech-keyboard"
    keymap_path = f"{base_path}/keymap/keymap.c"
    info_path = f"{base_path}/qmk_firmware/keyboards/bastardkb/charybdis/4x6/info.json"
    output_path = f"{base_path}/charybdis_layout.pdf"

    if not os.path.exists(keymap_path):
        print(f"Error: keymap not found at {keymap_path}")
        return
    if not os.path.exists(info_path):
        print(f"Error: info.json not found at {info_path}")
        return

    generate_pdf(output_path, keymap_path, info_path)


if __name__ == "__main__":
    main()
