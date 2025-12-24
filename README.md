# BastardKB Charybdis 4x6 - Custom Layout (Dcar)

This project contains the QMK firmware and custom utilities for my BastardKB Charybdis 4x6 split mechanical keyboard.

## Hardware Setup
- **Keyboard:** BastardKB Charybdis 4x6 (Nano)
- **Controller:** Splinky (RP2040) / Elite-Pi (RP2040)
- **Trackball:** Integrated Right-side Trackball (PMW3360 sensor)
- **Switches:** (Add switch type if known, e.g., Silent Alpacas)

## Features
- **Auto-Mouse Layer:** Automatically switches to the Mouse layer (Layer 3) when the trackball is moved.
- **Mouse Lock:** Toggleable layer for dedicated mouse usage.
- **One-Handed Mode:** mirrored styling for typing with one hand (Layer 4).
- **RGB Feedback:** Layer-specific RGB lighting for visual confirmation.

## Keymap Layers & Colors
The RGB matrix changes color based on the active layer:

| Layer | Name | Color | Description |
| :--- | :--- | :--- | :--- |
| **0** | **BASE** | White | Default QWERTY typing layer. |
| **1** | **SYMBOLS** | Blue | Numbers, symbols, and navigation. |
| **2** | **MEDIA** | Green | Function keys (F1-F12), media controls, volume. |
| **3** | **MOUSE** | Yellow | Mouse clicks, scrolling, DPI, Sniping. (Auto-activates on move) |
| **4** | **ONE-HAND** | Cyan | Mirrored layout for one-handed typing. |
| **3** | **MOUSE LOCK** | Pink | Locked mouse layer (toggled via key). |
| **-** | **FLASHLIGHT** | White | Max brightness white (Double-tap Z). |

## Visualizing the Layout
A Python script is included to print the current keymap layout in the terminal with color coding.

```bash
python3 print_layout.py
```

## Compilation & Flashing

### Prerequisites
- QMK CLI installed
- `arm-none-eabi-gcc` toolchain (usually handled by QMK)

### Compile Command
To compile the firmware with the Elite-Pi converter:

```bash
# From the qmk_firmware directory
qmk compile -kb bastardkb/charybdis/4x6/elitec -km dcar -e CONVERT_TO=elite_pi
```

### Flash Command
The compiled UF2 file will be placed in the `qmk_firmware` directory and copied to the root.
To flash, put the keyboard into bootloader mode (double-tap reset) and copy the `.uf2` file to the mounted drive (RPI-RP2).

```bash
cp bastardkb_charybdis_4x6_elitec_dcar_elite_pi.uf2 /media/$USER/RPI-RP2/
```

## Custom Keycodes & Combos
- **`TD(TD_Z_LAYER)`**: Tap for 'Z', Hold for Mouse Layer, Double-tap for Flashlight Mode.
- **Combos**:
  - `Ctrl+C` (Copy): A + S
  - `Ctrl+V` (Paste): S + D
  - `Ctrl+Shift+V` (Paste Special): D + F
  - `Ctrl+Shift+C` (Copy Special): A + F
  - `Delete`: J + K
