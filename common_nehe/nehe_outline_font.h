#ifndef NEHE_OUTLINE_FONT_H
#define NEHE_OUTLINE_FONT_H

#include "nehe_bitmap_font.h"

static inline int nehe_outline_font_pixel(const uint8_t *rows, int row, int col)
{
    if (!nehe_font_row_has_pixel(rows, row, col)) {
        return 0;
    }

    if (row == 0 || row == NEHE_FONT_ROWS - 1 || col == 0 || col == NEHE_FONT_COLS - 1) {
        return 1;
    }
    if (!nehe_font_row_has_pixel(rows, row - 1, col) ||
        !nehe_font_row_has_pixel(rows, row + 1, col) ||
        !nehe_font_row_has_pixel(rows, row, col - 1) ||
        !nehe_font_row_has_pixel(rows, row, col + 1)) {
        return 1;
    }
    return 0;
}

static inline float nehe_outline_text_width(const char *text, float cell)
{
    int chars = 0;

    if (text == 0) {
        return 0.0f;
    }
    while (*text++ != '\0') {
        ++chars;
    }
    return chars > 0 ? (float)chars * cell * 6.0f - cell : 0.0f;
}

#define NEHE_SYMBOL_ROWS 13
#define NEHE_SYMBOL_COLS 13

static inline int nehe_skull_symbol_pixel(int row, int col)
{
    static const char *rows[NEHE_SYMBOL_ROWS] = {
        "0000011100000",
        "0001111111000",
        "0011111111100",
        "0110111110110",
        "0110011100110",
        "0111111111110",
        "0011101011100",
        "0001111111000",
        "0010011100100",
        "0101011101010",
        "1000100010001",
        "0101000001010",
        "0010000000100"
    };

    if (row < 0 || row >= NEHE_SYMBOL_ROWS || col < 0 || col >= NEHE_SYMBOL_COLS) {
        return 0;
    }
    return rows[row][col] == '1';
}

static inline int nehe_skull_outline_pixel(int row, int col)
{
    if (!nehe_skull_symbol_pixel(row, col)) {
        return 0;
    }
    if (row == 0 || row == NEHE_SYMBOL_ROWS - 1 || col == 0 || col == NEHE_SYMBOL_COLS - 1) {
        return 1;
    }
    if (!nehe_skull_symbol_pixel(row - 1, col) ||
        !nehe_skull_symbol_pixel(row + 1, col) ||
        !nehe_skull_symbol_pixel(row, col - 1) ||
        !nehe_skull_symbol_pixel(row, col + 1)) {
        return 1;
    }
    return 0;
}

#endif
