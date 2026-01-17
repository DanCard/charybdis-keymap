# Charybdis 4x6 (dcar layout) Firmware

## Project Overview
This repository contains the custom QMK firmware for a BastardKB Charybdis 4x6 split mechanical keyboard. It features an integrated trackball and runs on Elite-Pi (RP2040) controllers.

**Key Features:**
*   **Auto-Mouse Layer:** Trackball movement automatically activates the mouse layer.
*   **Custom Split Sync:** Custom RPC protocol ensures RGB effects and state are perfectly synchronized between halves.
*   **Hold-to-Toggle:** Custom tap-hold logic (distinct from QMK's mod-tap) for layer toggling.
*   **Extensive Logging:** Real-time diagnostics via `qmk console`.

## Building and Running

### Build Firmware
**CRITICAL:** This project targets the **Elite-Pi (RP2040)** controller. You MUST use the `CONVERT_TO=elite_pi` flag. The default target in QMK is for AVR (Elite-C) and will not work.

**Quick Build:**
```bash
./build.sh
```

**Manual Build:**
```bash
qmk compile -kb bastardkb/charybdis/4x6/elitec -km dcar -e CONVERT_TO=elite_pi
```
*Output:* `qmk_firmware/bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2`

### Flash Firmware
**IMPORTANT:** You must flash **BOTH** halves for split RGB to work correctly.

1.  **Right Half (Master):** Put in bootloader mode (double-tap reset). Copy `.uf2` to the mounted drive.
2.  **Left Half (Slave):** Disconnect TRRS. Put in bootloader mode. Copy the **SAME** `.uf2` file.
3.  **Reconnect:** Reconnect TRRS cable, then plug USB into the right half.

### Diagnostics
View real-time logs:
```bash
qmk console
```

### Documentation Generation
Update the PDF layout and wallpaper after modifying `keymap.c`:
```bash
python3 generate_layout_pdf.py
python3 wallpaper.py
```

## Development Conventions

### Critical Rules
*   **Do NOT Reflow:** Never change the line endings or whitespace of `LAYOUT` macros, visual grid arrays, or enum lists. These are formatted to represent the physical key positions.
*   **Indentation:** Use 2 spaces.
*   **Symlink:** Edit files in `keymap/`. This directory is symlinked to `qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/`. Do not edit files inside `qmk_firmware/` directly.

### Code Structure
*   **`keymap/keymap.c`**: Main entry point, `process_record_user`, and layer definitions.
*   **`keymap/features/`**: Modularized features (RGB, Sync, Mouse, etc.). Each feature has a `.h` and `.c` file.
*   **`keymap/config.h`**: Hardware configuration and constants.
*   **`keymap/rules.mk`**: Build flags and source file inclusion.

### Custom Patterns
*   **Tap-Hold:** Do not use standard QMK `LT()`. Use the custom system defined in `features/tap_hold.c` and `features/tap_hold.h`.
*   **Logging:** Use `LOG_TIME()` and `uprintf()` for consistent timestamped logs.
*   **Split Sync:** Use `user_sync_info_t` and `transaction_rpc_send` to sync state from Master to Slave.

## Architecture

*   **Controller:** Elite-Pi (RP2040)
*   **Trackball:** PMW3360 sensor (Right side).
*   **RGB:** 58 LEDs (29 per half), WS2812 driver.
*   **Sync Protocol:** Master sends state (RGB mode, flashlight, RNG seed) to Slave. Slave sends local activity back to Master.
