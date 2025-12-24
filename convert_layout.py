import json
import os

def convert():
    with open('charybdis.layout.json', 'r') as f:
        data = json.load(f)
    
    layers = data['layers']
    
    custom_map = {
        "CUSTOM(0)": "DPI_MOD", "CUSTOM(1)": "DPI_RMOD", "CUSTOM(2)": "S_D_MOD",
        "CUSTOM(3)": "S_D_RMOD", "CUSTOM(4)": "SNIPING", "CUSTOM(5)": "SNP_TOG",
        "CUSTOM(6)": "DRGSCRL", "CUSTOM(7)": "DRG_TOG", "RESET": "QK_BOOT",
        "RGB_MOD": "RM_NEXT", "KC_MS_BTN1": "MS_BTN1", "KC_MS_BTN2": "MS_BTN2",
        "KC_MS_BTN3": "MS_BTN3"
    }
    
    def map_key(k):
        return custom_map.get(k, k)

    processed_layers = []
    for layer in layers:
        new_layer = [None] * 56
        for r in range(4):
            for c in range(6):
                new_layer[r * 12 + c] = map_key(layer[r * 6 + c])
                new_layer[r * 12 + 6 + c] = map_key(layer[30 + r * 6 + (5 - c)])
        new_layer[48] = map_key(layer[27])
        new_layer[49] = map_key(layer[28])
        new_layer[50] = map_key(layer[25])
        new_layer[51] = map_key(layer[55])
        new_layer[52] = map_key(layer[57])
        new_layer[53] = map_key(layer[29])
        new_layer[54] = map_key(layer[26])
        new_layer[55] = map_key(layer[59])
        processed_layers.append(new_layer)

    processed_layers[0][53] = "KC_LALT"
    processed_layers[1][53] = "KC_DEL"
    processed_layers[2][53] = "KC_DEL"
    processed_layers[3][53] = "KC_DEL"
    processed_layers[3][40] = "C(S(KC_V))"
    processed_layers[3][38] = "TG(4)"
    processed_layers[3][41] = "TG(5)"

    l0 = processed_layers[0]
    layer4 = list(l0)
    for r in range(4):
        for c in range(6):
            layer4[r * 12 + c] = l0[r * 12 + 6 + c]
    layer4[48] = l0[51]
    layer4[49] = l0[52]
    layer4[54] = l0[55]
    layer4[50] = "TG(4)"
    layer4[53] = "KC_DEL"
    processed_layers.append(layer4)

    layer5 = list(processed_layers[3])
    layer5[41] = "TG(5)"
    processed_layers.append(layer5)

    c_content = """#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
"""

    for i, layer in enumerate(processed_layers):
        keys = ", ".join(layer)
        c_content += f"    [{i}] = LAYOUT({keys}),\n"

    c_content += """};

#ifdef RGB_MATRIX_ENABLE
bool rgb_matrix_indicators_user(void) {
    uint8_t layer = get_highest_layer(layer_state);
    switch (layer) {
        case 5: rgb_matrix_set_color_all(255, 255, 255); return false; // White (Locked)
        case 4: rgb_matrix_set_color_all(255, 0, 0); return false; // Red (One-Hand)
        case 3: rgb_matrix_set_color_all(255, 255, 0); return false; // Yellow (Mouse)
        case 2: rgb_matrix_set_color_all(0, 255, 0); return false; // Green (Function)
        case 1: rgb_matrix_set_color_all(0, 0, 255); return false; // Blue (Symbols)
        default: return true;
    }
}
#endif
"""

    with open('qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/keymap.c', 'w') as f:
        f.write(c_content.replace("\n", "\n"))
    
    with open('qmk_firmware/keyboards/bastardkb/charybdis/4x6/keymaps/dcar/config.h', 'w') as f:
        f.write('#pragma once\n#define MASTER_RIGHT\n#define SPLIT_USB_DETECT\n#define SPLIT_POINTING_ENABLE\n#define POINTING_DEVICE_RIGHT\n'.replace("\\n", "\n").replace("\n", "\n"))

if __name__ == "__main__":
    convert()
