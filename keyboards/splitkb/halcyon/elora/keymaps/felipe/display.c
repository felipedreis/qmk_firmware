// Shows the active profile (MAC / WIN) and the current layer on the Halcyon TFT display.
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "qp.h"
#include "drivers/sensors/cirque_pinnacle.h"
#include "drivers/sensors/cirque_pinnacle_gestures.h"
#include "graphics/big.qff.h"
#include "graphics/small.qff.h"

// Layer indexes live in keymap.c; keep in sync with `enum layers` there.
enum { L_MAC, L_MSYM, L_NAV, L_WIN, L_WSYM, L_WNAV, L_ADJUST };

static painter_device_t lcd;
static painter_font_handle_t font_big_h, font_small_h;
static int8_t last_profile = -1, last_layer = -1;

static void draw_centered(const char *text, painter_font_handle_t font, uint16_t y, uint8_t fg_h, uint8_t fg_s, uint8_t fg_v, uint8_t bg_h, uint8_t bg_s, uint8_t bg_v) {
    uint16_t w = qp_textwidth(font, text);
    qp_drawtext_recolor(lcd, (LCD_WIDTH - w) / 2, y, font, text, fg_h, fg_s, fg_v, bg_h, bg_s, bg_v);
}

void keyboard_post_init_user(void) {
    // Circular scroll tuning (runs on both halves; only the half with the trackpad uses it).
    // args: outer ring %, trigger px, trigger angle (pi = 32768; 9102 = 50 deg), wheel clicks per full turn, left-handed
    cirque_pinnacle_configure_circular_scroll(SCROLL_RING_PCT, SCROLL_TRIGGER_PX, 9102, SCROLL_CLICKS_PER_TURN, false);

    if (!is_keyboard_left()) return; // display is on the left half

    gpio_set_pin_output(LCD_BL_PIN);
    gpio_write_pin_high(LCD_BL_PIN);

    lcd = qp_st7789_make_spi_device(LCD_WIDTH, LCD_HEIGHT, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, LCD_SPI_DIVISOR, LCD_SPI_MODE);
    qp_init(lcd, QP_ROTATION_0);
    qp_set_viewport_offsets(lcd, LCD_OFFSET_X, LCD_OFFSET_Y);
    qp_power(lcd, true);
    font_big_h   = qp_load_font_mem(font_big);
    font_small_h = qp_load_font_mem(font_small);
    qp_rect(lcd, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, 0, 0, 0, true);
    qp_flush(lcd);
}

void housekeeping_task_user(void) {
    if (!is_keyboard_left() || !lcd) return;

    uint8_t base    = get_highest_layer(default_layer_state);
    uint8_t top     = get_highest_layer(layer_state | default_layer_state);
    int8_t  profile = (base == L_WIN) ? 1 : 0;
    if (profile == last_profile && top == last_layer) return;

    const char *layer_name;
    switch (top) {
        case L_MSYM:
        case L_WSYM:   layer_name = "SYMBOLS"; break;
        case L_NAV:
        case L_WNAV:   layer_name = "NAV"; break;
        case L_ADJUST: layer_name = "ADJUST"; break;
        default:       layer_name = "BASE"; break;
    }

    // Mac = blue, Win = orange
    uint8_t hue = profile ? 20 : 170;
    qp_rect(lcd, 0, 0, LCD_WIDTH - 1, 119, hue, 255, 200, true);
    draw_centered(profile ? "WIN" : "MAC", font_big_h, 38, 0, 0, 255, hue, 255, 200);
    qp_rect(lcd, 0, 120, LCD_WIDTH - 1, LCD_HEIGHT - 1, 0, 0, 0, true);
    draw_centered(layer_name, font_small_h, 160, 0, 0, 255, 0, 0, 0);
    qp_flush(lcd);

    last_profile = profile;
    last_layer   = top;
}
