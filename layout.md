# Keyboard Layout Visualization & Guide

This document provides a visual reference for the current keymap layout of the BastardKB Charybdis 4x6 (Dcar's Layout).

## Visual Layout

### BASE (Layer 0)
```
Esc       1/L1      2/L2      3/L3      4/L4      5        |  6         7         8         9         0         -
Tab       Q         W         E         R         T        |  Y         U         I         O         P         \
Shift     A         S         D         F         G        |  H         J         K         L         +/:       '
Ctrl      Z(TD)     X         C         V         B        |  N         M         ,         .         /(L3)     Shift
                    Space/L2  Ent/L4    L1(Tog)            |  Del       Ent/L2    Alt
```
*Note: TD(Z) = Tap: Z, Hold: Layer 4. Double Tap: Flashlight.*

### NUMPAD + LIGHT CONTROL (Layer 1)
```
Boot      PScr      Exit      Exit      Exit      Exit     |  Exit      Exit      Exit      Exit      PScr      Boot
Tab       -         7         8         9         *        |  RGBNext   [         ]         {         }         Trans
Exit      +         4         5         6         /        |  RGBPrev   Left      Up        Down      Right     =
Ctrl      0         1         2         3         =        |  Hue+      Hue-      Sat+      Sat-      Val+      Val-
                    Spc/Exit  Ent/Exit  L1(Tog)            |  R_TG2     Ent/Exit  Alt
```

### ARROW (Layer 2)
```
F12       F1        F2        F3        F4        F5       |  F6        F7        F8        F9        F10       F11
PScr      Exit      Exit      Exit      Exit               |  Fire      [         ]         {         }         
Shift     Left      Up        Down      Right     AutoRGB  |  Shift     Left      Down      Up        Right     
Exit      Home      PgUp      PgDn      End       .        |  .         Home      PgUp      PgDn      End       .
                    Spc/Exit  Ent/Exit  L1(Tog)            |  R_TG2     Ent/Exit  Del
```

### MOUSE (Layer 3)
```
Esc       ClrEE     FastUp    3/L3      4/L4      Boot     |  Trans     RCtl      RAlt      RGui      ClrEE     Boot
Ms3       Trans     DiagUL    MsUp      DiagUR    Exit     |  Time-     Time+     DPI+      DPI-      Snipe     Exit
FastL     MsL       Btn1      MsR       FastR              |  Exit      Btn1      DragScr   MsLock    Btn2      Exit
Z/Ms      DiagDL    MsDn      DiagDR    Exit               |  Exit      Exit      Btn3      Btn3      Btn3      RSft
                    Btn1      Btn2      Btn3               |  Exit      Btn1      Exit
```
*Note: This layer activates automatically on trackball movement.*
*Time+/Time-: Adjust auto-mouse timeout in 500ms steps (0.5s - 10s). Lower timeout for gaming, higher for general use.*

### ONE-HAND (Layer 4)
```
-/0       0/L1      9/L2      8/L3      7/0       6/0      |  6         7         8         9         0         -
\         P/0       O         I         U         Y        |  Y         U         I         O         P         \
'         +/:       L         K         J         H        |  H         J         K         L         +/:       '
Shift     /         .         ,         M         N        |  N         M         ,         .         /         Shift
                    Space     Ent/Exit  L1(Tog)            |  R_TG2     Ent/Exit  Alt
```

## User Guide

### 1. Layers Overview
- **Base (0):** Standard QWERTY layout.
- **Numpad + Light Control (1):** Blue RGB. Access via **Left Outer Thumb** (Toggle) or **Hold \'1\'**. Contains Numpad (Left hand) and Navigation/RGB controls (Right hand). Includes dedicated keys for RGB Mode, Hue, Saturation, and Value adjustments.
- **Arrow (2):** Green RGB. Access via **Right Middle Thumb** (Enter/Toggle L2) or **Hold \'2\'**. Contains F-keys, Arrow keys, navigation, and RGB Animation modes.
- **Mouse (3):** Yellow RGB. **Automatically activates** when you move the trackball. Also accessible via **Hold \'/'**.
- **One-Hand (4)::** Pink RGB. Access via **Hold '4'**, **Hold Left Middle Thumb (Enter)**, or **Hold 'Z'**. Mirrored layout for typing with one hand.
- **Flashlight:** White RGB. Double-tap **\'Z\'** (Layer 0) to toggle max brightness white light.

### 2. Combos
Simultaneous key presses for navigation and editing.

| Keys (Hold Together) | Result |
| :--- | :--- |
| **A + S** | Left Arrow |
| **S + D** | Up Arrow |
| **D + F** | Down Arrow |
| **F + G** | Right Arrow |
| **J + K** | Delete |
| **Z + X** | Home |
| **X + C** | Page Up |
| **C + V** | Page Down |
| **V + B** | End |
| **LShift + RShift** | Caps Lock |

### 3. Mouse Features
- **Auto-Mouse:** Moving the trackball switches to Layer 3 (Yellow). Activates on movement > threshold. Reverts to previous layer after timeout period (unless Locked).
- **Timeout Adjustment:** Press `Time+`/`Time-` (Layer 3) to adjust auto-deactivation timeout in 500ms increments (range: 0.5s - 10s). Default is 2.0s. Lower timeout (0.5-1.5s) recommended for gaming; higher timeout (3-5s) for general use.
- **Mouse Lock:** Press `MsLock` (Layer 3, \'H\' position) to lock the mouse layer.
- **Snipe Mode:** Press `Snipe` (Layer 3, \'K\' position) to lower DPI (250) for precision.
- **Drag Scroll:** Press `DragScr` (Layer 3, \'J\' position) to turn trackball into a scroll wheel.
