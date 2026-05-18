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

#endif
