# AGENTS.md - Guidelines for AI Agents Working on This Repository

Firmware for BastardKB Charybdis 4x6 with trackball, using QMK Firmware. Custom Python scripts generate documentation.

## Build & Development Commands

### Building Firmware
```bash
./build.sh                                          # Quick build for Elite-Pi (RP2040)
qmk compile -kb bastardkb/charybdis/4x6/elitec -km dcar -e CONVERT_TO=elite_pi
```
Output: `qmk_firmware/bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2`

**CRITICAL**: Must use `-e CONVERT_TO=elite_pi` flag - this keyboard uses Elite-Pi (RP2040), not Elite-C (AVR).

### Flashing Firmware
**IMPORTANT: For split RGB to work, you must flash BOTH controllers!**

1. **Flash right side (master with trackball)**:
   - Put in bootloader mode: double-tap reset OR hold BOOT while plugging USB
   - Copy UF2: `cp bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2 /media/$USER/RPI-RP2/`

2. **Flash left side (slave)**:
   - Disconnect TRRS cable
   - Put left controller in bootloader: hold BOOT continuously while plugging USB
   - Copy SAME UF2 file: `cp bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2 /media/$USER/RPI-RP2/`

3. **Reconnect**: Reconnect TRRS cable, plug USB into right side (master)

### Diagnostics & Testing
```bash
qmk console                                        # View real-time logs from keyboard
```

This project has no traditional unit tests. Verify by flashing firmware and using `qmk console`.

**Logging in C code**:
```c
uprintf("[%lu.%03lu] Event: %s\n", sec, ms, description);
```
Use `LOG_TIME()` macro defined in `features/logging.h` for consistent timestamps:
```c
LOG_TIME();
uprintf("My event occurred\n");
```

### Documentation Generation
```bash
python3 generate_layout_pdf.py                     # Update charybdis_layout.pdf
python3 wallpaper.py                                 # Update wallpaper.png
```

**Python dependencies**:
- `pip install pillow` - for wallpaper.py
- `pip install reportlab` - for generate_layout_pdf.py

Both scripts parse `keymap.c` to extract layout and QMK's `info.json` for physical key positions.

## Code Style Guidelines

### C Code (keymap.c, features/*)

**Indentation & Formatting**
- Use 2 spaces for indentation
- **CRITICAL**: Do NOT reflow or change line endings in `LAYOUT` macro arrays or visual grid layouts
- Preserve existing visual alignment (especially in keyboard keymaps)
- Keep visual grid layouts intact - they represent physical keyboard arrangement

**Naming Conventions**
- Custom keycodes: Uppercase with underscores, prefix `KC_` (e.g., `KC_RAINBOW`, `KC_MOUSE_LOCK`)
- Static variables: snake_case (e.g., `is_flashlight`, `auto_mouse_timeout`)
- Functions: snake_case (e.g., `user_sync_info_slave_handler`)
- Constants: UPPERCASE with underscores (e.g., `HEARTBEAT_INTERVAL`, `DAY_BRIGHTNESS`)
- Tap-hold indices: Prefix `TH_` in enums (e.g., `TH_K1`, `TH_ENT_MO`)
- Feature state globals: snake_case, externed in .h files (e.g., `is_sniping_active`, `mouse_is_locked`)

**Feature File Organization**
Each feature is split into header and implementation:
- `features/feature_name.h` - Function prototypes, extern globals, typedefs
- `features/feature_name.c` - Implementations
- Include pattern: `#include "features/feature_name.h"`

**Custom Keycodes**
Define in `custom_keycodes` enum (features/keycodes.h):
```c
enum custom_keycodes {
  KC_CUSTOM = QK_USER_0,      // Start at QK_USER_0
  KC_ANOTHER,                 // Auto-increments
};
```

Process in `process_record_user()`:
```c
case KC_CUSTOM:
  if (record->event.pressed) {
    // Handle press
  }
  return false;  // Don't continue processing
```

**Tap-Hold Pattern (Custom System)**

