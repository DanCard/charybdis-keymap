# Charybdis 4x6 Layout Analysis (dcar)

## Matrix vs Physical Mapping

The `LAYOUT` macro maps keys sequentially. For the 4x6 Charybdis, the rows are 12 keys wide (6 left + 6 right).

### Row 3 (Bottom Row) Alignment

**Base Layer (Layer 0):**
| Index | Left Hand Key | Keycode |
| :--- | :--- | :--- |
| 36 | Outer Column (Pinky) | `KC_LCTL` |
| 37 | Pinky Column | `TD(TD_Z_LAYER)` (Z) |
| 38 | Ring Column | `KC_X_TG2` (X) | Tap: X, Long: Toggle L2 |
| 39 | Middle Column | `KC_C_TG1` (C) | Tap: C, Long: Toggle L1 |
| 40 | Index Column | `KC_V` |
| 41 | Inner Column | `KC_B` |

**Mouse Layer (Layer 3):**
| Index | Hand/Key | Keycode | Function |
| :--- | :--- | :--- | :--- |
| 36 | Left Hand, Outer | `KC_NO` | |
| 37 | Left Hand, Pinky | `KC_NO` | |
| 41 | Left Hand, Inner | `KC_NO` | |
| 42 | Right Hand, Inner (N) | `MS_BTN1` | Left Click |
| 43 | Right Hand, Index (M) | `KC_MOUSE_LOCK` | Lock Mouse Layer |
| 44 | Right Hand, Middle (,) | `SNIPING` | Sniping Mode |
| 45 | Right Hand, Ring (.) | `DRGSCRL` | Drag Scroll Mode |
| 46 | Right Hand, Pinky (/) | `KC_TRNS` | (Fallback to LT(3,/)) |
| 47 | Right Hand, Outer | `KC_NO` | |

### Current Functional Mappings

1.  **Persistent Layer 2 (Media):**
    *   **Activation:** Double-tap **Left Inner Thumb**.
    *   **Deactivation:** Single-tap **Left Inner Thumb** (while in Layer 2).
    *   **Code:** Handled in `process_record_user` via `KC_L1_TG2`.

2.  **Persistent Mouse Layer (Layer 3):**
    *   **Activation:** Move trackball (auto-on) -> Press `KC_MOUSE_LOCK`.
    *   **Physical Keys for Lock:**
        *   **Left Control** (Bottom Left Corner)
        *   **Z** key (Next to Bottom Left Corner)
        *   **B** key (Inner-most key, Bottom Row, Left Hand)
    *   **Visual Feedback:** LEDs turn from Yellow (Active) to Pink (Locked).
