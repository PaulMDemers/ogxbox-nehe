#ifndef NEHE_TEXTURE_FONT_H
#define NEHE_TEXTURE_FONT_H

#include "nehe_bitmap_font.h"

#include <stdint.h>
#include <string.h>

#define NEHE_TEXTURE_FONT_CELL 8
#define NEHE_TEXTURE_FONT_GRID 16
#define NEHE_TEXTURE_FONT_SIZE (NEHE_TEXTURE_FONT_CELL * NEHE_TEXTURE_FONT_GRID)

static inline void nehe_texture_font_fill_rgba(uint8_t *pixels)
{
    memset(pixels, 0, NEHE_TEXTURE_FONT_SIZE * NEHE_TEXTURE_FONT_SIZE * 4u);

    for (int glyph = 0; glyph < 256; ++glyph) {
        int ch = glyph & 0x7f;
        int slanted = glyph >= 128;
        int cell_x = (glyph & 15) * NEHE_TEXTURE_FONT_CELL;
        int cell_y = (glyph >> 4) * NEHE_TEXTURE_FONT_CELL;
        const uint8_t *rows = nehe_font_rows((char)ch);

        for (int row = 0; row < NEHE_FONT_ROWS; ++row) {
            for (int col = 0; col < NEHE_FONT_COLS; ++col) {
                if (nehe_font_row_has_pixel(rows, row, col)) {
                    int slant = slanted ? (NEHE_FONT_ROWS - 1 - row) / 3 : 0;
                    int x = cell_x + 1 + col + slant;
                    int y = cell_y + row;
                    uint32_t dst = ((uint32_t)y * NEHE_TEXTURE_FONT_SIZE + (uint32_t)x) * 4u;

                    pixels[dst + 0] = 255;
                    pixels[dst + 1] = 255;
                    pixels[dst + 2] = 255;
                    pixels[dst + 3] = 255;
                }
            }
        }
    }
}

static inline void nehe_texture_font_uv(unsigned char ch, int set, float *u0, float *v0, float *u1, float *v1)
{
    int glyph = ((int)ch & 0x7f) + (set ? 128 : 0);
    int gx = glyph & 15;
    int gy = glyph >> 4;
    float cell = 1.0f / (float)NEHE_TEXTURE_FONT_GRID;

    *u0 = (float)gx * cell;
    *v0 = (float)gy * cell;
    *u1 = (float)(gx + 1) * cell;
    *v1 = (float)(gy + 1) * cell;
}

#endif
