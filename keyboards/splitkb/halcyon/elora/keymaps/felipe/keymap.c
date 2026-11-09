// Felipe (macOS, ABNT2 OS layout) + wife (Windows, French AZERTY OS layout) on one Halcyon Elora v2.
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "keymap_french.h"

enum layers {
    _MAC = 0, // Felipe: plain KC_ codes, macOS runs ABNT2
    _MSYM,
    _NAV,
    _WIN,     // wife: keycaps are the source of truth, OS stays French AZERTY
    _WSYM,    // wife: accented letters (Alt-codes) and misc symbols
    _WNAV,
    _ADJUST,  // must be the highest layer so it stays reachable from both profiles
};

#define MSYM MO(_MSYM)
#define NAV  MO(_NAV)
#define ADJ  MO(_ADJUST)
#define WSYM MO(_WSYM)
#define WNAV MO(_WNAV)

// ---------------------------------------------------------------------------
// Wife profile. The OS is French AZERTY, but the keycaps are QWERTY/US-style.
// Each FRK_* key sends whatever produces the legend printed on the keycap
// (unshifted, or shifted when Shift is held), regardless of AZERTY quirks.
// ALK_* keys type Portuguese accented letters via Windows Alt+numpad codes
// (Windows-1252), which work with any layout. Shift gives the capital.
// ---------------------------------------------------------------------------
enum custom_keycodes {
    FRK_1 = SAFE_RANGE, FRK_2, FRK_3, FRK_4, FRK_5, FRK_6, FRK_7, FRK_8, FRK_9, FRK_0,
    FRK_MINS, FRK_EQL, FRK_LBRC, FRK_RBRC, FRK_BSLS, FRK_SCLN, FRK_QUOT,
    FRK_COMM, FRK_DOT, FRK_SLSH, FRK_GRV, FRK_TILD,
    FRK_LAST = FRK_TILD,
    ALK_AACU, ALK_EACU, ALK_IACU, ALK_OACU, ALK_UACU,
    ALK_ATIL, ALK_OTIL, ALK_ACIR, ALK_ECIR, ALK_OCIR,
    ALK_AGRV, ALK_CCED,
    ALK_LAST = ALK_CCED,
};

typedef struct {
    uint16_t plain;
    uint16_t shifted;
} fr_pair_t;

static const fr_pair_t fr_pairs[] = {
    [FRK_1    - FRK_1] = {FR_1,    FR_EXLM},
    [FRK_2    - FRK_1] = {FR_2,    FR_AT},
    [FRK_3    - FRK_1] = {FR_3,    FR_HASH},
    [FRK_4    - FRK_1] = {FR_4,    FR_DLR},
    [FRK_5    - FRK_1] = {FR_5,    FR_PERC},
    [FRK_6    - FRK_1] = {FR_6,    FR_CIRC},
    [FRK_7    - FRK_1] = {FR_7,    FR_AMPR},
    [FRK_8    - FRK_1] = {FR_8,    FR_ASTR},
    [FRK_9    - FRK_1] = {FR_9,    FR_LPRN},
    [FRK_0    - FRK_1] = {FR_0,    FR_RPRN},
    [FRK_MINS - FRK_1] = {FR_MINS, FR_UNDS},
    [FRK_EQL  - FRK_1] = {FR_EQL,  FR_PLUS},
    [FRK_LBRC - FRK_1] = {FR_LBRC, FR_LCBR},
    [FRK_RBRC - FRK_1] = {FR_RBRC, FR_RCBR},
    [FRK_BSLS - FRK_1] = {FR_BSLS, FR_PIPE},
    [FRK_SCLN - FRK_1] = {FR_SCLN, FR_COLN},
    [FRK_QUOT - FRK_1] = {FR_QUOT, FR_DQUO},
    [FRK_COMM - FRK_1] = {FR_COMM, FR_LABK},
    [FRK_DOT  - FRK_1] = {FR_DOT,  FR_RABK},
    [FRK_SLSH - FRK_1] = {FR_SLSH, FR_QUES},
    [FRK_GRV  - FRK_1] = {FR_GRV,  FR_TILD},
    [FRK_TILD - FRK_1] = {FR_TILD, FR_CIRC},
};

