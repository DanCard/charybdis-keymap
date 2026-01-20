# AGENTS.md - Guidelines for AI Agents Working on This Repository

Firmware for BastardKB Charybdis 4x6 with trackball, using QMK Firmware. Custom Python scripts generate documentation.

## Build & Development Commands

```bash
qmk compile -kb bastardkb/charybdis/4x6/elitec -km dcar -e CONVERT_TO=elite_pi
```
Output: `qmk_firmware/bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2`

**CRITICAL**: Must use `-e CONVERT_TO=elite_pi` flag - this keyboard uses Elite-Pi (RP2040), not Elite-C (AVR).

### Flashing Firmware
**IMPORTANT: Flash BOTH controllers for split RGB to work!**

1. Flash right side (master): double-tap reset, copy UF2 to `/media/$USER/RPI-RP2/`
2. Flash left side (slave): disconnect TRRS, hold BOOT while plugging USB, copy same UF2
3. Reconnect TRRS cable

### Testing
No unit tests - verify via `qmk console` (real-time logging). Log with `LOG_TIME(); uprintf("Event\n");`

```bash
python3 generate_layout_pdf.py && python3 wallpaper.py  # Regenerate docs
```

Python deps: `pip install pillow reportlab`

## Code Style Guidelines

### C Code (keymap.c, features/*)
- **2 spaces indentation**
- **CRITICAL**: Do NOT reflow `LAYOUT` macros, enums, or array lists - preserve visual grid layouts
- Custom keycodes: Uppercase with underscores, prefix `KC_` (e.g., `KC_RAINBOW`, `KC_MOUSE_LOCK`)
- Static variables: snake_case (e.g., `is_flashlight`, `auto_mouse_timeout`)
- Functions: snake_case (e.g., `user_sync_info_slave_handler`)
- Constants: UPPERCASE (e.g., `HEARTBEAT_INTERVAL`, `DAY_BRIGHTNESS`)
- Use `static` variables (no dynamic allocation), `extern` globals in .h files
- Types: `uint8_t`, `uint16_t`, `uint32_t`, `bool`, `typedef` for structs, enums for states
- Return `true` to continue QMK processing, `false` to stop

### Feature File Organization
Each feature: `features/feature_name.h` (prototypes, extern globals, typedefs) + `features/feature_name.c` (implementations)

### Python Scripts
- Follow PEP 8, descriptive names, docstrings, module-level constants
- Imports: `json`, `re`, `os`, `from PIL import Image, ImageDraw, ImageFont`, `from reportlab.lib import colors`
- Parse keymap.c with regex, match keycodes via KEY_LABELS dict, use QMK's info.json

## Project-Specific Rules

**From .cursorrules**:
1. 2 spaces indentation
2. **CRITICAL**: Do NOT reflow `LAYOUT` macros, enums, or array lists
3. Preserve visual grid layouts

**QMK Conventions**:
- Split keyboard with custom sync via `USER_SYNC_INFO` transaction
- Trackball: PMW3360 sensor, auto-activates Layer 3 on movement
- RGB Matrix: Both halves sync modes and random seeds
- Symlink: `keymap/` is source of truth, symlinked into qmk_firmware directory

## Git Conventions

**Commit Messages**:
- **CRITICAL**: Do NOT use "feat:" prefix - user preference
- Other prefixes like "fix:", "chore:" are acceptable
- **CRITICAL**: Use longer, detailed messages with bullet points
- Bullet format: describe the "why" first, then list specific changes
- Do NOT mention regeneration of wallpaper.png or charybdis_layout.pdf (done regularly)
- Example:
  ```
  reorganize layers: remove 6 and 7, update settings and tap-hold

  - Removed layers 6 and 7 (Settings and Right Arrows)
  - Renumbered Settings to Layer 5, Right Arrows merged into Layer 1
  - Updated KC_7_L7 to KC_7_L6 in keycodes and tap-hold
  - Removed Layer 6 from RGB indicators (police theme moved to Layer 5)
  - Updated documentation scripts (wallpaper.py, layout_common.py)
  - Reduced NIGHT_BRIGHTNESS from 8 to 1
  ```

## Common Patterns

