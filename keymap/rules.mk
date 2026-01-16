TAP_DANCE_ENABLE = yes
LTO_ENABLE = yes
COMBO_ENABLE = yes
MOUSEKEY_ENABLE = yes
RGB_MATRIX_ENABLE = yes
WS2812_DRIVER = vendor
CONSOLE_ENABLE = yes

SRC += features/sync.c
SRC += features/mouse.c
SRC += features/rgb.c
SRC += features/tap_hold.c
