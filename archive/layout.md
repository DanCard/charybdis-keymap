# Keyboard Layout Visualization & Guide

This document provides a visual reference for the current keymap layout of the BastardKB Charybdis 4x6 (Dcar's Layout).

## Visual Layout

```
                                                 BASE (Layer 0)
Esc       1/L1      2/L2      3/L3      4/L4      5                6         7         8         9         0         -        
Tab       Q/L4      W         E         R         T                Y         U         I         O         P         \        
Shift     A         S         D         F         G                H         J         K         L         +/:       '        
Ctrl      Z/Ms      X/L2      C         V         B                N         M         ,         .         //Ms      Shift    
                               Space/TG2 Ent/TG2   L1 Toggle        Delete    Ent/TG2   
                               Alt       Backsp                     Backsp    
```

```
                                               SYMBOLS (Layer 1)
~         !         @         #         $         %                ^         &         *         (         )         _        
Tab       Num-/L4   Num 7     Num 8     Num 9     Num *            [         [         ]         {         }         .        
Exit      Num +     Num 4     Num 5     Num 6     Num /            +         Left      Up        Down      Right     =        
Ctrl      Num 0     Num 1     Num 2     Num 3     Num =            Hue+      Hue-      Sat+      Sat-      Val+      Val-     
                               Space/Ex  Enter/Ex  L1 Toggle        L2 Toggle Enter/Ex  
                               Alt       Back/Ex                    Back/Ex   
```

```
                                                MEDIA (Layer 2)
F12       F1        F2        F3        F4        F5               F6        F7        F8        F9        F10       F11
L/R       Exit      Exit      Exit      Exit      Exit             Flash     [         ]         {         }         Prev     
Shift     Left      Up        Down      Right     AutoRGB          Shift     Left      Down      Up        Right     Next     
Exit      Home/Ms   PgUp      PgDn      End       .                .         Home      PgUp      PgDn      End       .        
                               Space     Enter     L1 Toggle        L2 Toggle Enter     
                               Del       Backsp                     Backsp    
```

```
                                                MOUSE (Layer 3)
Boot      ClearEE   FastUp    3/L3      4/L4      Next             Trans     Trans     .         .         ClearEE   Boot
Ms3       Trans     DiagUL    Up        DiagUR    ScrMode          DPI+      ScrollLck LayerLock Snipe     DPI-      .
FastL     Left      Btn1      Right     FastR     .                Exit      LeftClk   MidClk    MidClk    RightClk  Exit
Z/Ms      DiagDL    Down      FastD     DiagDR    .                .         Exit      Ctrl      Alt       Gui       Trans
                               Btn1      Enter     L1 Toggle        L2 Toggle Enter
                               Btn3      Btn2                       Btn2
```

```
                                               ONE-HAND (Layer 4)
-/Ex      0/L1      9/L2      8/L3      7/Ex      6/Ex             6         7         8         9         0         -        
\         P/Exit    O         I         U         Y                Y         U         I         O         P         \        
'         +/:       L         K         J         H                H         J         K         L         +/:       '        
Shift     //Ms      .         ,         M         N                N         M         ,         .         //Ms      Shift    
                               Space     Exit      L1 Toggle        L2 Toggle Exit      
                               Alt       Backsp                     Backsp    
```

## User Guide

### 1. Layers Overview
- **Base (0):** Standard QWERTY layout.
- **Symbols (1):** Blue RGB. Access via **Left Outer Thumb** (Toggle) or **Hold '1'**. Contains Numpad (Left hand) and Navigation/RGB controls (Right hand).
- **Media/Function (2):** Green RGB. Access via **Right Inner Thumb** (Toggle) or **Hold '2'** or **Hold 'X'**. Contains F-keys, Media controls, and RGB Animation modes.
- **Mouse (3)::** Yellow RGB. **Automatically activates** when you move the trackball or via **Hold 'Z'**. Contains mouse keys, scrolling, and DPI controls.
- **One-Hand (4):** Pink RGB. Access via **Hold 'Q'**, **Hold '4'**, or **Hold Enter**. Mirrored layout for typing with one hand.
- **Flashlight:** White RGB. Double-tap **'Z'** to toggle max brightness white light.

...

#### Auto-Mouse
- Moving the trackball switches to Layer 3 (Yellow). Activates on ANY movement.
- Reverts to previous layer after **2000ms (2s)** of inactivity.
- **Layer Lock:** Press `LayerLock` key on Layer 3 to lock the layer and prevent auto-timeout.
- **Scroll Lock:** Press `ScrollLck` (I key) on Layer 3 to lock to Layer 3 and enable scroll mode. Visual indicator: rainbow far left column. Press again to exit.
- **Snipe Mode:** Press `Snipe` to toggle high precision/low speed (Red RGB). Press again to exit.

#### Combos
- `Ctrl+C` (Copy): **A + S**
- `Ctrl+V` (Paste): **S + D**
- `Ctrl+Shift+C` (Copy Special): **A + F**
- `Ctrl+Shift+V` (Paste Special): **D + F**
- `Delete`: **J + K**