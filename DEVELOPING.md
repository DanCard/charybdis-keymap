# Developer Guide for Charybdis 4x6 (dcar layout)

This document explains how to build the firmware, modify the keymap logic, and use the diagnostic tools.

## 1. Keymap Source Files

The keymap source files are stored in the `keymap/` directory of this repository.

| Location | Purpose |
|----------|---------|
| `keymap/` | **Source of Truth** - Edit files here. |
| `qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/` | **Symlink** - Points back to `keymap/` so QMK can build it. |

### Files
- `keymap.c` - Layer definitions, custom keycodes, and behavior logic.
- `config.h` - Charybdis hardware configuration (DPI, offsets, etc).
- `rules.mk` - QMK build rules and features.

---

## 2. Building the Firmware

### Recommended: Quick Build
Use the provided script in the root directory:
```bash
./build.sh
```

### Manual Build
If you need to pass extra flags:
```bash
cd qmk_firmware
qmk compile -kb bastardkb/charybdis/4x6/elitec -km dcar -e CONVERT_TO=elite_pi
```

**Artifact:** The compiled file is `bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2`.

---

## 3. Live Diagnostics & Logging

This layout includes extensive instrumentation for real-time debugging.

### Viewing Logs
Connect your keyboard and run:
```bash
qmk console
```
You will see real-time output including:
- **Matrix Scans:** Timestamped key presses/releases with millisecond precision.
- **Layer Changes:** Every layer transition is logged with the specific logic reason (e.g., `Auto Mouse Movement`, `Thumb Hold Triggered`).
- **Mouse Stats:** Current CPI, movement deltas, and auto-layer timeouts.
- **Split Sync:** Heartbeat logs showing communication health between the left and right halves.

### Custom Logging
In `keymap.c`, use `uprintf()` to send messages to the console:
```c
uprintf("My custom event happened at %u ms\n", timer_read());
```

---

## 4. Implementing "Hold-to-Toggle" Logic

Standard QMK `LT(layer, key)` (Layer-Tap) functions require you to *hold* the key to keep the layer active. The custom logic implemented here allows you to *hold* the key briefly to **toggle** the layer permanently (on/off), while a quick *tap* sends a standard keycode.

### Components Required
To add this behavior to a new key (e.g., Key `X` toggling Layer `Y`):

1.  **Define Keycode:** Add `KC_X_TGY` to the `custom_keycodes` enum.
2.  **State Variables:** Define static variables to track the key's state and timer.
3.  **Process Record:** Handle the press (start timer) and release (if not triggered, tap keycode).
4.  **Matrix Scan:** Check if `timer_elapsed() > MY_TAPPING_TERM`. If so, toggle the layer and set `triggered = true`.

Refer to existing implementations like `KC_ENT_TG2` in `keymap.c` for concrete examples.

---

## 5. Tap Dance (Z Key)

The `Z` key uses QMK's "Tap Dance" feature for complex multi-tap logic:
- **Tap:** Send 'Z'.
- **Hold:** Toggle **Layer 4** (One-Handed / Mirrored).
- **Double Tap:** Toggle **Flashlight Mode** (All LEDs White Max Brightness).

This is configured via `tap_dance_actions[]` and the `z_finished`/`z_reset` functions.

---

## 6. Mouse & Trackball Logic

### Auto-Mouse Layer
The trackball automatically activates Layer 3 (Mouse) upon movement.
- **Logic:** `pointing_device_task_user` detects movement and signals the master.
- **Timeout:** `matrix_scan_user` turns off Layer 3 after a period of inactivity (default 1.5s), unless **Mouse Lock** is active.

### Sniper & Fast Modes
- **Sniper Mode:** Drops CPI to 250 for precision.
- **Fast Mode:** Boosts CPI to 3000 for large movements.
- Both modes temporarily override Mouse Key speeds as well for consistent feel.

---

## 7. Split Sync Architecture

The left and right halves communicate via a custom RPC transaction (`USER_SYNC_INFO`).
- **Master drives:** RGB modes, flashlight state, and "Show Mode" (flashing digits for mode indices).
- **Slave reports:** Local trackball activity back to master to trigger auto-layering.
- **Random Seeds:** The master shares a random seed every time an RGB effect changes to ensure both halves are perfectly synchronized (e.g., for `DIGITAL_RAIN` or `PIXEL_FRACTAL`).

---

## 8. Generating Documentation (PDF & Wallpaper)

The layout documentation is auto-generated from `keymap.c`.

### Update the PDF
```bash
python3 generate_layout_pdf.py
```
Updates `charybdis_layout.pdf`. If you add new keycodes, update the `custom_map` dictionary in the script to provide a clean label.

### Update the Wallpaper
```bash
python3 wallpaper.py
```
Updates `wallpaper.png`. This script also attempts to refresh the XFCE desktop background automatically if you are on Linux.

### How it Works
Both scripts parse `keymap.c` to extract the `keymaps` array. They use the `info.json` from QMK to determine the physical key positions for the 4x6 Charybdis.