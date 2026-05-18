#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_bitmap_font.h"
#include "nehe_native.h"
#include "nehe_outline_font.h"
#include "nehe_scene.h"
#include "nehe_starfield.h"
#include "nehe_world_data.h"

#include <math.h>
#include <pbkit/pbkit.h>

static const N3Color red = { 1.0f, 0.05f, 0.05f, 1.0f };
static const N3Color green = { 0.08f, 0.9f, 0.2f, 1.0f };
static const N3Color blue = { 0.1f, 0.25f, 1.0f, 1.0f };

static N3Color face[6] = {
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.5f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 1.0f },
};

static N3Texture nehe_texture;
static N3Texture crate_texture;
static N3Texture glass_texture;
static N3Texture star_texture;
static N3Texture mud_texture;
static N3Texture tim_texture;
static N3Texture cube_texture;

static const char *titles[NEHE_LESSON_COUNT] = {
    "NeHe 01 - window / clear",
    "NeHe 02 - first polygons",
    "NeHe 03 - color",
    "NeHe 04 - rotation",
    "NeHe 05 - 3D shapes",
    "NeHe 06 - texture mapping",
    "NeHe 07 - light/filter/input",
    "NeHe 08 - blending",
    "NeHe 09 - moving bitmaps/starfield",
    "NeHe 10 - world walkthrough",
    "NeHe 11 - waving texture mesh",
    "NeHe 12 - display lists",
    "NeHe 13 - bitmap fonts",
    "NeHe 14 - outline fonts",
    "NeHe 15 - texture mapped outline fonts"
};

static const char *details[NEHE_LESSON_COUNT] = {
    "Native pbkit/NV2A frame setup",
    "Triangle and quad placement",
    "Smooth vertex color and flat face color",
    "Animated transforms",
    "Pyramid and cube made from triangles",
    "Original NeHe.bmp texture on cube faces",
    "Original crate texture with lighting",
    "Original glass texture alpha cube",
    "Original star texture field",
    "Original mud texture world mesh",
    "Original tim texture waving mesh",
    "Original cube texture repeated stack",
    "Animated bitmap-style text",
    "3D outline-style text",
    "Texture mapped outline symbol"
};

static int clamp_lesson(int lesson)
{
    if (lesson < 0) {
        return 0;
    }
    if (lesson >= NEHE_LESSON_COUNT) {
        return NEHE_LESSON_COUNT - 1;
    }
    return lesson;
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&nehe_texture, NEHE_ASSET_NEHE_W, NEHE_ASSET_NEHE_H, nehe_asset_nehe_rgba) != 0) {
        return -1;
    }
    if (n3_texture_create_rgba(&crate_texture, NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba) != 0 ||
        n3_texture_create_rgba(&glass_texture, NEHE_ASSET_GLASS_W, NEHE_ASSET_GLASS_H, nehe_asset_glass_rgba) != 0 ||
        n3_texture_create_rgba(&star_texture, NEHE_ASSET_STAR_W, NEHE_ASSET_STAR_H, nehe_asset_star_rgba) != 0 ||
        n3_texture_create_rgba(&mud_texture, NEHE_ASSET_MUD_W, NEHE_ASSET_MUD_H, nehe_asset_mud_rgba) != 0 ||
        n3_texture_create_rgba(&tim_texture, NEHE_ASSET_TIM_W, NEHE_ASSET_TIM_H, nehe_asset_tim_rgba) != 0 ||
        n3_texture_create_rgba(&cube_texture, NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba) != 0) {
        n3_texture_destroy(&nehe_texture);
        n3_texture_destroy(&crate_texture);
        n3_texture_destroy(&glass_texture);
        n3_texture_destroy(&star_texture);
        n3_texture_destroy(&mud_texture);
        n3_texture_destroy(&tim_texture);
        n3_texture_destroy(&cube_texture);
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&nehe_texture);
    n3_texture_destroy(&crate_texture);
    n3_texture_destroy(&glass_texture);
    n3_texture_destroy(&star_texture);
    n3_texture_destroy(&mud_texture);
    n3_texture_destroy(&tim_texture);
    n3_texture_destroy(&cube_texture);
}

static void lesson_01(float t)
{
    (void)t;
}

static void lesson_02(float t)
{
    (void)t;
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
    n3_draw_triangle(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, white, white, white);
    n3_draw_quad(1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, white);
}

