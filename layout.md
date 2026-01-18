# Keyboard Layout Visualization & Guide

This document provides a visual reference for the current keymap layout of the BastardKB Charybdis 4x6 (Dcar's Layout).

## Visual Layout

### BASE (Layer 0)
```
Esc       1/L1      2/L2      3/L3      4/L4      5/L5     |  6/L6      7/L7      8         9         0         -
Tab       Q/L4      W         E         R         T        |  Y         U         I         O         P         \
Shift     A         S         D         F         G        |  H         J         K         L         +/:       '
Ctrl      Z(TD)     X         C         V         B        |  N         M         ,         .         ?//       Shift
                     Space/L2  Ent/L4    L1(Tog)            |  Del       Ent/L2    Alt
                    Alt       Bspc
```
*Note: This is the "Clean" layer. Standard numbers. Hold numbers to peek other layers.*

### base + arrows (Layer 1)
```
Esc       1/L1      2/L2      3/L3      4/L4      5/L5     |  6/L6      7/L7      8         9         0         -
Tab       Q/L4      W         E         R         T        |  Y         U         I         O         P         Up
Shift     A         S         D         F         G        |  H         J         K         L         Left      Right
Ctrl      Z(TD)     X         C         V         B        |  N         M         ,         .         Down      Shift
                     Space/L2  Ent/L4    L1(Tog)            |  Del       Ent/L2    Alt
                    Alt       Bspc
```
*Note: Base layer copy. Right-hand punctuation keys replaced with Arrows (Up, Left, Right, Down).*

### FUNCTION (Layer 2)
```
Boot      F1        F2        F3        F4        F5       |  F6        F7        F8        F9        F10       Boot
PScr      Exit      Exit      Exit      Exit      Boot     |  Exit      Exit      Exit      Exit      Exit      F11
Shift     Left      Up        Down      Right     A-Home   |  Exit      Left      Down      Up        Right     F12
Ctrl      Home      PgUp      PgDn      End       Exit     |  Exit      Home      PgUp      PgDn      End       Shift
                    Spc/Exit  Ent/Exit  L1(Tog)            |  Exit      Ent/Exit  Alt
```

### MOUSE (Layer 3)
```
Esc       Exit      FastUp    Exit      Exit      Boot     |  Exit      RCtl      RAlt      RGui      Exit      Boot
Exit      DiagUL    MsUp      DiagUR    Btn1      Exit     |  Exit      Exit      Snipe     Fast      Exit      Exit
FastL     MsL       Btn1      MsR       FastR     Btn2     |  Exit      Btn1      DragScr   MsLock    Btn2      Exit
Exit      DiagDL    MsDn      DiagDR    Btn3      Exit     |  Exit      Exit      Btn3      Btn3      Btn3      Shift
                    To L2     To L4     To L6              |  Exit      Exit      Exit
```
*Note: This layer activates automatically on trackball movement.*

### ONE-HAND (Layer 4)
```
-/Base    0/L1      9/L2      8/L3      7/Base    6/Base   |  6         7         8         9         0         -
 \         P/L0      O         I         U         Y        |  Y         U         I         O         P         \
'         +/:       L         K         J         H        |  H         J         K         L         +/:       '
 Shift     / L0       .         ,         M         N        |  N         M         ,         .         / L0      RCtl
                    Space     Ent/Exit  Shift              |  R_TG2     Ent/Exit  Alt
```

### NAV / NUM (Layer 5)
```
PScr      1/L1      2/L2      3/L3      4/L4      5/L5     |  6/Left    7         8         9         0/Base    Boot
Tab       -         7         8         9         Exit     |  Exit      [         ]         {         }         Hypr(N)
Exit      +         4         5         6         Exit     |  Exit      Left      Up        Down      Right     Exit
Ctrl      0         1         2         3         =        |  Exit      Exit      Exit      Exit      Exit      Exit
                    Spc/Exit  Ent/Exit  L1(Tog)            |  R_TG2     Ent/Exit  Alt
```

### SETTINGS (Layer 6)
```
PScr      Exit      Exit      Exit      Exit      Exit     |  DbgSync*  PStats*   PStatsGr* Exit      Exit      PScr
 Exit      RGB_Tog   RGB_Nxt   RGB_Prv   AutoRGB   Exit     |  Fire      Val+      Val-      Exit      Exit      ClrEE
 Exit      FlashLt   Val+      Val-      Day*      Night*   |  Exit      DPI+      DPI-      Jitter    Exit      SyncDbg
Exit      Hue+      Hue-      Sat+      Sat-      Exit     |  Exit      Time+     Time-     Exit      Exit      Exit
                    Exit      Exit      Exit               |  Exit      Exit      Exit
```
*Note: Keys marked with * auto-exit to Layer 0 after activation. Access via **Hold '5'**.*

### old arrow base (Layer 7)
```
Esc       1         Left/L2   Up/L3     Down/L4   Right/L5 |  Left/L6   7/Up      8/Down    9/Right   0/L0      -
Tab       Q/L4      W         E         R         T        |  Y         U         I         O         P         \
Shift     A         S         D         F         G        |  H         J         K         L         +/:       '
Ctrl      Z(TD)     X         C         V         B        |  N         M         ,         .         ?//       Shift
                     Space/L2  Ent/L4    Exit               |  Del       Ent/L2    Alt
                    Alt       Bspc
```
*Note: Dual-function arrow row (Old Layer 1). Tap: Arrow (Left, Up, Down, Right pattern), Shift+Tap: Number.*

## User Guide

### 1. Layers Overview
- **Base (0):** Clean layer. Standard numbers. Hold numbers to peek other layers (e.g., Hold '2' for Layer 2).
- **base + arrows (1):** Base Layer variant. Right-hand punctuation keys (+\':./) map to Arrows (Up, Left, Right, Down). Access via **Hold '1'**.
- **Function (2):** F-keys and navigation. Access via **Hold '2'**.
- **Mouse (3):** Trackball layer. Automatically activates on movement.
- **One-Hand (4):** Mirrored layout. Access via **Hold '4'** or **Hold 'Z'**.
- **Nav/Num (5):** Numpad and secondary navigation. Access via **Hold '6'** (on Layer 0/1) or Mouse Thumb Hold.
- **Settings (6):** RGB, Mouse config, and Statistics. Access via **Hold '5'**.
- **old arrow base (7):** Legacy Arrow layer. Dual-function numbers.

### 2. Timing (LONG_PRESS_TIMEOUT)
The unified timing for all tap-hold actions is **250ms**. This applies to Layer-Tap (LT), custom logic, and statistics tracking (Short vs Long presses).

### 3. Combos
Home-row combos for quick number entry and navigation.

| Keys (Hold Together) | Result |
| :--- | :--- |
| **A + S** | 2 |
| **S + D** | 3 |
| **D + F** | 4 |
| **A + F** | 5 |
| **J + :** | 6 |
| **J + K** | 7 |
| **K + L** | 8 |
| **L + :** | 9 |
| **A + D** | Delete |
| **Z + X** | Home |
| **X + C** | Page Up |
| **C + V** | Page Down |
| **Z + V** | End |
| **LShift + RShift** | Caps Lock |

### 4. Statistics Tracking
The firmware tracks physical key usage, categorized into **Short (<250ms)** and **Long (>=250ms)** presses. Use the Settings layer (Hold '5') to print these statistics to the console.
- **KC_PRINT_STATS**: Prints a detailed list.
- **KC_PRINT_STATS_GRID**: Prints a 2D histogram aligned with the Charybdis physical layout.

