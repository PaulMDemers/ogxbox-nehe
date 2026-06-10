#include "nehe_lessons.h"
#include "nehe_bitmap_font.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>

#define LOCAL_NEHE_LESSON_INDEX 12

static void draw_bitmap_text_native(const char *text, float x, float y, float cell, N3Color color)
{
    float cursor = x;

    n3_set_depth(false, false);
    for (const char *p = text; *p != '\0'; ++p) {
        const uint8_t *rows = nehe_font_rows(*p);
        for (int row = 0; row < NEHE_FONT_ROWS; ++row) {
            for (int col = 0; col < NEHE_FONT_COLS; ++col) {
                if (nehe_font_row_has_pixel(rows, row, col)) {
                    n3_draw_quad(cursor + ((float)col + 0.5f) * cell,
                                 y - ((float)row + 0.5f) * cell,
                                 0.0f,
                                 cell * 0.43f,
                                 cell * 0.43f,
                                 0.0f,
                                 color);
                }
            }
        }
        cursor += cell * 6.0f;
    }
    n3_set_depth(true, true);
}

static void lesson_13(float t)
{
    float cnt1 = t * 0.60f;
    float cnt2 = t * 0.486f;
    N3Color color = {
        fmaxf(0.0f, cosf(cnt1)),
        fmaxf(0.0f, sinf(cnt2)),
        fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(cnt1 + cnt2))),
        1.0f
    };
    float x = -0.80f + 0.35f * cosf(cnt1);
    float y = 0.08f + 0.35f * sinf(cnt2);

    n3_set_camera(0.0f, 0.0f, -3.0f, 0.0f, 0.0f, 0.0f);
    draw_bitmap_text_native("OpenGL With NeHe", x, y, 0.017f, color);
}

int nehe_lessons_init(void)
{
    return 0;
}

void nehe_lessons_shutdown(void)
{
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_13(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 13 - bitmap fonts";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Animated bitmap-style text";
}

bool nehe_lesson_blend_enabled(int lesson)
{
    (void)lesson;
    return false;
}

uint32_t nehe_lesson_clear_color(int lesson)
{
    (void)lesson;
    return 0x00070B14u;
}
