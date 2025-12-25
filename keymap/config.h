#pragma once
#define MASTER_RIGHT
#define SPLIT_USB_DETECT
#define SPLIT_POINTING_ENABLE
#define POINTING_DEVICE_RIGHT

/* Mouse Keys Configuration */
#define MOUSEKEY_DELAY 0
#define MOUSEKEY_INTERVAL 16
#define MOUSEKEY_MAX_SPEED 12
#define MOUSEKEY_TIME_TO_MAX 40
#define MOUSEKEY_WHEEL_DELAY 0

/* Auto Mouse Configuration */
#define AUTO_MOUSE_DEFAULT_LAYER 3    // Switch to Layer 3 (MOUSE)
#define AUTO_MOUSE_TIME 650           // Return to base layer after 650ms of inactivity
#define AUTO_MOUSE_DEBOUNCE 10        // Ignore small accidental movements

#define ENABLE_RGB_MATRIX_SOLID_REACTIVE
#define RGB_MATRIX_KEYPRESSES
#define ENABLE_RGB_MATRIX_SOLID_SPLASH
#define ENABLE_RGB_MATRIX_SPLASH

#define COMBO_TERM 15
#define TAPPING_TERM 200