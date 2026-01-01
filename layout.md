# Keyboard Layout Visualization & Guide

This document provides a visual reference for the current keymap layout of the BastardKB Charybdis 4x6 (Dcar's Layout).

## Visual Layout

```
BASE (Layer 0)
Esc     1/L1    2/L2    3/L3    4/L4    5          6       7       8       9       0       -      
Tab     Q/L4    W       E       R       T          Y       U       I       O       P       \      
Shift   A       S       D       F       G          H       J       K       L       +/:     '      
Ctrl    Z/Ms    X/L2    C       V       B          N       M       ,       .       //Ms    Shift  

Space   Enter   L1 Toggle                          L2 Toggle Enter   
Alt     Backsp                                     Backsp    
```

```
SYMBOLS (Layer 1 - Toggle Left Thumb Outer)
Esc     1/L1    2/L2    3/L3    4/L4    5          ^       &       *       (       )       _      
Tab     Num -   Num 7   Num 8   Num 9   Num *      [       [       ]       {       }       .      
Exit    Num +   Num 4   Num 5   Num 6   Num /      +       Left    Up      Down    Right   =      
Ctrl    Num 0   Num 1   Num 2   Num 3   Num =      Hue+    Hue-    Sat+    Sat-    Val+    Val-   

Space/Ex Enter/Ex L1 Toggle                        L2 Toggle Enter/Ex
Alt     Back/Ex                                    Back/Ex   
```

```
MEDIA (Layer 2 - Toggle Right Thumb Inner)
`       1/L1    2/L2    3/L3    4/L4    F5         F6      F7      F8      F9      F10     F11    
.       Rainbow React   Jelly   Spiral  Chevron    .       [       ]       {       }       Prev   
Exit    Left    Up      Down    Right   AutoRGB    Exit    Left    Down    Up      Right   Next   
.       Home/Ms PgUp    PgDn    End     .          .       Home    PgUp    PgDn    End     .      

Space   Enter   L1 Toggle                          L2 Toggle Enter   
Del     Backsp                                     Backsp    
```

```
MOUSE (Layer 3 - Auto-Active on Trackball Move or Hold Z)
Boot    ClearEE FastUp  3/L3    4/L4    Next       Trans   Trans   Rainbow React   ClearEE Boot   
Ms3     Trans   DiagUL  Up      DiagUR  ScrMode    DPI     S-D     Turbo   DPI     LedDbg  .      
FastL   Left    Btn1    Right   FastR   .          Btn3    Shift   Ctrl    Alt     Gui     .      
Z/Ms    DiagDL  Down    FastD   DiagDR  .          .       Btn1    MsLock  Snipe   DragScr Trans  

Btn1    Enter   L1 Toggle                          L2 Toggle Enter   
Btn3    Btn2                                       Btn2      
```

```
ONE-HAND (Layer 4 - Hold Q or Hold Enter)
-/Ex    0/L1    9/L2    8/L3    7/Ex    6/Ex       6       7       8       9       0       -      
\       P/Exit  O       I       U       Y          Y       U       I       O       P       \      
'       +/:     L       K       J       H          H       J       K       L       +/:     '      
Shift   //Ms    .       ,       M       N          N       M       ,       .       //Ms    Shift  

Space   Exit    L1 Toggle                          L2 Toggle Exit    
Alt     Backsp                                     Backsp    
```

## User Guide

### 1. Layers Overview
- **Base (0):** Standard QWERTY layout.
- **Symbols (1):** Blue RGB. Access via **Left Outer Thumb** (Toggle) or **Hold '1'**. Contains Numpad (Left hand) and Navigation/RGB controls (Right hand).
- **Media/Function (2):** Green RGB. Access via **Right Inner Thumb** (Toggle) or **Hold '2'** or **Hold 'X'**. Contains F-keys, Media controls, and RGB Animation modes.
- **Mouse (3):** Yellow RGB. **Automatically activates** when you move the trackball or via **Hold 'Z'**. Contains mouse keys, scrolling, and DPI controls.
- **One-Hand (4):** Teal RGB. Access via **Hold 'Q'**, **Hold '4'**, or **Hold Enter**. Mirrored layout for typing with one hand.
- **Flashlight:** White RGB. Double-tap **'Z'** to toggle max brightness white light.

### 2. Special Keys & Features

#### Layer Toggles (Hold for 175ms)
- **Top Row Numbers (1-4):** Hold to momentarily activate Layers 1, 2, 3, or 4 respectively.
- **Z Key:** Tap for 'Z', Hold for **Mouse Layer (3)**, Double-Tap for **Flashlight Mode**.
- **X Key:** Tap for 'X', Hold for **Media Layer (2)**.
- **Q Key:** Tap for 'Q', Hold for **One-Hand Layer (4)**.
- **Enter (Base Layer):** Tap for Enter, Hold for **One-Hand Layer (4)**.
- **Slash (Base Layer):** Tap for '/', Hold for **Mouse Layer (3)**.

#### Thumb Cluster
- **Left Thumbs:**
  - Top: Space, Enter (Hold for L4), Layer 1 Toggle.
  - Bottom: Alt, Backspace.
- **Right Thumbs:**
  - Top: Layer 2 Toggle, Enter (Hold for L4).
  - Bottom: Backspace.

#### Custom Keys
- **+/: Key:** Tap for `+` (Plus), Shift+Tap for `:` (Colon). Located next to 'L'.
- **Exit Keys:**
  - **Thumb Toggles:** Pressing L1/L2 Toggle on their respective layers returns to Base.
  - **P (Layer 4):** Exits One-Hand mode immediately.
  - **Enter/Space/Backspace (Layer 2/4):** Special variants that can exit the layer on tap/hold depending on context.

#### Auto-Mouse
- Moving the trackball switches to Layer 3 (Yellow).
- Reverts to previous layer after **650ms** of inactivity.
- **Mouse Lock:** Press `MouseLck` key (Pink RGB) on Layer 3 to stay in mouse mode.

#### Combos
- `Ctrl+C` (Copy): **A + S**
- `Ctrl+V` (Paste): **S + D**
- `Ctrl+Shift+C` (Copy Special): **A + F**
- `Ctrl+Shift+V` (Paste Special): **D + F**
- `Delete`: **J + K**