// Alt codes (Windows-1252): lower / upper
static const char *const alt_codes[][2] = {
    [ALK_AACU - ALK_AACU] = {"0225", "0193"}, // á Á
    [ALK_EACU - ALK_AACU] = {"0233", "0201"}, // é É
    [ALK_IACU - ALK_AACU] = {"0237", "0205"}, // í Í
    [ALK_OACU - ALK_AACU] = {"0243", "0211"}, // ó Ó
    [ALK_UACU - ALK_AACU] = {"0250", "0218"}, // ú Ú
    [ALK_ATIL - ALK_AACU] = {"0227", "0195"}, // ã Ã
    [ALK_OTIL - ALK_AACU] = {"0245", "0213"}, // õ Õ
    [ALK_ACIR - ALK_AACU] = {"0226", "0194"}, // â Â
    [ALK_ECIR - ALK_AACU] = {"0234", "0202"}, // ê Ê
    [ALK_OCIR - ALK_AACU] = {"0244", "0212"}, // ô Ô
    [ALK_AGRV - ALK_AACU] = {"0224", "0192"}, // à À
    [ALK_CCED - ALK_AACU] = {"0231", "0199"}, // ç Ç
};

static void send_alt_code(const char *digits) {
    register_code(KC_LALT);
    for (; *digits; digits++) {
        tap_code(*digits == '0' ? KC_KP_0 : KC_KP_1 + (*digits - '1'));
    }
    unregister_code(KC_LALT);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return true;

    if (keycode >= FRK_1 && keycode <= FRK_LAST) {
        const fr_pair_t *p = &fr_pairs[keycode - FRK_1];
        uint8_t mods    = get_mods();
        bool    shifted = mods & MOD_MASK_SHIFT;
        del_mods(MOD_MASK_SHIFT);
        tap_code16(shifted ? p->shifted : p->plain);
        // ^ (shifted 6) and ` (plain grave) are dead keys on French layout
        if ((keycode == FRK_6 && shifted) || (keycode == FRK_GRV && !shifted) || (keycode == FRK_TILD && shifted)) tap_code(KC_SPC);
        set_mods(mods);
        return false;
    }

    if (keycode >= ALK_AACU && keycode <= ALK_LAST) {
        uint8_t mods    = get_mods();
        bool    shifted = mods & MOD_MASK_SHIFT;
        clear_mods();
        send_alt_code(alt_codes[keycode - ALK_AACU][shifted]);
        set_mods(mods);
        return false;
    }
    return true;
}

// clang-format off

