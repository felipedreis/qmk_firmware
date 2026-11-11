/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 * Copyright 2022 Daniel Kao <daniel.m.kao@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <lib/lib8tion/lib8tion.h>
#include "cirque_pinnacle_gestures.h"
#include "pointing_device.h"
#include "timer.h"
#include "wait.h"
#if defined(SPLIT_POINTING_ENABLE) && defined(POINTING_DEVICE_COMBINED)
#    include "keyboard.h"
#endif

#if (defined(CIRQUE_PINNACLE_TAP_ENABLE) || defined(CIRQUE_PINNACLE_CIRCULAR_SCROLL_ENABLE)) && CIRQUE_PINNACLE_POSITION_MODE
static cirque_pinnacle_features_t features = {.tap_enable = true, .circular_scroll_enable = true};
#endif

#if defined(CIRQUE_PINNACLE_TAP_ENABLE) && CIRQUE_PINNACLE_POSITION_MODE
static trackpad_tap_context_t tap;

static report_mouse_t trackpad_tap(report_mouse_t mouse_report, pinnacle_data_t touchData) {
    bool down = touchData.touchDown;

    /* Finish a right-click pulse started by a long press */
    if (tap.rclick_reports && --tap.rclick_reports == 0) {
        mouse_report.buttons = pointing_device_handle_buttons(mouse_report.buttons, false, POINTING_DEVICE_BUTTON2);
    }

    if (down && !tap.touchDown) {
        /* Touch started. A quick re-touch after a tap becomes a drag (or the second click of a double click). */
        tap.touchDown  = true;
        tap.down_timer = timer_read();
        tap.start_x    = touchData.xValue;
        tap.start_y    = touchData.yValue;
        if (tap.recent_tap && timer_elapsed(tap.release_timer) < CIRQUE_PINNACLE_DRAG_WINDOW) {
            tap.state = TAP_DRAG;
        } else {
            tap.state = TAP_TOUCH;
        }
        tap.recent_tap = false;
    } else if (!down && tap.touchDown) {
        /* Touch ended. A short, nearly motionless touch is a tap: click now. */
        tap.touchDown = false;
        if (tap.state == TAP_TOUCH && timer_elapsed(tap.down_timer) < CIRQUE_PINNACLE_TAPPING_TERM) {
            mouse_report.buttons = pointing_device_handle_buttons(mouse_report.buttons, true, POINTING_DEVICE_BUTTON1);
            tap.release_timer    = timer_read();
            tap.recent_tap       = true;
        }
        tap.state = TAP_IDLE;
    } else if (down && tap.state == TAP_TOUCH) {
        int32_t dx = (int32_t)touchData.xValue - tap.start_x;
        int32_t dy = (int32_t)touchData.yValue - tap.start_y;
        if (dx * dx + dy * dy > (int32_t)CIRQUE_PINNACLE_TAP_MOVE_LIMIT * CIRQUE_PINNACLE_TAP_MOVE_LIMIT) {
            tap.state = TAP_MOVED;
        } else if (CIRQUE_PINNACLE_LONGPRESS_TERM && timer_elapsed(tap.down_timer) >= CIRQUE_PINNACLE_LONGPRESS_TERM) {
            /* Held still: right click */
            mouse_report.buttons = pointing_device_handle_buttons(mouse_report.buttons, true, POINTING_DEVICE_BUTTON2);
            tap.rclick_reports   = 3;
            tap.state            = TAP_LONG_FIRED;
        }
    }

    /* Hold the left button for the whole second touch (drag / double click) */
    if (tap.state == TAP_DRAG && tap.touchDown) {
        mouse_report.buttons = pointing_device_handle_buttons(mouse_report.buttons, true, POINTING_DEVICE_BUTTON1);
    }

    /* A stale tap expires */
    if (tap.recent_tap && timer_elapsed(tap.release_timer) >= CIRQUE_PINNACLE_DRAG_WINDOW) {
        tap.recent_tap = false;
    }

    return mouse_report;
}

void cirque_pinnacle_enable_tap(bool enable) {
    features.tap_enable = enable;
}

bool cirque_pinnacle_drag_active(void) {
    return tap.state == TAP_DRAG && tap.touchDown;
}
#endif

#ifdef CIRQUE_PINNACLE_CIRCULAR_SCROLL_ENABLE
#    if !CIRQUE_PINNACLE_POSITION_MODE
#        error "Circular scroll is not supported in relative mode"
#    endif
/* To set a trackpad exclusively as scroll wheel: outer_ring_pct = 100, trigger_px = 0, trigger_ang = 0 */
static circular_scroll_context_t scroll = {.config = {.outer_ring_pct = 33,
                                                      .trigger_px     = 16,
                                                      .trigger_ang    = 9102, /* 50 degrees */
                                                      .wheel_clicks   = 18}};

static inline uint16_t atan2_16(int32_t dy, int32_t dx) {
    if (dy == 0) {
        if (dx >= 0) {
            return 0;
        } else {
            return 32768;
        }
    }

    int32_t abs_y = dy > 0 ? dy : -dy;
    int16_t a;

    if (dx >= 0) {
        a = 8192 - (8192 * (dx - abs_y) / (dx + abs_y));
    } else {
        a = 24576 - (8192 * (dx + abs_y) / (abs_y - dx));
    }

    if (dy < 0) {
        return -a; // negate if in quad III or IV
    }
    return a;
}

