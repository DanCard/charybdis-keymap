# Developer Guide for Charybdis 4x6 (dcar layout)

This document explains how to build the firmware and how to modify the keymap logic, including the custom "hold-to-toggle" features.

## 1. Keymap Source Files

The keymap source files are stored in two locations:

| Location | Purpose |
|----------|---------|
| `qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/` | **Working files** - Edit and build from here |
| `keymap/` | **Backup** - Version controlled copy for git |

The `qmk_firmware/` directory is a git submodule pointing to the official QMK repository. Since we can't commit to upstream QMK, the keymap is backed up to `keymap/` for version control.

### Workflow

1. **Edit** files in `qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/`
2. **Build** with `qmk compile` (see section 2)
3. **Backup** when ready to commit:
   ```bash
   ./backup.sh
   git commit -m "Update keymap"
   ```

### Restoring from Git

To restore a previous version:
```bash
cp keymap/* qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/
```

### Files

- `keymap.c` - Layer definitions, custom keycodes, and behavior logic
- `config.h` - Charybdis hardware configuration
- `rules.mk` - QMK build rules and features

## 2. Building the Firmware

The project uses QMK. The build command is specific to the hardware setup (Charybdis 4x6 with Elite-Pi/Splinky).

**Command:**
```bash
# Run from the root of the repository or qmk_firmware directory
cd qmk_firmware
qmk compile -kb bastardkb/charybdis/4x6/elitec -km dcar -e CONVERT_TO=elite_pi
```

**Artifact:**
The compiled file is `bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2`.

## 3. Keymap Architecture

The core logic resides in `keymap/keymap.c` (source of truth) which gets copied to `qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/keymap.c` for building.

### A. Keymap Definitions
The key layouts for all layers are defined in the `keymaps` array:
```c
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(...), // Base Layer
    [1] = LAYOUT(...), // Symbols
    // ...
};
```
To change a standard key, simply replace its keycode here (e.g., change `KC_A` to `KC_B`).

### B. Custom Keycodes
Custom behaviors (like "Hold Q to Toggle Layer 4") use custom keycodes defined in the `custom_keycodes` enum:
```c
enum custom_keycodes {
    KC_RAINBOW = QK_USER_0,
    KC_Q_TG4,     // Custom code for Q / Layer 4 Toggle
    KC_PGUP_TG2,  // Custom code for PgUp / Layer 2 Toggle
    // ...
};
```

## 4. Implementing "Hold-to-Toggle" Logic

Standard QMK `LT(layer, key)` (Layer-Tap) functions require you to *hold* the key to keep the layer active. The custom logic implemented here allows you to *hold* the key briefly to **toggle** the layer permanently (on/off), while a quick *tap* sends the keycode.

### Components Required

To add this behavior to a new key (e.g., Key `X` toggling Layer `Y`):

1.  **Define Keycode:** Add `KC_X_TGY` to the `custom_keycodes` enum.
2.  **State Variables:** Define static variables to track the key's state and timer:
    ```c
    static uint16_t x_tap_timer = 0;
    static bool x_held = false;
    static bool x_triggered = false;
    ```
3.  **Process Record (Key Press/Release):**
    In `process_record_user`, handle the press and release events:
    ```c
    case KC_X_TGY:
        if (record->event.pressed) {
            x_held = true;            // Mark as held
            x_triggered = false;      // Reset trigger flag
            x_tap_timer = timer_read(); // Start timer
        } else {
            x_held = false;           // Mark as released
            if (!x_triggered) {       // If we haven't toggled the layer yet...
                tap_code(KC_X);       // ...it was just a tap. Send 'X'.
            }
        }
        return false; // Tell QMK we handled this key manually
    ```
4.  **Matrix Scan (Time-based Trigger):**
    In `matrix_scan_user` (which runs constantly), check if the key has been held long enough:
    ```c
    if (x_held && !x_triggered && timer_elapsed(x_tap_timer) > TAPPING_TERM) {
        layer_invert(Y);     // Toggle Layer Y
        x_triggered = true;  // Mark as triggered so we don't send 'X' on release
    }
    ```

## 5. Tap Dance (Z Key)

The `Z` key uses QMK's "Tap Dance" feature for more complex multi-tap logic:
- **Tap:** Send 'Z'
- **Hold:** Toggle Layer 3 (Mouse)
- **Double Tap:** Toggle Flashlight Mode

This is configured via `tap_dance_actions[]` and the `z_finished`/`z_reset` functions.

## 6. Mouse Logic

The trackball automatically switches to Layer 3 (Mouse) upon movement.
- **Logic:** `pointing_device_task_user` detects movement and calls `layer_on(3)`.
- **Timeout:** `matrix_scan_user` checks `auto_mouse_timer` and turns off Layer 3 after 650ms of inactivity, unless `mouse_is_locked` is true.
