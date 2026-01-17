# Keyboard Layout Visualization & Guide

This document provides a visual reference for the current keymap layout of the BastardKB Charybdis 4x6 (Dcar's Layout).

## Visual Layout

### BASE (Layer 0)
```
Esc       1/L1      2/L        3/U        4/D        5/L       |  6/L6       7/U        8/D        9/L        0         -
Tab       Q/L4      W          E          R          T         |  Y          U          I          O          P         \
Shift     A          S          D          F          G         |  H          J          K          L          +/:       '
Ctrl      Z(TD)     X          C          V          B         |  N          M          ,          .          /(L3)     Shift
                     Space/L2   Ent/L4     L1(Tog)             |  Del        Ent/L2     Alt
                    Alt        Bspc
```
*Note: Numbers 2-9 are dual-function. Tap: Arrow (Left, Up, Down, Left pattern), Shift+Tap: Number. Key 6 Hold: Switch to Layer 6 (Nav).*

### NUMBERS (Layer 1)
```
Esc       1/L1      2/L2      3/L3      4/L4      5/L5     |  6         7         8         9         0/L0      -
Tab       Q         W         E         R         T        |  Y         U         I         O         P         \
Shift     A         S         D         F         G        |  H         J         K         L         +/:       '
Ctrl      Z(TD)     X         C         V         B        |  N         M         ,         .         /(L3)     Shift
                     Space/L2  Ent/L4    L1(Tog)            |  Del       Ent/L2    Alt
                    Alt       Bspc
```
*Note: This is a copy of the original Layer 0 for standard number access. Key 0 Hold: Switch back to Layer 0.*

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
                    To L2     To L4     To L6 (Nav)        |  Exit      Exit      Exit
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

### SETTINGS (Layer 5)
```
PScr      Exit      Exit      Exit      Exit      Exit     |  DbgSync*  PStats*   PStatsGr* Exit      Exit      PScr
 Exit      RGB_Tog   RGB_Nxt   RGB_Prv   AutoRGB   P.Frac   |  Fire      Val+      Val-      Exit      Exit      ClrEE
 Exit      Exit      Exit      Day*      FlashLt   Night*   |  Exit      DPI+      DPI-      Jitter    Exit      SyncDbg
Exit      Exit      Exit      Exit      Exit      Exit     |  Pinwhl    Time+     Time-     Exit      Exit      Exit
                    Exit      Exit      Exit               |  Exit      Exit      Exit
```
*Note: Keys marked with * auto-exit to Layer 0 after activation. Access via **Hold '5'**.*

### NAV (Layer 6)
```
PScr      Exit      To L2     To L3     To L4     Exit     |  Exit      Exit      Exit      Exit      PScr      Boot
Tab       -         7         8         9         *        |  Exit      [         ]         {         }         Hypr(N)
Exit      +         4         5         6         /        |  Exit      Left      Up        Down      Right     Exit
Ctrl      0         1         2         3         =        |  Exit      Exit      Exit      Exit      Exit      Exit
                    Spc/Exit  Ent/Exit  L1(Tog)            |  R_TG2     Ent/Exit  Alt
```
*Note: This used to be Layer 1. Access via **Hold '6'** on Layer 0 or **Mouse Thumb Hold**.*

## User Guide

### 1. Layers Overview
- **Base (0):** Dual-function row. Numbers 2-9 act as Arrows (L, U, D, L pattern). Shift+Tap sends the number. Hold '6' to access Nav (L6).
- **Numbers (1):** Original standard number row. Access via **Hold '1'**. Hold '0' to return to Base.
- **Arrow (2):** F-keys and navigation. Access via **Right Middle Thumb** (Enter/Toggle L2).
- **Mouse (3):** Trackball layer. Automatically activates on movement.
- **One-Hand (4):** Mirrored layout. Access via **Hold '4'** or **Hold 'Z'**.
- **Settings (5):** RGB, Mouse config, and Statistics. Access via **Hold '5'**. Most functions auto-exit to Layer 0.
- **Nav (6):** Numpad and secondary navigation. Access via **Hold '6'** on Layer 0 or **Mouse Thumb Hold**.

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