**Custom Keycodes** - Define in `custom_keycodes` enum (features/keycodes.h):
```c
enum custom_keycodes { KC_CUSTOM = QK_USER_0, KC_ANOTHER };
```
Process in `process_record_user()` with `case KC_CUSTOM: return false;`

**Tap-Hold** (Custom system, NOT QMK mod-tap):
1. Add index to `tap_hold_idx` enum in features/tap_hold.h
2. Add entry to `simple_tap_holds[]` in features/tap_hold.c
3. Handle in `housekeeping_tap_hold()` for layer switch
Macros: `TH_PRESS(idx)`, `TH_CHECK(idx)`, `TH_TRIGGER(idx)`, `TH_RELEASE_TAP(idx)`

**Simple Toggle**:
```c
static bool my_mode = false;
case KC_MY_TOGGLE: if (record->event.pressed) my_mode = !my_mode; return false;
```

**RGB Mode Switch**:
```c
case KC_MY_THEME: if (record->event.pressed) rgb_matrix_mode_noeeprom(RGB_MATRIX_MY_EFFECT); return false;
```

**EEPROM Deferred Write** (avoid USB timeout - delay writes 1500ms via `EEPROM_DEFER_MS`):
```c
static uint16_t pending_eeprom_config = 0; static bool eeprom_update_pending = false; static uint32_t eeprom_defer_timer = 0;
// In housekeeping: if (eeprom_update_pending && timer_elapsed(eeprom_defer_timer) > EEPROM_DEFER_MS) { eeconfig_update_user(pending_eeprom_config); eeprom_update_pending = false; }
```

**Split Sync State Update** (master only):
```c
if (is_keyboard_master()) {
    user_sync_info_t sync_data = { .is_flashlight = is_flashlight, .random_seed = random16_get_seed() };
    transaction_rpc_send(USER_SYNC_INFO, sizeof(sync_data), &sync_data);
}
```

## Important Gotchas

1. **CRITICAL**: Flash BOTH halves for split RGB to work
2. **CRITICAL**: Use `-e CONVERT_TO=elite_pi` - RP2040, not AVR
3. Do NOT reflow LAYOUT macros, enums, or visual arrays
4. Edit in `keymap/` (symlinked to qmk_firmware)
5. Tap-hold is custom system, NOT QMK mod-tap
6. EEPROM writes deferred 1500ms to avoid USB timeout
7. LED indices: local 0-28, add 29 for right half global indices
8. Layer 3 auto-activates on trackball move unless disabled
9. Python deps required: `pip install pillow reportlab`

## File Structure

```
keymap/                              # Source of truth (symlinked to qmk_firmware/.../keymaps/dcar/)
  ├── keymap.c                       # Main logic, layer definitions, process_record_user()
  ├── config.h                       # Hardware config, RGB enables, timeouts, constants
  ├── rules.mk                       # QMK build flags, feature enables, source files
  ├── rgb_matrix_user.inc            # Custom RGB effects
  └── features/                      # Modular feature files (.h/.c pairs)
      ├── keycodes.h                 # Custom keycodes enum
      ├── tap_hold.h/c               # Custom tap-hold state machine
      ├── rgb.h/c                    # RGB matrix effects, modes, custom effects
      ├── sync.h/c                   # Split keyboard sync protocol
      ├── mouse.h/c                  # Trackball logic, mouse modes
      ├── logging.h                  # Logging macros
      └── statistics.h/c             # Key usage statistics

generate_layout_pdf.py, wallpaper.py  # Documentation scripts
```

## Key Features

- **Tap Dance (Z key)**: Tap='Z', Hold=Layer 4 (One-Handed), Double-tap=Flashlight Mode
- **Dual-function numbers (2-9)**: Tap=arrow, Shift+Tap=number, Hold=layer toggle
- **Auto-Mouse Layer**: Trackball movement activates Layer 3 (1500ms timeout)
- **Mouse Modes**: Sniper (250 CPI), Fast (3000 CPI), Lock, Jitter filter
- **Custom RGB Effects**: Fire, Wildfire, Campfire (in rgb_matrix_user.inc)
- **Statistics**: Track key usage, print histogram via `KC_PRINT_STATS`

## Debugging

Use `qmk console` for real-time logs. Log with `LOG_TIME(); uprintf("Event\n");`. Use `layer_change_reason` for layer changes. Debug sync with `KC_DEBUG_SYNC`.
