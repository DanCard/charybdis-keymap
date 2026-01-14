# AGENTS.md - Guidelines for AI Agents Working on This Repository

This repository contains firmware for a BastardKB Charybdis 4x6 mechanical keyboard with a trackball. It uses QMK Firmware and includes custom Python scripts for documentation generation.

## Build & Development Commands

### Building Firmware
```bash
./build.sh                    # Quick build for Elite-Pi (RP2040)
qmk compile -kb bastardkb/charybdis/4x6/elitec -km dcar -e CONVERT_TO=elite_pi  # Manual build
```

The compiled `.uf2` file appears in `qmk_firmware/` directory as `bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2`.

### Diagnostics & Logging
```bash
qmk console                  # View real-time logs from the keyboard
```
Use `uprintf()` in C code to send debug messages to the console.

### Documentation Generation
```bash
python3 generate_layout_pdf.py   # Update charybdis_layout.pdf
python3 wallpaper.py             # Update wallpaper.png
```

### Testing
This project does not use traditional unit tests. Testing is done via:
- Flashing firmware to the keyboard and verifying functionality
- `qmk console` for real-time diagnostics
- Manual verification of layer behaviors and RGB effects

## Code Style Guidelines

### C Code (keymap.c, config.h, rgb_matrix_user.inc)

**Indentation & Formatting**
- Use 2 spaces for indentation
- **CRITICAL**: Do NOT reflow or change line endings in `LAYOUT` macro arrays or any visual grid layouts
- Preserve existing visual alignment in code (especially keyboard keymaps)

**Naming Conventions**
- Custom keycodes: Uppercase with underscores, prefix `KC_` (e.g., `KC_RAINBOW`, `KC_MOUSE_LOCK`)
- Static variables: snake_case (e.g., `is_flashlight`, `auto_mouse_timeout`)
- Functions: snake_case (e.g., `user_sync_info_slave_handler`)
- Constants: Uppercase with underscores (e.g., `HEARTBEAT_INTERVAL`)
- Layer names: Uppercase in comments (e.g., `BASE`, `NUMPAD`, `MOUSE`)

**Custom Keycodes**
Define custom keycodes in the `custom_keycodes` enum:
```c
enum custom_keycodes {
  KC_CUSTOM = QK_USER_0,
  KC_ANOTHER,
};
```
Process them in `process_record_user()`:
```c
switch (keycode) {
  case KC_CUSTOM:
    if (record->event.pressed) {
      // Handle press
    }
    return false;
}
```

**Layer State Tracking**
Use `layer_change_reason` variable to explain layer changes for debugging:
```c
static const char *layer_change_reason = NULL;
static char layer_reason_buffer[64];

layer_change_reason = "Reason for layer change";
layer_move(target_layer);
```

**Logging**
Use `LOG_TIME()` macro for timestamped logging:
```c
#define LOG_TIME() do { \
    uint32_t _t = timer_read32(); \
    uprintf("[%lu.%03lu] ", (unsigned long)(_t / 1000), (unsigned long)(_t % 1000)); \
} while(0)

LOG_TIME();
uprintf("Event occurred\n");
```

**Error Handling**
- Use QMK's return values: `true` to continue processing, `false` to stop
- Check return values from split sync operations and log failures
- Use descriptive variable names to track state (e.g., `sync_fail_count`, `sync_success_count`)

**Memory Management**
- Use static variables for persistent state
- Avoid dynamic allocation (embedded C environment)
- Use `PROGMEM` for large constant arrays
- Defer EEPROM writes to avoid USB timeouts on boot

**Types**
- Use QMK types: `uint8_t`, `uint16_t`, `uint32_t`, `bool`
- Use `typedef` for complex structures
- Use enums for state machines and mode constants

### Python Scripts (generate_layout_pdf.py, wallpaper.py, scripts/*)

**Style**
- Follow PEP 8 conventions
- Use descriptive variable names
- Type hints not strictly required but helpful
- Include docstrings for functions

**Imports**
```python
import json
import re
import os
from PIL import Image, ImageDraw, ImageFont  # For wallpaper.py
from reportlab.lib import colors  # For generate_layout_pdf.py
```

**Patterns**
- Parse `keymap.c` using regex to extract LAYOUT arrays
- Handle both local and global LED indices (subtract 29 for right half)
- Map QMK keycodes to human-readable labels via dictionaries