static circular_scroll_t circular_scroll(pinnacle_data_t touchData) {
    circular_scroll_t report = {0, 0, false};
    int8_t            x, y, wheel_clicks;
    uint8_t           center = INT8_MAX, mag;
    int16_t           ang, dot, det, opposite_side, adjacent_side;
    uint16_t          scale = cirque_pinnacle_get_scale();

    if (touchData.zValue) {
        /*
         * Place origin at center of trackpad, treat coordinates as vectors.
         * Scale to +/-INT8_MAX; angles are independent of resolution.
         */
        if (scale) {
            /* Rotate coordinates into a consistent orientation */
            report_mouse_t rot = {.x = (int8_t)((int32_t)touchData.xValue * INT8_MAX * 2 / scale - center), .y = (int8_t)((int32_t)touchData.yValue * INT8_MAX * 2 / scale - center)};
#    if defined(SPLIT_POINTING_ENABLE) && defined(POINTING_DEVICE_COMBINED)
            if (!is_keyboard_left()) {
                rot = pointing_device_adjust_by_defines_right(rot);
            } else
#    endif
            {
                rot = pointing_device_adjust_by_defines(rot);
            }
            x = rot.x;
            y = rot.y;
        } else {
            x = 0;
            y = 0;
        }

        /* Check if first touch */
        if (!scroll.z) {
            report.suppress_touch = false;
            /* Check if touch falls within outer ring */
            mag = sqrt16(x * x + y * y);
            if (mag * 100 / center >= 100 - scroll.config.outer_ring_pct) {
                scroll.state = SCROLL_DETECTING;
                scroll.x     = x;
                scroll.y     = y;
                scroll.mag   = mag;
                /*
                 * Decide scroll axis:
                 *   Vertical if started from righ half
                 *   Horizontal if started from left half
                 * Flipped for left-handed
                 */
                scroll.axis = x < 0;
            }
        } else if (scroll.state == SCROLL_DETECTING) {
            report.suppress_touch = true;
            /* Already detecting scroll, check movement from touchdown location */
            mag = sqrt16((x - scroll.x) * (x - scroll.x) + (y - scroll.y) * (y - scroll.y));
            if (mag >= scroll.config.trigger_px) {
                /*
                 * Find angle of movement.
                 * 0 degrees here means movement towards center of circle
                 */
                dot           = scroll.x * x + scroll.y * y;
                det           = scroll.x * y - scroll.y * x;
                opposite_side = abs(det);                                /* Based on scalar rejection */
                adjacent_side = abs(scroll.mag * scroll.mag - abs(dot)); /* Based on scalar projection */
                ang           = (int16_t)atan2_16(opposite_side, adjacent_side);
                if (ang < scroll.config.trigger_ang) {
                    /* Not a scroll, release coordinates */
                    report.suppress_touch = false;
                    scroll.state          = NOT_SCROLL;
                } else {
                    /* Scroll detected */
                    scroll.state = SCROLL_VALID;
                }
            }
        }
        if (scroll.state == SCROLL_VALID) {
            report.suppress_touch = true;
            dot                   = scroll.x * x + scroll.y * y;
            det                   = scroll.x * y - scroll.y * x;
            ang                   = (int16_t)atan2_16(det, dot);
            wheel_clicks          = ((int32_t)ang * scroll.config.wheel_clicks) / 65536;
            if (wheel_clicks >= 1 || wheel_clicks <= -1) {
                if (scroll.config.left_handed) {
                    if (scroll.axis == 0) {
                        report.h = -wheel_clicks;
                    } else {
                        report.v = wheel_clicks;
                    }
                } else {
                    if (scroll.axis == 0) {
                        report.v = -wheel_clicks;
                    } else {
                        report.h = wheel_clicks;
                    }
                }
                scroll.x = x;
                scroll.y = y;
            }
        }
    }

    scroll.z = touchData.zValue;
    if (!scroll.z) scroll.state = SCROLL_UNINITIALIZED;

    return report;
}

void cirque_pinnacle_enable_circular_scroll(bool enable) {
    features.circular_scroll_enable = enable;
}

void cirque_pinnacle_configure_circular_scroll(uint8_t outer_ring_pct, uint8_t trigger_px, uint16_t trigger_ang, uint8_t wheel_clicks, bool left_handed) {
    scroll.config.outer_ring_pct = outer_ring_pct;
    scroll.config.trigger_px     = trigger_px;
    scroll.config.trigger_ang    = trigger_ang;
    scroll.config.wheel_clicks   = wheel_clicks;
    scroll.config.left_handed    = left_handed;
}
#endif

bool cirque_pinnacle_gestures(report_mouse_t* mouse_report, pinnacle_data_t touchData) {
    bool suppress_mouse_update = false;

#ifdef CIRQUE_PINNACLE_CIRCULAR_SCROLL_ENABLE
#    if !CIRQUE_PINNACLE_POSITION_MODE
#        error "Circular scroll is not supported in relative mode"
#    endif
    circular_scroll_t scroll_report;
    if (features.circular_scroll_enable) {
        scroll_report         = circular_scroll(touchData);
        mouse_report->v       = scroll_report.v;
        mouse_report->h       = scroll_report.h;
        suppress_mouse_update = scroll_report.suppress_touch;
    }
#endif

#if defined(CIRQUE_PINNACLE_TAP_ENABLE) && CIRQUE_PINNACLE_POSITION_MODE
    if (features.tap_enable) {
        *mouse_report = trackpad_tap(*mouse_report, touchData);
    }
#endif

    return suppress_mouse_update;
}
