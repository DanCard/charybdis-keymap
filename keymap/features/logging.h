#pragma once

#include QMK_KEYBOARD_H

#define LOG_TIME() do { \
    uint32_t _t = timer_read32(); \
    uprintf("[%lu.%03lu] ", (unsigned long)(_t / 1000), (unsigned long)(_t % 1000)); \
} while(0)