static void lesson_03(float t)
{
    (void)t;
    n3_draw_triangle(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, red, green, blue);
    n3_draw_quad(1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, (N3Color){ 0.5f, 0.5f, 1.0f, 1.0f });
}

static void lesson_04(float t)
{
    n3_draw_triangle(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, t * 70.0f * NEHE_DEG_TO_RAD, red, green, blue);
    n3_draw_quad(1.5f, 0.0f, 0.0f, 1.0f, 1.0f, -t * 55.0f * NEHE_DEG_TO_RAD, (N3Color){ 0.5f, 0.5f, 1.0f, 1.0f });
}

static void lesson_05(float t)
{
    n3_draw_pyramid(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, t * 0.20944f, 0.0f);
    n3_set_cull(true);
    n3_draw_cube_axis_angle(1.5f, 0.0f, -1.0f, 2.0f, 2.0f, 2.0f, -t * 0.15708f, 1.0f, 1.0f, 1.0f, face);
    n3_set_cull(false);
}

static void lesson_06(float t)
{
    n3_bind_texture(&nehe_texture);
    n3_set_cull(true);
    n3_draw_textured_cube_axis_angle(0.0f, 0.0f, 0.0f, 1.0f, t * 45.0f * NEHE_DEG_TO_RAD, 1.0f, 1.0f, 0.0f);
    n3_set_cull(false);
}

static N3Vec3 rotate_axis_angle(N3Vec3 p, float angle, float ax, float ay, float az)
{
    float len = sqrtf(ax * ax + ay * ay + az * az);
    if (len <= 0.000001f) {
        return p;
    }

    ax /= len;
    ay /= len;
    az /= len;

    float c = cosf(angle);
    float s = sinf(angle);
    float one_c = 1.0f - c;
    return (N3Vec3){
        (c + ax * ax * one_c) * p.x + (ax * ay * one_c - az * s) * p.y + (ax * az * one_c + ay * s) * p.z,
        (ay * ax * one_c + az * s) * p.x + (c + ay * ay * one_c) * p.y + (ay * az * one_c - ax * s) * p.z,
        (az * ax * one_c - ay * s) * p.x + (az * ay * one_c + ax * s) * p.y + (c + az * az * one_c) * p.z,
    };
}

