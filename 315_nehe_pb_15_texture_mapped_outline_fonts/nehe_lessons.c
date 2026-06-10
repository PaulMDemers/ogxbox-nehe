#include "nehe_lessons.h"
#include "nehe_outline_font.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 14

static N3Texture star_texture;

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

static void push_textured_symbol_pixel(float x0, float y0, float x1, float y1, float z, float rot,
                                       float u0, float v0, float u1, float v1)
{
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
    N3Vertex a = { rotate_lesson_14_point(x0, y0, z, rot), white, u0, v0 };
    N3Vertex b = { rotate_lesson_14_point(x1, y0, z, rot), white, u1, v0 };
    N3Vertex c = { rotate_lesson_14_point(x1, y1, z, rot), white, u1, v1 };
    N3Vertex d = { rotate_lesson_14_point(x0, y1, z, rot), white, u0, v1 };

    n3_push_quad(a, b, c, d);
}

static void draw_textured_skull_native(float ox, float oy, float rot)
{
    const float cell = 0.145f;
    const float depth = 0.10f;
    float left = ox - (float)NEHE_SYMBOL_COLS * cell * 0.5f;
    float top = oy + (float)NEHE_SYMBOL_ROWS * cell * 0.5f;

    n3_set_depth(true, true);
    n3_set_cull(false);
    for (int row = 0; row < NEHE_SYMBOL_ROWS; ++row) {
        for (int col = 0; col < NEHE_SYMBOL_COLS; ++col) {
            if (nehe_skull_outline_pixel(row, col)) {
                float x0 = left + (float)col * cell;
                float y0 = top - (float)row * cell;
                float x1 = x0 + cell * 0.94f;
                float y1 = y0 - cell * 0.94f;
                float u0 = (float)col / (float)NEHE_SYMBOL_COLS;
                float v0 = (float)row / (float)NEHE_SYMBOL_ROWS;
                float u1 = (float)(col + 1) / (float)NEHE_SYMBOL_COLS;
                float v1 = (float)(row + 1) / (float)NEHE_SYMBOL_ROWS;

                push_textured_symbol_pixel(x0, y0, x1, y1, -depth, rot, u0, v0, u1, v1);
                push_textured_symbol_pixel(x0, y0, x1, y1, 0.0f, rot, u0, v0, u1, v1);
            }
        }
    }
}

static void lesson_15(float t)
{
    float rot = t * 55.0f;
    float x = sinf(t * 0.7f) * 0.35f;
    float y = cosf(t * 0.5f) * 0.22f;

    n3_bind_texture(&star_texture);
    n3_set_camera(0.0f, 0.0f, -5.6f, 0.0f, 0.0f, 0.0f);
    draw_textured_skull_native(x, y, rot);
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&star_texture, NEHE_ASSET_STAR_W, NEHE_ASSET_STAR_H, nehe_asset_star_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&star_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_15(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 15 - texture mapped outline fonts";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Texture mapped outline symbol";
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
