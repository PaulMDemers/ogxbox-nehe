#include "nehe_lessons.h"
#include "nehe_outline_font.h"
#include "nehe_bitmap_font.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>

#define LOCAL_NEHE_LESSON_INDEX 13

static N3Vec3 rotate_lesson_14_point(float x, float y, float z, float rot)
{
    float rx = rot * NEHE_DEG_TO_RAD;
    float ry = rot * 1.2f * NEHE_DEG_TO_RAD;
    float rz = rot * 1.4f * NEHE_DEG_TO_RAD;
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float sz = sinf(rz), cz = cosf(rz);
    float y1 = y * cx - z * sx;
    float z1 = y * sx + z * cx;
    float x2 = x * cy + z1 * sy;
    float z2 = -x * sy + z1 * cy;
    float x3 = x2 * cz - y1 * sz;
    float y3 = x2 * sz + y1 * cz;

    return (N3Vec3){ x3, y3, z2 };
}

static void push_outline_pixel(float x0, float y0, float x1, float y1, float z, float rot, N3Color color)
{
    N3Vertex a = { rotate_lesson_14_point(x0, y0, z, rot), color, 0.0f, 0.0f };
    N3Vertex b = { rotate_lesson_14_point(x1, y0, z, rot), color, 1.0f, 0.0f };
    N3Vertex c = { rotate_lesson_14_point(x1, y1, z, rot), color, 1.0f, 1.0f };
    N3Vertex d = { rotate_lesson_14_point(x0, y1, z, rot), color, 0.0f, 1.0f };

    n3_push_quad(a, b, c, d);
}

static void draw_outline_text_native(const char *text, float rot, N3Color color)
{
    const float cell = 0.080f;
    const float depth = 0.12f;
    float cursor = -nehe_outline_text_width(text, cell) * 0.5f;
    N3Color back = { color.r * 0.20f, color.g * 0.20f, color.b * 0.20f, 1.0f };

    n3_set_depth(true, true);
    n3_set_cull(false);
    for (const char *p = text; *p != '\0'; ++p) {
        const uint8_t *rows = nehe_font_rows(*p);
        for (int row = 0; row < NEHE_FONT_ROWS; ++row) {
            for (int col = 0; col < NEHE_FONT_COLS; ++col) {
                if (nehe_outline_font_pixel(rows, row, col)) {
                    float x0 = cursor + (float)col * cell;
                    float y0 = 0.38f - (float)row * cell;
                    float x1 = x0 + cell * 0.78f;
                    float y1 = y0 - cell * 0.78f;

                    push_outline_pixel(x0, y0, x1, y1, -depth, rot, back);
                    push_outline_pixel(x0, y0, x1, y1, 0.0f, rot, color);
                }
            }
        }
        cursor += cell * 6.0f;
    }
}

static void lesson_14(float t)
{
    float rot = t * 3.0f;
    N3Color color = {
        fmaxf(0.0f, cosf(rot / 20.0f)),
        fmaxf(0.0f, sinf(rot / 25.0f)),
        fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(rot / 17.0f))),
        1.0f
    };

    n3_set_camera(0.0f, 0.0f, -10.0f, 0.0f, 0.0f, 0.0f);
    draw_outline_text_native("OpenGL With NeHe", rot, color);
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
    lesson_14(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 14 - outline fonts";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "3D outline-style text";
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