### Configuration Files (rules.mk, config.h)

**rules.mk**
- Enable features with `FEATURE_ENABLE = yes`
- Keep one feature per line
- Comment disabled features with `#` and brief reason

**config.h**
- Use `#pragma once` at the top
- `#define` all constants in UPPERCASE
- Group related defines with comments
- Keep RGB matrix defines together

## Important Project-Specific Rules

### From .cursorrules
1. **Indentation**: 2 spaces is standard
2. **Line Endings**: CRITICAL - Do NOT reflow or change line endings in `LAYOUT` macros, enums, or array lists
3. **Visual Alignment**: Preserve existing visual grid layouts in keyboard keymaps

### QMK-Specific Conventions
- **Split Keyboard**: This is a split keyboard with custom sync protocol via `USER_SYNC_INFO`
- **Trackball**: PMW3360 sensor on right half, auto-activates mouse layer on movement
- **RGB Matrix**: Both halves must sync RGB modes and random seeds
- **Elite-Pi**: Must use `-e CONVERT_TO=elite_pi` flag when building

### Layer System
The keyboard uses 6 layers (0-5):
- Layer 0: BASE (White) - Default typing
- Layer 1: NUMPAD (Blue) - Numbers and symbols
- Layer 2: ARROW (Green) - F-keys, arrows, navigation
- Layer 3: MOUSE (Yellow) - Mouse keys, auto-activates on trackball
- Layer 4: ONE-HAND (Orange) - Mirrored layout
- Layer 5: SETTINGS (Pink) - Configuration

### Custom Features
- **Tap Dance**: `TD(TD_Z_LAYER)` on Z key (tap=Z, hold=Layer 4, double-tap=flashlight)
- **Combos**: Key combinations for copy/paste/delete
- **Auto Mouse**: Layer 3 activates automatically on trackball movement
- **Split Sync**: Custom transaction protocol keeps halves synchronized
- **RGB Effects**: 30+ effects with shared random seeds

## File Structure

```
keymap/                 # Source of truth for keymap files
  ├── keymap.c          # Main keymap logic, custom keycodes, layer definitions
  ├── config.h          # Hardware configuration, RGB matrix enables, timeouts
  └── rules.mk          # QMK build flags and feature enables

qmk_firmware/           # QMK firmware repository (submodule)
  └── keyboards/bastardkb/charybdis/4x6/keymaps/dcar/  # Symlink to ../keymap/

scripts/                # Utility scripts for debugging and analysis
  ├── pretty_print_layout.py
  ├── find_led_indices.py
  └── generate_color_wheel.py

generate_layout_pdf.py  # Generate charybdis_layout.pdf
wallpaper.py            # Generate wallpaper.png
build.sh                # Quick build script
README.md               # Project overview and usage
DEVELOPING.md           # Detailed developer guide
```

## Adding New Features

1. **Define the keycode** in `custom_keycodes` enum
2. **Implement behavior** in `process_record_user()` or appropriate handler
3. **Add logging** with `LOG_TIME()` and `uprintf()` for debugging
4. **Test with `qmk console`** to verify behavior
5. **Update documentation** if adding new layers or features
6. **Regenerate layout** if modifying keymap: `python3 generate_layout_pdf.py && python3 wallpaper.py`

## Common Patterns

### Hold-to-Toggle Layer
```c
static uint16_t my_key_timer = 0;
static bool my_key_triggered = false;

case KC_MY_KEY:
  if (record->event.pressed) {
    my_key_timer = timer_read();
  } else {
    if (!my_key_triggered) {
      tap_code(KC_MY_KEYCODE);
    }
  }
  return false;

// In matrix_scan_user():
if (timer_elapsed(my_key_timer) > MY_TAPPING_TERM && !my_key_triggered) {
  layer_on(MY_LAYER);
  my_key_triggered = true;
}
```

### Toggle Mode
```c
static bool my_mode = false;

case KC_MY_TOGGLE:
  if (record->event.pressed) {
    my_mode = !my_mode;
  }
  return false;
```

### RGB Mode Switch
```c
case KC_MY_THEME:
  if (record->event.pressed) {
    rgb_matrix_mode_noeeprom(RGB_MATRIX_MY_EFFECT);
  }
  return false;
```
