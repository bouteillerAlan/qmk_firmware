/*
Copyright 2019 @foostan
Copyright 2020 Drashna Jaelre <@drashna>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "quantum.h"
#include "graph.h"

#ifdef SWAP_HANDS_ENABLE
__attribute__((weak)) const keypos_t PROGMEM hand_swap_config[MATRIX_ROWS][MATRIX_COLS] = {
    // Left
    {{0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}},
    {{0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5}, {5, 5}},
    {{0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}},
    {{0, 7}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}},
    // Right
    {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}},
    {{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}},
    {{0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}},
    {{0, 3}, {1, 3}, {2, 3}, {3, 3}, {4, 3}, {5, 3}}
};
#endif

#ifdef OLED_ENABLE

#define ANIM_SIZE 352
#define IDLE_FRAMES 35
#define ANI_BYTE_SIZE 8
#define COPY_BIT(dest, id, src, is) dest = (( dest & ~(1<<id) ) | ((src & (1<<is))>>is) << id );

void render_spacer(void) {
	oled_write_ln_P(PSTR("\n"), false);
}

oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    } else {
		return OLED_ROTATION_270;
	}
    return rotation;
}

static void oled_render_layer_state(void) {
	static const char PROGMEM default_layer[] = {
        0x20, 0x94, 0x95, 0x96, 0x20,
        0x20, 0xb4, 0xb5, 0xb6, 0x20,
        0x20, 0xd4, 0xd5, 0xd6, 0x20, 0};
    static const char PROGMEM raise_layer[] = {
        0x20, 0x97, 0x98, 0x99, 0x20,
        0x20, 0xb7, 0xb8, 0xb9, 0x20,
        0x20, 0xd7, 0xd8, 0xd9, 0x20, 0};
    static const char PROGMEM lower_layer[] = {
        0x20, 0x9a, 0x9b, 0x9c, 0x20,
        0x20, 0xba, 0xbb, 0xbc, 0x20,
        0x20, 0xda, 0xdb, 0xdc, 0x20, 0};
    static const char PROGMEM adjust_layer[] = {
        0x20, 0x9d, 0x9e, 0x9f, 0x20,
        0x20, 0xbd, 0xbe, 0xbf, 0x20,
        0x20, 0xdd, 0xde, 0xdf, 0x20, 0};

    switch (get_highest_layer(layer_state)) {
        case 1:
            oled_write_P(default_layer, false);
            break;
        case 2:
            oled_write_P(raise_layer, false);
            break;
        case 3:
            oled_write_P(adjust_layer, false);
            break;
        default:
            oled_write_P(lower_layer, false);
            break;
    }
}

static void oled_render_wpm(void) {
	oled_write_ln_P(PSTR("WPM"), false);
    oled_write(get_u8_str(get_current_wpm(), '0'), false);
}

char     key_name = ' ';
uint16_t last_keycode;
uint8_t  last_row;
uint8_t  last_col;

static const char PROGMEM code_to_name[60] = {
    ' ', ' ', ' ', ' ', 'a',
    'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k',
    'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u',
    'v', 'w', 'x', 'y', 'z',
    '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0',
    'R', 'E', 'B', 'T', '_',
    '-', '=', '[', ']', '\\',
    '#', ';', '\'', '`', ',',
    '.', '/', ' ', ' ', ' '
};

static void set_keylog(uint16_t keycode, keyrecord_t *record) {
    // save the row and column (useful even if we can't find a keycode to show)
    last_row = record->event.key.row;
    last_col = record->event.key.col;

    key_name     = ' ';
    last_keycode = keycode;
    if (IS_QK_MOD_TAP(keycode)) {
        if (record->tap.count) {
            keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
        } else {
            keycode = 0xE0 + biton(QK_MOD_TAP_GET_MODS(keycode) & 0xF) + biton(QK_MOD_TAP_GET_MODS(keycode) & 0x10);
        }
    } else if (IS_QK_LAYER_TAP(keycode) && record->tap.count) {
        keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
    } else if (IS_QK_MODS(keycode)) {
        keycode = QK_MODS_GET_BASIC_KEYCODE(keycode);
    } else if (IS_QK_ONE_SHOT_MOD(keycode)) {
        keycode = 0xE0 + biton(QK_ONE_SHOT_MOD_GET_MODS(keycode) & 0xF) + biton(QK_ONE_SHOT_MOD_GET_MODS(keycode) & 0x10);
    }
    if (keycode > ARRAY_SIZE(code_to_name)) {
        return;
    }

    // update keylog
    key_name = pgm_read_byte(&code_to_name[keycode]);
}

// depad_str("###exemple", '#') >>> exemple
static const char *depad_str(const char *depad_str, char depad_char) {
    while (*depad_str == depad_char)
        ++depad_str;
    return depad_str;
}

static void oled_render_keypos(void) {
    oled_write_char('0' + last_row, false);
    oled_write_P(PSTR("x"), false);
    oled_write_char('0' + last_col, false);
}

static void oled_render_keylog(void) {
    oled_write_P(PSTR("k"), false);
    const char *last_keycode_str = get_u16_str(last_keycode, ' ');
    oled_write(depad_str(last_keycode_str, ' '), false);
    oled_write_P(PSTR(":"), false);
    oled_write_char(key_name, false);
}

static const char PROGMEM frame[] = {
	0x2F, 0x17, 0x4B, 0xA5, 0x43, 0x01, 0x02, 0x01, 0x00, 0x81, 0x80, 0x80, 0xC0, 0xC0, 0xC0,
	0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x90, 0x80, 0x81, 0x00, 0x01, 0x02, 0x01, 0x03, 0x05, 0x8B,
	0x17, 0x2F, 0x00, 0x00, 0x80, 0xE0, 0xF8, 0x9C, 0xC6, 0x66, 0x3F, 0x1B, 0x19, 0x09, 0x1F,
	0x17, 0x3F, 0x3F, 0x1F, 0x0F, 0x0F, 0x0F, 0x0F, 0x1D, 0xF9, 0xFB, 0xFE, 0x9E, 0x1C, 0xF8,
	0xE0, 0x80, 0x00, 0x00, 0x00, 0xFC, 0x87, 0x03, 0x01, 0x01, 0x0F, 0x04, 0x80, 0x80, 0x80,
	0x80, 0x40, 0x30, 0x3F, 0xBE, 0x86, 0xC4, 0xC4, 0xE6, 0xF8, 0xF1, 0xE0, 0xE0, 0xF0, 0xF9,
	0xAF, 0xCF, 0xFF, 0xFF, 0xFC, 0x00, 0x28, 0x10, 0x07, 0x1E, 0x30, 0x60, 0xC2, 0xC0, 0x82,
	0x07, 0x01, 0x13, 0x0C, 0x1C, 0x9D, 0xFC, 0xE7, 0x81, 0xC2, 0x83, 0x8F, 0xDF, 0xFF, 0xFF,
	0x7F, 0xBF, 0x7F, 0x27, 0x1F, 0x07, 0x00, 0x00, 0x50, 0xA0, 0x40, 0x82, 0x00, 0x00, 0x00,
	0x01, 0x03, 0x03, 0x04, 0x04, 0x45, 0x05, 0x0E, 0x0C, 0x0C, 0x0F, 0x07, 0x07, 0x05, 0x05,
	0x02, 0x83, 0x01, 0x10, 0x28, 0x10, 0x00, 0x80, 0x40, 0xA0, 0x2F, 0x4E, 0x5D, 0xBA, 0xBD,
	0x7A, 0x74, 0x78, 0xF4, 0xE8, 0xF4, 0xE8, 0xF0, 0xE8, 0xD0, 0xE8, 0xD0, 0xE8, 0xD0, 0xE8,
	0xF0, 0xE8, 0xF4, 0xE8, 0x74, 0x78, 0x74, 0xBA, 0xB5, 0x5A, 0x4D, 0x2E, 0x00, 0x08, 0x00,
	0x00, 0x00, 0x81, 0x01, 0x01, 0x02, 0x02, 0x02, 0x42, 0x05, 0x05, 0x05, 0x05, 0x25, 0x05,
	0x05, 0x05, 0x02, 0x02, 0x22, 0x02, 0x81, 0x01, 0x11, 0x10, 0x28, 0xC6, 0x28, 0x10, 0x08,
	0x40, 0x00, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x80,
	0x40, 0x80, 0x00, 0x04, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
	0x00, 0x60, 0x60, 0x70, 0x68, 0x70, 0x60, 0x60, 0x62, 0x60, 0x60, 0x60, 0x60, 0x60, 0x68,
	0x60, 0x60, 0x61, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x61,
	0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t cumsum_inds[] = {
	0b00011001, 0b10000100, 0b11101011, 0b01001101, 0b11101001, 0b00100010, 0b00001010, 0b00110000, 0b10000100, 0b10000010, 0b00011010, 0b01000110, 0b01011000, 0b00011000, 0b10000101,
	0b10010101, 0b11011110, 0b10110011, 0b00100000, 0b01001001, 0b00101010, 0b00000000,
};

static const  uint8_t change_inds[] = {
	0b00000111, 0b10100000, 0b01011101, 0b01000000, 0b00100000, 0b00011000, 0b00010000, 0b11111110, 0b10010010, 0b01101011, 0b00000000, 0b11011101, 0b01000000, 0b00110000, 0b11101000,
	0b11111000, 0b01111110, 0b01010111, 0b00101100, 0b00010110, 0b01001110, 0b00100111, 0b00100011, 0b10011011, 0b00010101, 0b11001000, 0b11100101, 0b01110011, 0b00000000, 0b10000000,
	0b01100000, 0b01000001, 0b11110000, 0b11111100, 0b10101110, 0b01011000, 0b00101100, 0b10011100, 0b01001110, 0b01000111, 0b00110100, 0b01001010, 0b00101001, 0b00010110, 0b10001100,
	0b01000110, 0b10100011, 0b10010001, 0b11101010, 0b10000101, 0b01001010, 0b10101001, 0b11001010, 0b01110001, 0b00111001, 0b00011100, 0b11010001, 0b00101000, 0b10100100, 0b01011010,
	0b00110001, 0b00011010, 0b10001110, 0b01000111, 0b10101001, 0b10010100, 0b11101010, 0b10000101, 0b01001010, 0b10101101, 0b10001000, 0b11000101, 0b01100011, 0b00110001, 0b11010001,
	0b01001000, 0b10110101, 0b00110010, 0b10011101, 0b01010000, 0b10101001, 0b01010101, 0b00101010, 0b11011000, 0b10001100, 0b01010110, 0b00110011, 0b00011101, 0b10101100, 0b11100101,
	0b10001110, 0b00110001, 0b00011000, 0b10101100, 0b01100110, 0b00111011, 0b01101001, 0b10111010, 0b11100100, 0b01110010, 0b10111001, 0b10011000, 0b10001100, 0b01010110, 0b00111100,
	0b01110000, 0b00111010, 0b10000011, 0b01101101, 0b10110111, 0b00011011, 0b10101101, 0b11100110, 0b11111100, 0b01101001, 0b10110110, 0b11011100, 0b01101110, 0b10110111, 0b10011011,
	0b11101000, 0b00110110, 0b11010011, 0b01110110, 0b00001111, 0b00001101, 0b01001100, 0b10100110, 0b10010011, 0b01110000, 0b11010100, 0b11010011, 0b10000101, 0b10110000, 0b11110101,
	0b10000011, 0b10000101, 0b00011101, 0b11001110, 0b11110111, 0b10000011, 0b11000101, 0b11100101, 0b00001111, 0b10001000, 0b01000100, 0b01010011, 0b01001101, 0b10000111, 0b00001010,
	0b00000101, 0b00110010, 0b10011010, 0b01001101, 0b10111101, 0b01000001, 0b11100001, 0b01000000, 0b11101010, 0b00000101, 0b10010110, 0b11010110, 0b01101101, 0b00110111, 0b01011101,
	0b11001110, 0b11110111, 0b10000011, 0b11000101, 0b11100101, 0b00001111, 0b10001000, 0b01000100, 0b01000001, 0b11101000, 0b11000110, 0b10110011, 0b01101001, 0b10111010, 0b11100001,
	0b00001010, 0b00000111, 0b01010001, 0b10001100, 0b10110000, 0b00010000, 0b00001100, 0b00001000, 0b00010100, 0b01000110, 0b00100000, 0b11010011, 0b00001001, 0b10010100, 0b11010010,
	0b11001001, 0b01100110, 0b10110100, 0b01011010, 0b10101101, 0b10010110, 0b11101011, 0b10000110, 0b01101011, 0b00111001, 0b10011110, 0b11010000, 0b01110000, 0b00111000, 0b01100001,
	0b10100000, 0b00100000, 0b00011000, 0b00010001, 0b00110000, 0b10011001, 0b01001101, 0b00101100, 0b11010110, 0b10001011, 0b01010101, 0b10110010, 0b11011101, 0b01110000, 0b11001010,
	0b01100101, 0b10110011, 0b00011001, 0b10101100, 0b11100111, 0b00000011, 0b10100001, 0b11010010, 0b11101010, 0b01110101, 0b11000011, 0b01000010, 0b10001000, 0b00110101, 0b10010011,
	0b00101001, 0b10010110, 0b11001100, 0b01100110, 0b10110011, 0b10011001, 0b11101101, 0b00000111, 0b00111011, 0b10100001, 0b11010010, 0b11101010, 0b01110101, 0b10000101, 0b00010001,
	0b10001101, 0b00000111, 0b00110011, 0b10011101, 0b11010000, 0b11101001, 0b01110101, 0b00111010, 0b11100000, 0b10110001, 0b10100011, 0b11111010, 0b01001001, 0b10100000, 0b11100110,
	0b01110100, 0b01000001, 0b01001111, 0b11101001, 0b00100111, 0b01111011, 0b11000001, 0b11100010, 0b11110101, 0b10001000, 0b01000110, 0b10001111, 0b11101111, 0b00110000, 0b10100010,
	0b00110001, 0b10110000, 0b11101111, 0b01111000, 0b00111100, 0b01011110, 0b01101111, 0b01011000, 0b10000000, 0b01010001, 0b00011000, 0b11010110, 0b01101101, 0b00110111, 0b01000001,
	0b11101000, 0b00010100, 0b10010011, 0b01011001, 0b10110000, 0b11011010, 0b01101110, 0b11000110, 0b10000000,
};

static const char PROGMEM change_vals[] = {
	0xC4, 0xA4, 0x00, 0x0B, 0x45, 0x03, 0x20, 0x27, 0x02, 0x05, 0x10, 0x45, 0x0B, 0x80, 0x40,
	0xF0, 0xF8, 0xFD, 0x99, 0xCE, 0x87, 0x01, 0x00, 0x01, 0x00, 0x4B, 0xA5, 0x43, 0x00, 0x00,
	0xE0, 0xF0, 0xF9, 0xF1, 0xFA, 0x9B, 0x83, 0xC4, 0xF4, 0x7D, 0x1F, 0x0F, 0x0F, 0xFE, 0xEF,
	0xF7, 0x00, 0x81, 0xC2, 0x83, 0x03, 0x84, 0xC4, 0x45, 0x05, 0x0E, 0x0C, 0xF4, 0xF8, 0xFC,
	0xFE, 0xE9, 0x38, 0x3F, 0x3F, 0x07, 0x04, 0x04, 0x74, 0x78, 0xF4, 0xE8, 0xF4, 0xE8, 0x30,
	0x79, 0x31, 0x01, 0x22, 0x01, 0x60, 0x48, 0x91, 0x01, 0x21, 0x01, 0x44, 0x01, 0x02, 0x01,
	0x00, 0x81, 0x01, 0x61, 0x8B, 0x80, 0x00, 0x10, 0x38, 0x10, 0x00, 0x64, 0x10, 0x28, 0x44,
	0x28, 0x10, 0x82, 0x11, 0xC6, 0x60, 0x60, 0x00, 0x10, 0x00, 0x68, 0x00, 0x00, 0x01, 0x00,
	0x62, 0x80, 0x80, 0x40, 0x20, 0x40, 0x80, 0x61, 0x62, 0x61, 0x10, 0x81, 0x40, 0xA0, 0x10,
	0x28, 0x10, 0x01, 0xC0, 0x90, 0x0B, 0xA4, 0x02, 0x02, 0x01, 0x44, 0x00, 0x80, 0x40, 0x80,
	0x00, 0x60, 0x61, 0x60, 0xC4, 0x05, 0x22, 0x11, 0xC6, 0x00, 0x80, 0x8B, 0x45, 0x42, 0x0B,
	0x45, 0x03, 0x90, 0x05, 0x80, 0x81, 0x50, 0x68, 0xF0, 0xF8, 0xF8, 0xEC, 0xF6, 0xEB, 0x75,
	0x1D, 0x0D, 0x07, 0x27, 0x00, 0x40, 0x60, 0x4B, 0xA5, 0x43, 0x01, 0x10, 0x28, 0xE8, 0xF0,
	0xE8, 0xF4, 0xE8, 0x74, 0x82, 0xE2, 0x75, 0x3D, 0x0F, 0x08, 0x1C, 0x07, 0x13, 0x01, 0x68,
	0x80, 0x82, 0xD0, 0x02, 0x42, 0x05, 0x05, 0x05, 0x05, 0x25, 0x60, 0x38, 0x1C, 0x1C, 0x02,
	0x90, 0x45, 0x05, 0x41, 0x00, 0x80, 0x00, 0x10, 0x00, 0x61, 0x60, 0x00, 0x07, 0x25, 0x01,
	0x00, 0x60, 0x20, 0x27, 0x00, 0x80, 0x00, 0x00, 0x60, 0x64, 0x00, 0x00, 0x80, 0x05, 0x01,
	0x80, 0x40, 0x80, 0x04, 0x01, 0x61, 0x90, 0x45, 0x02, 0x01, 0x44, 0xC0, 0xA0, 0x07, 0x22,
	0x81, 0x11, 0xC6, 0x60,
};

static uint16_t get_num(const uint8_t* arr, int bitsize, int index){
	int arr_index = ((bitsize*index)/ANI_BYTE_SIZE);
	int byte_index = 7-(((bitsize*index) % ANI_BYTE_SIZE));
	uint16_t res = 0;
	for(int i = bitsize-1;i >= 0; i--){
		COPY_BIT(res, i, arr[arr_index], byte_index);
		byte_index--;
		if(byte_index < 0){
			byte_index = 7;
			arr_index++;
		}
	}
	return res;
}

uint16_t index_start = 0;
uint16_t index_end = 0;

static void change_frame_bytewise(uint8_t frame_number){
	index_start = frame_number == 0 ? 0 : index_end;
	index_end = index_start + get_num(cumsum_inds, 5, frame_number);
	if (index_start != index_end){
		for (uint16_t i=index_start; i < index_end; i++){
			oled_write_raw_byte(pgm_read_byte(change_vals + i), get_num(change_inds, 9, i));
		}
	}
}

static int c_frame = 0;
bool first_render = true;

static void render_anim(void) {
    if (first_render) {
        oled_write_raw_P( frame, ANIM_SIZE);
        first_render = 0;
    } else {
        change_frame_bytewise(c_frame);
    }
    c_frame = c_frame+1 > IDLE_FRAMES ? 0 : c_frame+1;
}

// Function to check if Caps Lock is active
bool is_caps_lock_active(void) {
    return (host_keyboard_led_state().caps_lock);
}

// Function to check if Num Lock is active
bool is_num_lock_active(void) {
    return (host_keyboard_led_state().num_lock);
}

static void show_keystatus(void) {
    static const char PROGMEM caps_lock[] = {0x20, 0x20, 0x7F, 0x20, 0x20, 0};
    static const char PROGMEM num_lock[] = {0x20, 0x20, 0x23, 0x20, 0x20, 0};
    static const char PROGMEM caps_num_lock[] = {0x20, 0x7F, 0x23, 0x20, 0x20, 0};
    static const char PROGMEM void_line[] = {0x20, 0x20, 0x20, 0x20, 0x20, 0};

    oled_write_P(void_line, false);
    if (is_caps_lock_active() && is_num_lock_active()) oled_write_P(caps_num_lock, false);
    else if (is_caps_lock_active()) oled_write_P(caps_lock, false);
    else if (is_num_lock_active()) oled_write_P(num_lock, false);
    else oled_write_P(void_line, false);
}

bool no_anim = false;
void suspend_power_down_user(void) {
    // code will run multiple times while keyboard is suspended
    no_anim = true;
}

bool oled_task_kb(void) {
    if (!oled_task_user()) {
        return false;
    }
    if (is_keyboard_master()) {
		oled_render_layer_state();
        show_keystatus();
        render_spacer();
		oled_render_wpm();
		render_spacer();
        oled_render_keypos();
        render_spacer();
        oled_render_keylog();
        render_spacer();
    } else {
        if (!no_anim) {
            render_anim();
            render_wpm_graph();
        }
    }
    return false;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        set_keylog(keycode, record);
    }
    return process_record_user(keycode, record);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_LCTL:
        case KC_RCTL:
            break;
    }
    return true;
}
#endif // OLED_ENABLE
