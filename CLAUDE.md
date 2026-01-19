# Claude Memory - Mech Keyboard Project

## Documentation Generation

The wallpaper and PDF layout documentation are generated from Python scripts, not manually created:

- `scripts/generate_wallpaper.py` - Generates `wallpaper.png` (desktop wallpaper showing all layers)
- `scripts/generate_layout_pdf.py` - Generates `charybdis_layout.pdf` (printable PDF of layouts)
- `scripts/layout_common.py` - Shared code: colors, key labels, parsing, and `get_themed_colors_for_key()` for layer-specific highlighting

When updating RGB indicators in `keymap/features/rgb.c`, also update the corresponding color logic in `scripts/layout_common.py` to keep documentation in sync.

Regenerate after changes:
```bash
python3 scripts/generate_wallpaper.py && python3 scripts/generate_layout_pdf.py
```

## LED Layout Reference

Left half LEDs (local indices 0-28):
- Main keys snake through columns: Col0 (0,1,2,3), Col1 (7,6,5,4), etc.
- Thumb cluster: 24=inner bottom, 25=outer bottom, 26=outer top, 27=middle top, 28=inner top

Right half LEDs (local indices 0-28, same pattern mirrored)

## Build Verification

Build timestamp is printed in debug sync dump (`KC_DUMP_LOG` on L6) to verify firmware version.
