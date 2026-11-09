// Halcyon TFT LCD module (ST7789, 135x240) on the VIK connector.
// Pin values taken from splitkb's halcyon_modules userspace; cross-check against your module's docs.
#pragma once

#define LCD_RST_PIN     GP26
#define LCD_CS_PIN      GP13
#define LCD_DC_PIN      GP16
#define LCD_BL_PIN      GP27 // backlight, driven as plain on/off
#define LCD_SPI_DIVISOR 0
#define LCD_SPI_MODE    3
#define LCD_WIDTH       135
#define LCD_HEIGHT      240
#define LCD_OFFSET_X    52
#define LCD_OFFSET_Y    40

#define QUANTUM_PAINTER_SUPPORTS_NATIVE_COLORS TRUE
#define ST7789_NO_AUTOMATIC_VIEWPORT_OFFSETS
#define ST7789_NUM_DEVICES 1

// Halcyon Cirque trackpad on the right half (VIK, SPI). Values from splitkb's halcyon_modules userspace.
#define SPLIT_POINTING_ENABLE
#define POINTING_DEVICE_RIGHT
#define POINTING_DEVICE_CS_PIN GP13
#define CIRQUE_PINNACLE_DIAMETER_MM 35
#define CIRQUE_PINNACLE_CURVED_OVERLAY
#define CIRQUE_PINNACLE_POSITION_MODE CIRQUE_PINNACLE_ABSOLUTE_MODE
#define CIRQUE_PINNACLE_TAP_ENABLE
#define POINTING_DEVICE_GESTURES_CURSOR_GLIDE_ENABLE
#define POINTING_DEVICE_GESTURES_SCROLL_ENABLE

// Keep layer / default-layer state in sync between halves (needed for the display on the slave half)
#define SPLIT_LAYER_STATE_ENABLE

// Circular scroll: wheel clicks per full turn of the finger. QMK default is 18; lower = slower scrolling.
#define SCROLL_CLICKS_PER_TURN 9