// Layer diagrams below use short names instead of KC_* codes. Layout of each diagram:
//   rows 1-3: 6 keys per half | row 4: 6 keys, 4 inner keys, 6 keys | last row: 5 thumb keys per half.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * MAC base: Felipe, macOS running the ABNT2 layout. Glyphs shown are what ABNT2 types.
 * (blank = same as the layer below, xx = does nothing)
 *
 * |  ' "  |   1   |   2   |   3   |   4   |   5   |                               |   6   |   7   |   8   |   9   |   0   |  - _  |
 * |  Tab  |   Q   |   W   |   E   |   R   |   T   |                               |   Y   |   U   |   I   |   O   |   P   |  Bksp |
 * | Shift |   A   |   S   |   D   |   F   |   G   |                               |   H   |   J   |   K   |   L   |   ç   |   ~   |
 * |  Ctrl |   Z   |   X   |   C   |   V   |   B   |  SYM  |   ↑   |   ←   |  NAV  |   N   |   M   |   ,   |   .   |  ; :  |  / ?  |
 *          |  Esc  |  Alt  |  Cmd  | Space |   ↓   |                |   →   | Enter |  ADJ  |  ´ `  |  \ |  |
 */
    [_MAC] = LAYOUT(
        KC_GRV,  KC_1,   KC_2,    KC_3,    KC_4,   KC_5,                                       KC_6,    KC_7,    KC_8,    KC_9,   KC_0,    KC_MINS,
        KC_TAB,  KC_Q,   KC_W,    KC_E,    KC_R,   KC_T,                                       KC_Y,    KC_U,    KC_I,    KC_O,   KC_P,    KC_BSPC,
        KC_LSFT, KC_A,   KC_S,    KC_D,    KC_F,   KC_G,                                       KC_H,    KC_J,    KC_K,    KC_L,   KC_SCLN, KC_QUOT,
        KC_LCTL, KC_Z,   KC_X,    KC_C,    KC_V,   KC_B,  MSYM,   KC_RIGHT,        KC_UP, NAV , KC_N,    KC_M,    KC_COMM, KC_DOT, KC_SLSH, KC_INT1,
                                KC_ESC, KC_LALT, KC_LGUI, KC_SPC, KC_LEFT,    KC_DOWN, KC_ENT, ADJ, KC_LBRC, KC_NUBS
    ),

/*
 * MAC SYM (hold inner key "SYM"): numpad, symbols, arrows. Glyphs are what ABNT2 types.
 * (blank = same as the layer below, xx = does nothing)
 *
 * |   F1  |   F2  |   F3  |   F4  |   F5  |   {   |                               |   }   |   7   |   8   |   9   |   +   |  - _  |
 * |       |   !   |   @   |   #   |   $   |   (   |                               |   )   |   4   |   5   |   6   |   *   |       |
 * |       |   %   |   ¨   |   &   |   *   |  [ {  |                               |  ] }  |   1   |   2   |   3   |   ↑   |       |
 * |       |   =   |  - _  |   <   |   >   |  ] }  |       |       |       |       |  \ |  |   0   |   F8  |   ←   |   ↓   |   →   |
 *          |       |       |       |       |       |                |       |  ADJ  |       |       |       |
 */
    [_MSYM] = LAYOUT(
        KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_RCBR,                                           KC_PIPE, KC_7, KC_8,    KC_9,    KC_PLUS, KC_MINS,
        _______, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_LPRN,                                           KC_RPRN, KC_4, KC_5,    KC_6,    KC_ASTR, _______,
        _______, KC_PERC, KC_CIRC, KC_AMPR, KC_ASTR, KC_RBRC,                                           KC_BSLS, KC_1, KC_2,    KC_3,    KC_UP,   _______,
        _______, KC_EQL,  KC_MINS, KC_LT,   KC_GT,   KC_BSLS,    _______, _______, _______, _______,    KC_NUBS, KC_0, KC_F8,   KC_LEFT, KC_DOWN, KC_RGHT,
                 _______, _______, _______, _______, _______,                                           _______, ADJ,  _______, _______, _______
    ),

/*
 * MAC NAV (hold inner key "NAV"): arrows on home row, Ctrl+Z/X/C/V, Home/End, Del.
 * (blank = same as the layer below, xx = does nothing)
 *
 * |       |       |       |       |       |       |                               |       |       |       |       |       |       |
 * |       |  Ins  | PrtSc |  Menu |   xx  |   xx  |                               |  PgUp |   ^←  |   ↑   |   ^→  | ^Bksp |   xx  |
 * |       |  Alt  |  Ctrl | Shift |   xx  |  Caps |                               |  PgDn |   ←   |   ↓   |   →   |  Del  |   xx  |
 * |       |   ^Z  |   ^X  |   ^C  |   ^V  |   xx  |       |       |       |       |   xx  |  Home |   xx  |  End  |   xx  |       |
 *          |       |       |       |  ADJ  |       |                |       |       |       |       |       |
 */
    [_NAV] = LAYOUT(
        _______, _______,    _______,    _______,    _______,    _______,                                           _______, _______,       _______, _______,       _______,       _______,
        _______, KC_INS,     KC_PSCR,    KC_APP,     KC_NO,      KC_NO,                                             KC_PGUP, LCTL(KC_LEFT), KC_UP,   LCTL(KC_RGHT), LCTL(KC_BSPC), KC_NO,
        _______, KC_LALT,    KC_LCTL,    KC_LSFT,    KC_NO,      KC_CAPS,                                           KC_PGDN, KC_LEFT,       KC_DOWN, KC_RGHT,       KC_DEL,        KC_NO,
        _______, LCTL(KC_Z), LCTL(KC_X), LCTL(KC_C), LCTL(KC_V), KC_NO,      _______, _______, _______, _______,    KC_NO,   KC_HOME,       KC_NO,   KC_END,        KC_NO,         _______,
                 _______,    _______,    _______,    ADJ,        _______,                                           _______, _______,       _______, _______,       _______
    ),

/*
 * ADJUST (hold right thumb "ADJ"): RGB, media, bootloader, and the profile switch =Mac / =Win.
 * (blank = same as the layer below, xx = does nothing)
 *
 * |  BOOT |       |       |       |       |       |                               |       |       |       |       |       |       |
 * |       |       |       |       |       |       |                               |       |       |       |       |       |       |
 * |       |  RGB  |  Hue+ |  Sat+ |  Bri+ |       |                               |       |  Vol- |  Mute |  Vol+ |       |       |
 * |       |  Mode |  Hue- |  Sat- |  Bri- |       |       |       |       |       |       |  Prev |  Play |  Next |       |       |
 *          |  =Mac |  =Win |       |       |       |                |       |       |       |       |       |
 */
    [_ADJUST] = LAYOUT(
        QK_BOOT, _______,   _______,   _______, _______, _______,                                           _______, _______, _______, _______, _______, _______,
        _______, _______,   _______,   _______, _______, _______,                                           _______, _______, _______, _______, _______, _______,
        _______, RM_TOGG,   RM_HUEU,   RM_SATU, RM_VALU, _______,                                           _______, KC_VOLD, KC_MUTE, KC_VOLU, _______, _______,
        _______, RM_NEXT,   RM_HUED,   RM_SATD, RM_VALD, _______,    _______, _______, _______, _______,    _______, KC_MPRV, KC_MPLY, KC_MNXT, _______, _______,
                 PDF(_MAC), PDF(_WIN), _______, _______, _______,                                           _______, _______, _______, _______, _______
    ),

/*
 * WIN base: wife, Windows on French AZERTY. Every key types its keycap legend; Shift = 2nd legend.
 * (blank = same as the layer below, xx = does nothing)
 *
 * |  ' "  |   1   |   2   |   3   |   4   |   5   |                               |   6   |   7   |   8   |   9   |   0   |  [ {  |
 * |  Tab  |   Q   |   W   |   E   |   R   |   T   |                               |   Y   |   U   |   I   |   O   |   P   |  Bksp |
 * | Shift |   A   |   S   |   D   |   F   |   G   |                               |   H   |   J   |   K   |   L   |   ç   |  ~ ^  |
 * |  Ctrl |   Z   |   X   |   C   |   V   |   B   |  SYM  |  NAV  |   ←   |   ↑   |   N   |   M   |  , <  |  . >  |  ; :  |  / ?  |
 *          |  Esc  |  Cmd  |  Alt  | Space |   ↓   |                |  ] }  | Enter |   →   |  ADJ  |  \ |  |
 */
    [_WIN] = LAYOUT(
        FRK_QUOT, FRK_1,  FRK_2,   FRK_3,   FRK_4,  FRK_5,                                    FRK_6,   FRK_7,   FRK_8,    FRK_9,   FRK_0,    FRK_LBRC,
        KC_TAB,   FR_Q,   FR_W,    FR_E,    FR_R,   FR_T,                                     FR_Y,    FR_U,    FR_I,     FR_O,    FR_P,     KC_BSPC,
        KC_LSFT,  FR_A,   FR_S,    FR_D,    FR_F,   FR_G,                                     FR_H,    FR_J,    FR_K,     FR_L,    ALK_CCED, FRK_TILD,
        KC_LCTL,  FR_Z,   FR_X,    FR_C,    FR_V,   FR_B,      WSYM, WNAV, KC_LEFT, KC_UP,    FR_N,    FR_M,    FRK_COMM, FRK_DOT, FRK_SCLN, FRK_SLSH,
                  KC_ESC, KC_LGUI, KC_LALT, KC_SPC, KC_DOWN,                                   FRK_RBRC, KC_ENT,  KC_RGHT,  ADJ,     FRK_BSLS
    ),

/*
 * WIN SYM (hold inner key "SYM"): Portuguese accents (Shift = capital), - and =, special symbols.
 * (blank = same as the layer below, xx = does nothing)
 *
 * |       |       |       |       |       |       |                               |       |       |       |       |       |  - _  |
 * |       |   €   |   °   |   §   |   ²   |   £   |                               |   á   |   é   |   í   |   ó   |   ú   |       |
 * |       |   ¤   |   µ   |   ¨   |   ù   |   è   |                               |   ã   |   õ   |   â   |   ê   |   ô   |       |
 * |       |  = +  |       |       |       |       |       |       |       |       |   à   |   ç   |       |       |       |       |
 *          |       |       |       |       |       |                |       |       |       |       |       |
 */
    [_WSYM] = LAYOUT(
        _______, _______, _______, _______, _______, _______,                                           _______,  _______,  _______,  _______,  _______,  FRK_MINS,
        _______, FR_EURO, FR_DEG,  FR_SECT, FR_SUP2, FR_PND,                                            ALK_AACU, ALK_EACU, ALK_IACU, ALK_OACU, ALK_UACU, _______,
        _______, FR_CURR, FR_MICR, FR_DIAE, FR_UGRV, FR_EGRV,                                           ALK_ATIL, ALK_OTIL, ALK_ACIR, ALK_ECIR, ALK_OCIR, _______,
        _______, FRK_EQL, _______, _______, _______, _______,    _______, _______, _______, _______,    ALK_AGRV, ALK_CCED, _______,  _______,  _______,  _______,
                 _______, _______, _______, _______, _______,                                           _______,  _______,  _______,  _______,  _______
    ),

/*
 * WIN NAV (hold inner key "NAV"): same as MAC NAV, with Ctrl+Z/X/C/V correct on AZERTY.
 * (blank = same as the layer below, xx = does nothing)
 *
 * |       |       |       |       |       |       |                               |       |       |       |       |       |       |
 * |       |  Ins  | PrtSc |  Menu |   xx  |   xx  |                               |  PgUp |   ^←  |   ↑   |   ^→  | ^Bksp |   xx  |
 * |       |  Alt  |  Ctrl | Shift |   xx  |  Caps |                               |  PgDn |   ←   |   ↓   |   →   |  Del  |   xx  |
 * |       |   ^Z  |   ^X  |   ^C  |   ^V  |   xx  |       |       |       |       |   xx  |  Home |   xx  |  End  |   xx  |       |
 *          |       |       |       |       |       |                |       |       |       |       |       |
 */
    [_WNAV] = LAYOUT(
        _______, _______,    _______,    _______,    _______,    _______,                                           _______, _______,       _______, _______,       _______,       _______,
        _______, KC_INS,     KC_PSCR,    KC_APP,     KC_NO,      KC_NO,                                             KC_PGUP, LCTL(KC_LEFT), KC_UP,   LCTL(KC_RGHT), LCTL(KC_BSPC), KC_NO,
        _______, KC_LALT,    KC_LCTL,    KC_LSFT,    KC_NO,      KC_CAPS,                                           KC_PGDN, KC_LEFT,       KC_DOWN, KC_RGHT,       KC_DEL,        KC_NO,
        _______, LCTL(FR_Z), LCTL(FR_X), LCTL(FR_C), LCTL(FR_V), KC_NO,      _______, _______, _______, _______,    KC_NO,   KC_HOME,       KC_NO,   KC_END,        KC_NO,         _______,
                 _______,    _______,    _______,    _______,    _______,                                           _______, _______,       _______, _______,       _______
    ),
};
// clang-format on

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_MAC]    = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_PGDN, KC_PGUP)},
    [_MSYM]   = {ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______)},
    [_NAV]    = {ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______)},
    [_ADJUST] = {ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(RM_HUED, RM_HUEU)},
    [_WIN]    = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_PGDN, KC_PGUP)},
    [_WSYM]   = {ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______)},
    [_WNAV]   = {ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______)},
};
#endif
