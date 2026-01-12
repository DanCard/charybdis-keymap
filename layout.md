# Keyboard Layout Visualization & Guide

This document provides a visual reference for the current keymap layout of the BastardKB Charybdis 4x6 (Dcar's Layout).

## Visual Layout

### BASE (Layer 0)
```
Esc       1/L1      2/L2      3/L3      4/L4      5/L5     |  6         7         8         9         0         -
Tab       Q         W         E         R         T        |  Y         U         I         O         P         \
Shift     A         S         D         F         G        |  H         J         K         L         +/:       '
Ctrl      Z(TD)     X         C         V         B        |  N         M         ,         .         /(L3)     Shift
                    Space/L2  Ent/L4    L1(Tog)            |  Del       Ent/L2    Alt
```
*Note: TD(Z) = Tap: Z, Hold: Layer 4. Double Tap: Flashlight.*

### NUMPAD (Layer 1)
```
PScr      Exit      To L2     To L3     To L4     Exit     |  Exit      Exit      Exit      Exit      PScr      Boot
Tab       -         7         8         9         *        |  Exit      [         ]         {         }         Hypr(N)
Exit      +         4         5         6         /        |  Exit      Left      Up        Down      Right     Exit
Ctrl      0         1         2         3         =        |  Exit      Exit      Exit      Exit      Exit      Exit
                    Spc/Exit  Ent/Exit  L1(Tog)            |  R_TG2     Ent/Exit  Alt
```

### ARROW (Layer 2)
```
Boot      F1        F2        F3        F4        F5       |  F6        F7        F8        F9        F10       Boot
PScr      Exit      Exit      Exit      Exit      Exit     |  Exit      Exit      Exit      Exit      Exit      F11
Shift     Left      Up        Down      Right     A-Home   |  Exit      Left      Down      Up        Right     F12
Ctrl      Home      PgUp      PgDn      End       Exit     |  Exit      Home      PgUp      PgDn      End       Shift
                    Spc/Exit  Ent/Exit  L1(Tog)            |  R_TG2     Ent/Exit  Alt
```

### MOUSE (Layer 3)
```
Esc       Exit      FastUp    Exit      Exit      Boot     |  Exit      RCtl      RAlt      RGui      Exit      Boot
Exit      DiagUL    MsUp      DiagUR    Btn1      Exit     |  Exit      Exit      Snipe     Fast      Exit      Exit
FastL     MsL       Btn1      MsR       FastR     Btn2     |  Exit      Btn1      DragScr   MsLock    Btn2      Exit
Exit      DiagDL    MsDn      DiagDR    Btn3      Exit     |  Exit      Exit      Btn3      Btn3      Btn3      Shift
                    To L2     To L4     L1(Tog)            |  Exit      Exit      Exit
```
*Note: This layer activates automatically on trackball movement.*

### ONE-HAND (Layer 4)
```
-/Base    0/L1      9/L2      8/L3      7/Base    6/Base   |  6         7         8         9         0         -
\         P/Base    O         I         U         Y        |  Y         U         I         O         P         \
'         +/:       L         K         J         H        |  H         J         K         L         +/:       '
Shift     /(L3)     .         ,         M         N        |  N         M         ,         .         /(L3)     RCtl
                    Space     Ent/Exit  Shift              |  R_TG2     Ent/Exit  Alt
```

### SETTINGS (Layer 5)
```
PScr      Exit      Exit      Exit      Exit      Exit     |  Exit      Hue+      Hue-      Sat+      Sat-      PScr
Exit      RGB_Tog   RGB_Nxt   RGB_Prv   AutoRGB   P.Frac   |  Fire      Val+      Val-      Exit      Exit      ClrEE
Exit      Exit      Exit      Day       Night     Exit     |  Exit      DPI+      DPI-      Jitter    Exit      SyncDbg
Exit      Exit      Exit      Exit      Exit      Exit     |  Pinwhl    Time+     Time-     Exit      Exit      Exit
                    Exit      Exit      Exit               |  Exit      Exit      Exit
```
*Note: Access via **Hold '5'**. Contains all RGB and mouse configuration settings.*

## User Guide

### 1. Layers Overview
- **Base (0):** Standard QWERTY layout.
- **Numpad (1):** Blue RGB. Access via **Left Outer Thumb** (Toggle) or **Hold '1'**. Contains Numpad (Left hand) and Navigation (Right hand).
- **Arrow (2):** Green RGB. Access via **Right Middle Thumb** (Enter/Toggle L2) or **Hold '2'**. Contains F-keys, Arrow keys, and navigation.
- **Mouse (3):** Yellow RGB. **Automatically activates** when you move the trackball. Also accessible via **Hold '/'**.
- **One-Hand (4):** Pink RGB. Access via **Hold '4'**, **Hold Left Middle Thumb (Enter)**, or **Hold 'Z'**. Mirrored layout for typing with one hand.
- **Settings (5):** Cyan RGB. Access via **Hold '5'**. Contains all RGB controls (Hue, Saturation, Value, Mode) and mouse settings (DPI, Snipe, Timeout, Jitter).
- **Flashlight:** White RGB. Double-tap **'Z'** (Layer 0) to toggle max brightness white light.

### 2. Combos
Simultaneous key presses for navigation and editing.

| Keys (Hold Together) | Result |
| :--- | :--- |
| **A + S** | Left Arrow |
| **S + D** | Up Arrow |
| **D + F** | Down Arrow |
| **F + G** | Right Arrow |
| **A + F** | Right Arrow |
| **J + K** | Delete |
| **Z + X** | Home |
| **X + C** | Page Up |
| **X + V** | Paste (Term) |
| **C + V** | Page Down |
| **V + B** | End |
| **Z + V** | End |
| **LShift + RShift** | Caps Lock |

### 3. Mouse Features
- **Auto-Mouse:** Moving the trackball switches to Layer 3 (Yellow). Activates on movement > threshold. Reverts to previous layer after timeout period (unless Locked).
- **Mouse Lock:** Press `MsLock` (Layer 3, 'L' position) to lock the mouse layer.
- **Drag Scroll:** Press `DragScr` (Layer 3, 'K' position) to turn trackball into a scroll wheel.
- **Snipe Mode:** Press `Snipe` (Layer 3, 'I' position) to lower DPI for precision aiming.

### 4. Settings Layer (Hold '5')
All configuration settings are now on the Settings layer for easy access:
- **RGB Controls:** Toggle, Next/Prev mode, Hue+/- , Saturation+/-, Value+/-, AutoRGB
- **RGB Animations:** Direct access to Fire, Pixel Fractal, and Pinwheel effects
- **DPI Adjustment:** Press `DPI+`/`DPI-` to adjust mouse sensitivity.
- **Snipe Mode:** Press `Snipe` to toggle low DPI (250) for precision. Also available on Layer 3 at 'I' position.
- **Timeout Adjustment:** Press `Time+`/`Time-` to adjust auto-mouse timeout in 500ms increments (range: 0.5s - 10s). Default is 2.0s.
- **Jitter Filter:** Press `Jitter` to toggle the mouse jitter filter (useful for high-DPI stability).
- **Debug/Maintenance:** `ClrEE` (Clear EEPROM), `SyncDbg` (Debug sync state), `PScr` (Print Screen).
- **Brightness Presets:** `Day` (High Brightness) and `Night` (Low Brightness) on 'D' and 'F' keys.