static void lesson_07(float t)
{
    static const N3Vec3 normals[6] = {
        { 0.0f, 1.0f, 0.0f }, { 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f,-1.0f }, {-1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
    };
    N3Color lit[6];
    float angle = t * 42.0f * NEHE_DEG_TO_RAD;

    for (int i = 0; i < 6; ++i) {
        N3Vec3 normal = rotate_axis_angle(normals[i], angle, 1.0f, 1.0f, 0.0f);
        float diffuse = normal.z > 0.0f ? normal.z : 0.0f;
        float factor = 0.2f + diffuse * 0.8f;
        lit[i] = (N3Color){ factor, factor, factor, 1.0f };
    }

    n3_bind_texture(&crate_texture);
    n3_set_cull(true);
    n3_draw_textured_cube_axis_angle_face_colors(0.0f, 0.0f, 0.0f, 1.0f, angle, 1.0f, 1.0f, 0.0f, lit);
    n3_set_cull(false);
}

static void lesson_08(float t)
{
    n3_bind_texture(&glass_texture);
    n3_draw_textured_cube_axis_angle(0.0f, 0.0f, 0.0f, 1.0f, t * 40.0f * NEHE_DEG_TO_RAD, 1.0f, 1.0f, 0.0f);
}

static void lesson_09(float t)
{
    n3_bind_texture(&star_texture);
    n3_set_depth(false, false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    n3_set_camera(0.0f, 0.0f, NEHE_STAR_Z, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < NEHE_STAR_COUNT; ++i) {
        float x;
        float y;
        float spin;
        N3Color color = { 1.0f, 1.0f, 1.0f, 1.0f };

        nehe_star_state(i, t, &x, &y, &spin);
        nehe_star_color(i, &color.r, &color.g, &color.b);
        n3_draw_textured_quad(x, y, 0.0f, 1.0f, 1.0f, spin * NEHE_DEG_TO_RAD, color);
    }
}

static void lesson_10(float t)
{
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
    float yaw = sinf(t * 0.35f) * 0.12f;

    n3_bind_texture(&mud_texture);
    n3_set_camera(0.0f, -0.55f + sinf(t * 1.6f) * 0.04f, -6.0f, 0.0f, yaw, 0.0f);
    for (int i = 0; i < NEHE_WORLD_TRIANGLES; ++i) {
        N3Vertex a = {
            { nehe_world_vertices[i][0][0], nehe_world_vertices[i][0][1], nehe_world_vertices[i][0][2] },
            white, nehe_world_vertices[i][0][3], nehe_world_vertices[i][0][4]
        };
        N3Vertex b = {
            { nehe_world_vertices[i][1][0], nehe_world_vertices[i][1][1], nehe_world_vertices[i][1][2] },
            white, nehe_world_vertices[i][1][3], nehe_world_vertices[i][1][4]
        };
        N3Vertex c = {
            { nehe_world_vertices[i][2][0], nehe_world_vertices[i][2][1], nehe_world_vertices[i][2][2] },
            white, nehe_world_vertices[i][2][3], nehe_world_vertices[i][2][4]
        };
        n3_push_triangle(a, b, c);
    }
}

static void lesson_11(float t)
{
    N3Color color = { 1.0f, 1.0f, 1.0f, 1.0f };
    int cells = 22;
    float scale = 4.4f / (float)cells;

    n3_bind_texture(&tim_texture);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    for (int y = 0; y < cells; ++y) {
        for (int x = 0; x < cells; ++x) {
            float x0 = ((float)x - cells * 0.5f) * scale;
            float x1 = ((float)(x + 1) - cells * 0.5f) * scale;
            float y0 = ((float)y - cells * 0.5f) * scale;
            float y1 = ((float)(y + 1) - cells * 0.5f) * scale;
            float z00 = sinf(t * 2.2f + x0 * 2.0f + y0 * 1.4f) * 0.25f;
            float z10 = sinf(t * 2.2f + x1 * 2.0f + y0 * 1.4f) * 0.25f;
            float z11 = sinf(t * 2.2f + x1 * 2.0f + y1 * 1.4f) * 0.25f;
            float z01 = sinf(t * 2.2f + x0 * 2.0f + y1 * 1.4f) * 0.25f;
            float u0 = (float)x / (float)cells;
            float u1 = (float)(x + 1) / (float)cells;
            float v0 = (float)y / (float)cells;
            float v1 = (float)(y + 1) / (float)cells;
            N3Vertex a = { { x0, y0, z00 }, color, u0, v0 };
            N3Vertex b = { { x1, y0, z10 }, color, u1, v0 };
            N3Vertex c = { { x1, y1, z11 }, color, u1, v1 };
            N3Vertex d = { { x0, y1, z01 }, color, u0, v1 };
            n3_push_quad(a, b, c, d);
        }
    }
}

static void lesson_12(float t)
{
    static const N3Color boxcol[5] = {
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.5f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 1.0f, 1.0f },
    };
    n3_bind_texture(&cube_texture);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    n3_set_depth(true, true);
    for (int yloop = 5; yloop >= 1; --yloop) {
        for (int xloop = 0; xloop < yloop; ++xloop) {
            float px = ((float)xloop - ((float)yloop - 1.0f) * 0.5f) * 0.58f;
            float py = (3.0f - (float)yloop) * 0.42f;
            (void)t;
            n3_draw_textured_cube_tint(px, py, 0.0f, 0.26f, 0.49f, 0.61f, 0.0f, boxcol[yloop - 1]);
        }
    }
}

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

void nehe_lesson_render(int lesson, float t)
{
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);

    switch (clamp_lesson(lesson)) {
    case 0: lesson_01(t); break;
    case 1: lesson_02(t); break;
    case 2: lesson_03(t); break;
    case 3: lesson_04(t); break;
    case 4: lesson_05(t); break;
    case 5: lesson_06(t); break;
    case 6: lesson_07(t); break;
    case 7: lesson_08(t); break;
    case 8: lesson_09(t); break;
    case 9: lesson_10(t); break;
    case 10: lesson_11(t); break;
    case 11: lesson_12(t); break;
    case 12: lesson_13(t); break;
    case 13: lesson_14(t); break;
    case 14: lesson_15(t); break;
    }
}

const char *nehe_lesson_title(int lesson)
{
    return titles[clamp_lesson(lesson)];
}

const char *nehe_lesson_detail(int lesson)
{
    return details[clamp_lesson(lesson)];
}

bool nehe_lesson_blend_enabled(int lesson)
{
    lesson = clamp_lesson(lesson);
    return lesson == 7 || lesson == 8;
}

uint32_t nehe_lesson_clear_color(int lesson)
{
    return clamp_lesson(lesson) == 0 ? 0x00020A16 : 0x00070B14;
}
