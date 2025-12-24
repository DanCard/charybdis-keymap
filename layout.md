# Keyboard Layout Visualization & Guide

This document provides a visual reference for the current keymap layout of the BastardKB Charybdis 4x6.

## Visual Layout

```
BASE
Esc     1       2       3       4       5          6       7       8       9       0       -      
Tab     Q/TG(4) W       E       R       T          Y       U       I       O       P       \      
Shift   A       S       D       F       G          H       J       K       L       ;       '      
Ctrl    Z/TG(3) X/TG(2) C/TG(1) V/TG(5) B          N       M       ,       .       //Mouse Shift  

Space   Ent/Hold L4 L1 Toggle                           L2 Toggle Ent/Hold L4
Alt     Backsp                                           Backsp 


SYMBOLS (Layer 1 - Toggle Left Thumb Inner)
~       !       @       #       $       %          ^       &       *       (       )       _      
RGB Nex Rainbow React   .       .       .          [       [       ]       Down    Right   .      
Shift   Win     Alt     Ctrl    Shift   .          Num +   Left    Up      Down    Right   +      
Ctrl    Home/T3 PgUp/T2 PgDn/T1 End     .          Num *   Home    PgUp    PgDn    End     Num .  

.       Ent/Hold B  L0 Toggle                           L0 Toggle Ent/Hold B  
Delete  .                                                Num +  


MEDIA (Layer 2 - Toggle Right Thumb Inner)
`       F1      F2      F3      F4      F5         F6      F7      F8      F9      F10     F11    
Next    Rainbow React   .       .       .          [       [       ]       Down    Right   Vol+   
Play    Left    Up      Down    Right   .          .       Shift   Ctrl    Alt     Win     Mute   
Prev    Home/T3 PgUp/T2 PgDn/T1 End     .          Num *   Home    PgUp    PgDn    End     Vol-   

trans   Ent/Hold B  L0 Toggle                           L0 Toggle Ent/Hold B  
Delete  trans                                            .      


MOUSE (Layer 3 - Auto-Active on Trackball Move or Toggle Z)
Reset   Clear   .       .       .       .          .       .       .       .       Reset   Clear  
.       .       .       .       DPI     Snipe      Snipe   DPI     .       .       .       .      
.       Win     Alt     Ctrl    Shift   .          MidClk  Shift   Ctrl    Alt     Win     .      
.       Home/T3 PgUp/T2 PgDn/T1 Snipe   PasteSp    L-Click MouseLck Snipe   DragScr trans   .      

LeftClk RightCl L0 Toggle                           L0 Toggle LeftClk
MidClk  Ent/Hold B                                       Ent/Hold B


ONE-HAND (Layer 4 - Toggle 'Q')
-       0       9       8       7       6          6       7       8       9       0       -      
\       P/TO(0) O       I       U       Y          Y       U       I       O       P       \      
'       ;       L       K       J       H          H       J       K       L       ;       '      
Shift   / /T3   . /T2   , /T1   M       N          N       M       ,       .       //Mouse Shift  

Space   Ent/Hold B  L0 Toggle                           L0 Toggle Ent/Hold B
Alt     Backsp                                           Backsp 


NUMPAD (Layer 5 - Toggle 'V')
Esc     1       2       3       4       5          6       7       8       9       0       -      
Tab     Num -   Num 7   Num 8   Num 9   Num *      Y       U       I       O       P       \      
Shift   Num +   Num 4   Num 5   Num 6   Num /      H       J       K       L       ;       '      
Ctrl    Num 0   Num 1   Num 2   3/Base  Num =      N       M       ,       .       L3 Toggle Ent/Hold L4

Space   Ent/Hold B  L1 Toggle                           L2 Toggle Ent/Hold B
Alt     Backsp                                           Backsp 
```

## User Guide

### 1. Layers Overview
- **Base (Undefined):** Standard QWERTY layout. No specific color override (shows default animation).
- **Symbols (Blue):** Access via **Left Inner Thumb** (Toggle) or **Long Press C**. Contains numbers, symbols (`!@#$`), and navigation keys.
- **Media/Function (Green):** Access via **Right Inner Thumb** (Toggle) or **Long Press X**. Contains F-keys (`F1-F12`), media controls (Play/Pause, Vol+/-), and arrow keys.
- **Mouse (Yellow):** **Automatically activates** when you move the trackball or via **Long Press Z**.
- **One-Hand (Cyan):** Access via **Long Press Q** or **Hold Enter** (either hand). Mirrored layout for left-handed use.
- **Numpad (Purple):** Access via **Long Press V**. Standard calculator layout on the left half.

### 2. Special Keys & Features

#### Layer Toggles (Hold for 175ms to Trigger)
- **Long Press 'V' Key:** Toggles Layer 5 (Numpad). Returns to Base if already on Layer 5.
- **Long Press 'Q' Key:** Toggles Layer 4 (One-Hand). Returns to Base if already on Layer 4.
- **Long Press 'Z' Key:** Toggles Layer 3 (Mouse). Returns to Base if already on Layer 3.
- **Long Press 'X' Key:** Toggles Layer 2 (Media). Returns to Base if already on Layer 2.
- **Long Press 'C' Key:** Toggles Layer 1 (Symbols). Returns to Base if already on Layer 1.
- **Tap Behaviors:** 
  - Layers 1 & 2: Z=Home, X=PgUp, C=PgDn, V=End.
  - Layer 4: Z=/, X=., C=,, V=M.
  - Layer 5: V=Num 3.

#### Exit Keys (Immediate)
- **Thumb Toggle Keys (All Layers):** Pressing the Left or Right Inner Thumb keys on any non-base layer will immediately return to Layer 0 (Base).
- **Long Press 'P' (Layer 4):** Returns to Layer 0 (Base).

#### Momentary Enter Toggles (Universal)
- **From Base Layer:** Holding **either Enter key** momentarily switches to **Layer 4 (One-Hand)**.
- **From Any Other Layer:** Holding **either Enter key** momentarily switches to **Base Layer**.
- **Tapping:** Always sends a standard **Enter** keypress.

#### Auto-Mouse Layer
- Detects trackball movement and switches to the **Mouse Layer (Yellow)** automatically.
- Reverts after **650ms** of inactivity unless **Mouse Lock** (Pink) is active.

#### Combos
- Simultaneous press (15ms window): `A+S` (Copy), `S+D` (Paste), `A+F` (Copy Special), `D+F` (Paste Special), `J+K` (Delete).

### 3. Thumb Cluster
- **Left Thumbs:** Space, Enter (Hold for L4/Base), Layer 1 Toggle (Return to Base if not on L0), Alt, Backspace.
- **Right Thumbs:** Layer 2 Toggle (Return to Base if not on L0), Enter (Hold for L4/Base), Backspace.