This firmware uses a custom tap-hold system (NOT QMK's built-in mod-tap). For keys that toggle layer on hold, send keycode on tap:

1. Add index to `tap_hold_idx` enum in features/tap_hold.h:
   ```c
   enum tap_hold_idx {
       TH_MY_KEY,
       // ...
       TH_COUNT  // Must be last
   };
   ```

2. Add entry to `simple_tap_holds[]` in features/tap_hold.c:
   ```c
   static const simple_tap_hold_t simple_tap_holds[] = {
       {KC_MY_KEY, TH_MY_KEY, KC_MY_TAP_KEY},
   };
   ```

3. Handle in `housekeeping_tap_hold()` for layer switch logic:
   ```c
   if (TH_CHECK(TH_MY_KEY)) {  // Held past timeout, not triggered
       layer_move(MY_LAYER);
       TH_TRIGGER(TH_MY_KEY);   // Mark as triggered
   }
   ```

4. Use macros:
   - `TH_PRESS(idx)` - Start tracking, set held=true, start timer
   - `TH_CHECK(idx)` - Check if held past timeout and not triggered
   - `TH_TRIGGER(idx)` - Mark as triggered (layer activated)
   - `TH_RELEASE_TAP(idx)` - Check if should tap on release

**Layer State Tracking**
Use `layer_change_reason` variable for debugging:
```c
layer_change_reason = "Reason for layer change";
layer_move(target_layer);
rgb_matrix_indicators_user();  // Update RGB
```

**Logging**
```c
LOG_TIME();
uprintf("Event: %s\n", description);
```
Timestamps from `timer_read32()` give seconds.milliseconds format.

**Error Handling**
- Return `true` to continue QMK processing, `false` to stop
- Check split sync operation return values
- Track failures with counters (e.g., `sync_fail_count`)
- Use descriptive state variables for debugging

**Memory Management**
- Use `static` variables for persistent state (no dynamic allocation in embedded C)
- Use `PROGMEM` for large constant arrays (e.g., RGB mode names)
- Defer EEPROM writes to avoid USB timeouts (see EEPROM_DEFER_MS pattern)
- Global state should be `extern` in .h files, defined in .c files

**Types**
- Use QMK types: `uint8_t`, `uint16_t`, `uint32_t`, `bool`
- Use `typedef` for complex structures (especially sync structs)
- Use enums for state machines and mode indices

**LED Indices**
Left half uses global indices 0-28, right half uses 29-57. Local addressing (0-28) used in code - add 29 for right half global indices.

Key LED arrays (from rgb.c):
```c
// Number keys 1-0 (global indices)
const uint8_t number_key_leds[] = {7, 8, 15, 16, 20, 49, 45, 44, 37, 36};
// Letter keys Q-P (under numbers 1-0)
const uint8_t letter_key_leds[] = {6, 9, 14, 17, 21, 50, 46, 43, 38, 35};
```

### Python Scripts

**Style**
- Follow PEP 8 conventions
- Use descriptive variable names
- Include docstrings for functions
- Constants at module level (e.g., LAYER_COLORS, KEY_LABELS)

**Imports**
```python
import json, re, os
from PIL import Image, ImageDraw, ImageFont  # wallpaper.py
from reportlab.lib import colors              # generate_layout_pdf.py
```

**Pattern**:
- Parse keymap.c with regex to extract keycodes
- Match keycodes to display labels via KEY_LABELS dictionary
- Use QMK's info.json for physical key positions
- Layer colors must match RGB settings in keymap.c

### Configuration Files (rules.mk, config.h)

**rules.mk**:
- Enable features with `FEATURE_ENABLE = yes`, one per line
- Compile custom feature files: `SRC += features/feature_name.c`
- Order matters - keymap.c includes features

**config.h**:
- `#pragma once` at top
- `#define` constants in UPPERCASE, group related defines
- Timeouts in milliseconds (e.g., `TAPPING_TERM 250`)
- Hardware-specific tuning (DPI, mouse keys, RGB effects)

## Project-Specific Rules

**From .cursorrules**:
1. 2 spaces indentation
2. **CRITICAL**: Do NOT reflow `LAYOUT` macros, enums, or array lists
3. Preserve visual grid layouts

**QMK Conventions**:
- Split keyboard with custom sync via `USER_SYNC_INFO` transaction
- Trackball: PMW3360 sensor, auto-activates Layer 3 on movement
- RGB Matrix: Both halves sync modes and random seeds
- Build: Must use `-e CONVERT_TO=elite_pi` flag
- Symlink: `keymap/` is source of truth, symlinked into qmk_firmware directory

**Layers (0-6)**:
| Layer | Name | Color | Description |
|-------|------|-------|-------------|
| 0 | BASE | White | Default QWERTY typing |
| 1 | ORIGINAL | Cyan | Numbers, symbols, navigation (original Layer 0 copy) |
| 2 | ARROW | Green | Function keys, media controls, volume |
| 3 | MOUSE | Yellow | Mouse clicks, scrolling, DPI (auto-activates on trackball move) |
| 4 | ONE-HAND | Orange | Mirrored layout for one-handed typing |
| 5 | SETTINGS | Pink | RGB controls, debug keys, settings |
| 6 | NUMPAD | Blue | Navigation copy, numpad functions |

## File Structure

```
keymap/                              # Source of truth (symlinked to qmk_firmware/.../keymaps/dcar/)
  ├── keymap.c                       # Main logic, layer definitions, process_record_user()
  ├── config.h                       # Hardware config, RGB enables, timeouts, constants
  ├── rules.mk                       # QMK build flags, feature enables, source files
  ├── rgb_matrix_user.inc            # Custom RGB effects (fire, wildfire, campfire)
  └── features/                      # Modular feature files (.h/.c pairs)
      ├── keycodes.h                 # Custom keycodes enum
      ├── tap_hold.h/c               # Custom tap-hold state machine
      ├── rgb.h/c                    # RGB matrix effects, modes, custom user effects
      ├── sync.h/c                   # Split keyboard sync protocol
      ├── mouse.h/c                  # Trackball logic, mouse modes (snipe, fast, lock)
      ├── logging.h                  # Logging macros
      └── statistics.h/c             # Key usage statistics, histogram printing

scripts/                             # Utility scripts
  ├── convert_layout.py              # Layout conversion utilities
  ├── find_led_indices.py            # Find LED positions
  └── pretty_print_layout.py         # Pretty-print layout for debugging

generate_layout_pdf.py               # Generate charybdis_layout.pdf
wallpaper.py                         # Generate wallpaper.png
layout.md                            # Human-readable layout documentation
README.md                            # User-facing documentation
DEVELOPING.md                        # Developer guide
build.sh                             # Quick build script

qmk_firmware/                        # QMK Firmware (git submodule)
  └── keyboards/bastardkb/charybdis/4x6/keymaps/dcar/  # Symlink to ../../../../../keymap/
```

## Hardware Details

**Keyboard**: BastardKB Charybdis 4x6 (Nano)
**Controller**: Splinky (RP2040) / Elite-Pi (RP2040)
**Trackball**: Integrated Right-side Trackball (PMW3360 sensor)
**LEDs**: WS2812 driver, 58 total LEDs (29 per half)

**Configuration constants** (config.h):
```c
#define CHARYBDIS_MINIMUM_DEFAULT_DPI 800
#define CHARYBDIS_DEFAULT_DPI_CONFIG_STEP 200
#define TAPPING_TERM 250                           // Tap-hold timeout
#define AUTO_MOUSE_TIME 1500                      // Mouse layer auto-off timeout
#define SNIPE_MODE_TIMEOUT 2500                    // Snipe mode auto-off
#define FAST_MODE_TIMEOUT 2500                     // Fast mode auto-off
#define EEPROM_DEFER_MS 1500                      // Delay EEPROM writes
#define DAY_BRIGHTNESS 225                        // Day mode brightness
#define NIGHT_BRIGHTNESS 16                        // Night mode brightness
#define RGB_AUTO_CYCLE_INTERVAL 30000             // Auto-cycle RGB every 30s
```

## Adding New Features

1. Define keycode in `custom_keycodes` enum (features/keycodes.h)
2. Add processing logic in `process_record_user()` (keymap.c) OR tap-hold system
3. Add logging with `LOG_TIME()` and `uprintf()`
4. Test with `qmk console`
5. Regenerate documentation: `python3 generate_layout_pdf.py && python3 wallpaper.py`

## Common Patterns

**Hold-to-Toggle Layer** (via custom tap-hold system):
```c
// In process_record_user():
case KC_MY_KEY:
  if (record->event.pressed) TH_PRESS(TH_MY_IDX);
  else th[TH_MY_IDX].held = false;
  return false;

// In housekeeping_tap_hold():
if (TH_CHECK(TH_MY_IDX)) {  // Held past timeout, not triggered
  layer_move(MY_LAYER);
  TH_TRIGGER(TH_MY_IDX);
}
```

**Simple Toggle**:
```c
static bool my_mode = false;
case KC_MY_TOGGLE:
  if (record->event.pressed) my_mode = !my_mode;
  return false;
```

**RGB Mode Switch**:
```c
case KC_MY_THEME:
  if (record->event.pressed) rgb_matrix_mode_noeeprom(RGB_MATRIX_MY_EFFECT);
  return false;
```

**Custom RGB Mode Registration** (keymap.c):
```c
bool rgb_matrix_indicators_user(void) {
    // Register custom effects
    rgb_matrix_mode(RGB_MATRIX_CUSTOM_my_effect);
    return false;
}
```

**EEPROM Deferred Write** (to avoid USB timeout):
```c
static uint16_t pending_eeprom_config = 0;
static bool eeprom_update_pending = false;
static uint32_t eeprom_defer_timer = 0;

// In housekeeping task:
if (eeprom_update_pending && timer_elapsed(eeprom_defer_timer) > EEPROM_DEFER_MS) {
    eeconfig_update_user(pending_eeprom_config);
    eeprom_update_pending = false;
}
```

**Split Sync State Update** (master only):
```c
if (is_keyboard_master()) {
    user_sync_info_t sync_data = {
        .is_flashlight = is_flashlight,
        .is_sniping_active = is_sniping_active,
        // ... other fields
        .random_seed = random16_get_seed()
    };
    transaction_rpc_send(USER_SYNC_INFO, sizeof(sync_data), &sync_data);
}
```

## Key Features & Patterns

### Tap Dance (Z Key)
The Z key uses QMK Tap Dance for multi-tap logic:
- Tap: Send 'Z'
- Hold: Toggle Layer 4 (One-Handed)
- Double-tap: Toggle Flashlight Mode (all LEDs white max brightness)

Defined in `tap_dance_actions[]` with `z_finished()`/`z_reset()` handlers.

### Dual-Function Number Keys (2-9)
Numbers 2-9 are dual-function with shift-modified behavior:
- Tap: Arrow key (Left, Up, Down, Right pattern)
- Shift+Tap: Number (2, 3, 4, 5, 6, 7, 8, 9)
- Hold: Toggle layer (varies by key)

Implemented as custom keycodes (e.g., `KC_2_TO2`, `KC_3_TO3`) with special processing.

### Auto-Mouse Layer
Trackball movement automatically activates Layer 3 (MOUSE):
- Logic: `pointing_device_task_user()` detects movement
- Timeout: `AUTO_MOUSE_TIME` (1500ms) inactivity returns to base layer
- Exception: Mouse Lock mode keeps layer active
- Can be disabled with `auto_mouse_on = false`

### Mouse Modes
- **Sniper Mode**: Drops CPI to 250 for pixel-perfect precision
- **Fast Mode**: Boosts CPI to 3000 for large movements
- **Mouse Lock**: Toggles mouse layer on/off
- **Jitter Filter**: Filters small accidental movements

### Custom RGB Effects
Three custom user effects defined in `rgb_matrix_user.inc`:
- `RGB_MATRIX_CUSTOM_fire` - Fire effect
- `RGB_MATRIX_CUSTOM_wildfire` - Wildfire effect
- `RGB_MATRIX_CUSTOM_campfire` - Campfire effect

Registered via `RGB_MATRIX_CUSTOM_USER` flag in config.h.

### Split Keyboard Sync
Custom transaction protocol (`USER_SYNC_INFO`) keeps halves in sync:
- **Master sends**: RGB mode, flashlight state, snipe/fast modes, random seed
- **Slave responds**: Current RGB mode, task counter, local mouse activity
- **Heartbeat**: 15-second interval logs sync health
- **Random seeds**: Synced for effects like DIGITAL_RAIN, PIXEL_FRACTAL

Sync statistics tracked: `sync_success_count`, `sync_fail_count`, `last_sync_time`.

### Statistics System
Tracks key usage and prints histogram:
- `KC_PRINT_STATS` - Print statistics immediately
- `KC_PRINT_STATS_GRID` - Print in 2D grid format
- Automatic tracking via `process_statistics()` in matrix scan

### Debug Keys
- `KC_DEBUG_SYNC` - Dump sync state to console
- `KC_PRINT_STATS*` - Print usage statistics

## Important Gotchas

1. **CRITICAL**: Always flash BOTH halves for split RGB to work
2. **CRITICAL**: Use `-e CONVERT_TO=elite_pi` - this is RP2040, not AVR
3. **Do NOT reflow**: LAYOUT macros, enums, or visual arrays must preserve alignment
4. **Symlink structure**: Edit in `keymap/`, QMK builds via symlink
5. **Tap-hold is custom**: NOT QMK's mod-tap - use the custom system
6. **Split sync timing**: Master polls slave every 50ms for mouse activity
7. **EEPROM deferred**: Writes delayed 1500ms to avoid USB timeout
8. **RGB random seeds**: Must be synced between halves for consistent effects
9. **LED indices**: Local 0-28, add 29 for right half global indices
10. **Layer 3 auto-activation**: Trackball movement triggers it unless disabled
11. **Python dependencies**: Must install pillow and reportlab for documentation
12. **qmk console**: Real-time logging via `uprintf()` is primary debugging method

## Testing Strategy

Since there are no unit tests:
1. Use `qmk console` for real-time debugging
2. Log layer changes with `layer_change_reason`
3. Log state changes (RGB mode, mouse mode, etc.)
4. Use `KC_DEBUG_SYNC` to inspect sync state
5. Use statistics printing to verify key processing
6. Flash firmware and test manually after changes
7. Test both halves independently for split keyboard issues

## RGB Mode Constants

Common RGB modes used in keymap:
- `RGB_MATRIX_CYCLE_LEFT_RIGHT` - Rainbow cycling L→R
- `RGB_MATRIX_SPLASH` - Reactive splash effect
- `RGB_MATRIX_JELLYBEAN_RAINDROPS` - Random colored drops
- `RGB_MATRIX_CYCLE_SPIRAL` - Spiral cycle
- `RGB_MATRIX_RAINBOW_MOVING_CHEVRON` - Rainbow chevron
- `RGB_MATRIX_PIXEL_FRACTAL` - Pixel fractal
- `RGB_MATRIX_CYCLE_PINWHEEL` - Pinwheel cycle
- Custom: `RGB_MATRIX_CUSTOM_fire`, `RGB_MATRIX_CUSTOM_wildfire`, `RGB_MATRIX_CUSTOM_campfire`

Full list in rgb.c `rgb_mode_names[]` array.

## Troubleshooting

**Split keyboard not syncing**:
- Check both halves have same firmware version
- Verify TRRS cable is connected
- Check `qmk console` for sync heartbeat logs
- Use `KC_DEBUG_SYNC` to dump sync state

**RGB not working on one half**:
- Flash BOTH halves again
- Check sync state in console
- Verify LED indices are correct

**Trackball not detected**:
- Check `POINTING_DEVICE_RIGHT` in config.h
- Verify sensor is connected
- Check mouse activity logs in console

**Keymap not working after flash**:
- Verify symlinks are intact
- Check build completed successfully
- Rebuild from scratch: `qmk clean` then build
