#ifndef NEHE_BITMAP_FONT_H
#define NEHE_BITMAP_FONT_H

#include <stdint.h>

#define NEHE_FONT_ROWS 7
#define NEHE_FONT_COLS 5

typedef struct NeheBitmapGlyph {
    char ch;
    uint8_t rows[NEHE_FONT_ROWS];
} NeheBitmapGlyph;

static inline char nehe_font_fold_char(char ch)
{
    if (ch >= 'a' && ch <= 'z') {
        return (char)(ch - 'a' + 'A');
    }
    return ch;
}

static inline const uint8_t *nehe_font_rows(char ch)
{
    static const uint8_t blank[NEHE_FONT_ROWS] = { 0, 0, 0, 0, 0, 0, 0 };
    static const uint8_t fallback[NEHE_FONT_ROWS] = { 0x1f, 0x11, 0x05, 0x02, 0x04, 0x00, 0x04 };
    static const NeheBitmapGlyph glyphs[] = {
        { 'E', { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f } },
        { 'G', { 0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0e } },
        { 'H', { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 } },
        { 'I', { 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e } },
        { 'L', { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f } },
        { 'N', { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 } },
        { 'O', { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e } },
        { 'P', { 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10 } },
        { 'T', { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
        { 'W', { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0a } },
    };

    ch = nehe_font_fold_char(ch);
    if (ch == ' ') {
        return blank;
    }

    for (unsigned int i = 0; i < sizeof(glyphs) / sizeof(glyphs[0]); ++i) {
        if (glyphs[i].ch == ch) {
            return glyphs[i].rows;
        }
    }
    return fallback;
}

static inline int nehe_font_row_has_pixel(const uint8_t *rows, int row, int col)
{
    return (rows[row] & (uint8_t)(1u << (NEHE_FONT_COLS - 1 - col))) != 0;
}

#endif
