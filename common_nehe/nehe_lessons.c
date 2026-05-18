#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_bitmap_font.h"
#include "nehe_native.h"
#include "nehe_outline_font.h"
#include "nehe_scene.h"
#include "nehe_starfield.h"
#include "nehe_texture_font.h"
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
static N3Texture font_texture;
static uint8_t font_texture_pixels[NEHE_TEXTURE_FONT_SIZE * NEHE_TEXTURE_FONT_SIZE * 4];

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
    "NeHe 15 - texture mapped outline fonts",
    "NeHe 16 - cool looking fog",
    "NeHe 17 - 2D texture font",
    "NeHe 18 - quadrics"
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
    "Texture mapped outline symbol",
    "Fog over textured crates",
    "Texture atlas font overlay",
    "Cube and GLU-style quadric shapes"
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
    nehe_texture_font_fill_rgba(font_texture_pixels);
    if (n3_texture_create_rgba(&font_texture, NEHE_TEXTURE_FONT_SIZE, NEHE_TEXTURE_FONT_SIZE, font_texture_pixels) != 0) {
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
    n3_texture_destroy(&font_texture);
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

static float fog_factor_for_mode(int mode, float distance)
{
    const float density = 0.16f;
    const float start = 3.8f;
    const float end = 9.2f;

    if (mode == 0) {
        return expf(-density * distance);
    }
    if (mode == 1) {
        float f = density * distance;
        return expf(-(f * f));
    }
    return fmaxf(0.0f, fminf(1.0f, (end - distance) / (end - start)));
}

static N3Color fogged_lit_color(N3Vec3 normal, float angle, float distance, int mode)
{
    N3Color fog = { 0.50f, 0.50f, 0.50f, 1.0f };
    N3Vec3 rotated = rotate_axis_angle(normal, angle, 1.0f, 1.0f, 0.0f);
    float diffuse = rotated.z > 0.0f ? rotated.z : 0.0f;
    float lit = 0.22f + diffuse * 0.78f;
    float factor = fog_factor_for_mode(mode, distance);

    return (N3Color){
        fog.r + (lit - fog.r) * factor,
        fog.g + (lit - fog.g) * factor,
        fog.b + (lit - fog.b) * factor,
        1.0f
    };
}

static void draw_fogged_crate_native(float x, float z, float angle, int mode)
{
    static const N3Vec3 normals[6] = {
        { 0.0f, 1.0f, 0.0f }, { 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f,-1.0f }, {-1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
    };
    N3Color colors[6];
    float distance = -(z - 5.8f);

    for (int i = 0; i < 6; ++i) {
        colors[i] = fogged_lit_color(normals[i], angle, distance, mode);
    }
    n3_draw_textured_cube_axis_angle_face_colors(x, 0.0f, z, 0.62f, angle, 1.0f, 1.0f, 0.0f, colors);
}

static void lesson_16(float t)
{
    int mode = ((int)(t / 2.5f)) % 3;
    float angle = t * 42.0f * NEHE_DEG_TO_RAD;

    n3_bind_texture(&crate_texture);
    n3_set_camera(0.0f, 0.0f, -5.8f, 0.0f, 0.0f, 0.0f);
    n3_set_depth(true, true);
    n3_set_cull(true);
    draw_fogged_crate_native(-1.25f, 0.0f, angle, mode);
    draw_fogged_crate_native(0.0f, -1.45f, angle, mode);
    draw_fogged_crate_native(1.25f, -2.9f, angle, mode);
    n3_set_cull(false);
}

static N3Vec3 lesson_17_transform(N3Vec3 p, float angle, float pre_angle)
{
    float z45 = 45.0f * NEHE_DEG_TO_RAD;
    float sx = sinf(z45), cx = cosf(z45);
    N3Vec3 q = p;
    float x;
    float y;

    if (pre_angle != 0.0f) {
        q = rotate_axis_angle(q, pre_angle, 1.0f, 1.0f, 0.0f);
    }
    q = rotate_axis_angle(q, angle, 1.0f, 1.0f, 0.0f);

    x = q.x * cx - q.y * sx;
    y = q.x * sx + q.y * cx;
    return (N3Vec3){ x, y, q.z };
}

static void push_lesson_17_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };

    n3_push_quad(va, vb, vc, vd);
}

static void draw_lesson_17_object(float t)
{
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
    float angle = t * 30.0f * NEHE_DEG_TO_RAD;
    N3Vec3 a = lesson_17_transform((N3Vec3){ -1.0f,  1.0f, 0.0f }, angle, 0.0f);
    N3Vec3 b = lesson_17_transform((N3Vec3){  1.0f,  1.0f, 0.0f }, angle, 0.0f);
    N3Vec3 c = lesson_17_transform((N3Vec3){  1.0f, -1.0f, 0.0f }, angle, 0.0f);
    N3Vec3 d = lesson_17_transform((N3Vec3){ -1.0f, -1.0f, 0.0f }, angle, 0.0f);
    N3Vec3 e = lesson_17_transform((N3Vec3){ -1.0f,  1.0f, 0.0f }, angle, 90.0f * NEHE_DEG_TO_RAD);
    N3Vec3 f = lesson_17_transform((N3Vec3){  1.0f,  1.0f, 0.0f }, angle, 90.0f * NEHE_DEG_TO_RAD);
    N3Vec3 g = lesson_17_transform((N3Vec3){  1.0f, -1.0f, 0.0f }, angle, 90.0f * NEHE_DEG_TO_RAD);
    N3Vec3 h = lesson_17_transform((N3Vec3){ -1.0f, -1.0f, 0.0f }, angle, 90.0f * NEHE_DEG_TO_RAD);

    n3_bind_texture(&cube_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    push_lesson_17_quad(a, b, c, d, white);
    push_lesson_17_quad(e, f, g, h, white);
}

static void push_texture_font_char(float x, float y, float size, unsigned char ch, int set, N3Color color)
{
    float u0, v0, u1, v1;
    N3Vertex a;
    N3Vertex b;
    N3Vertex c;
    N3Vertex d;

    nehe_texture_font_uv(ch, set, &u0, &v0, &u1, &v1);
    a = (N3Vertex){ { x, y, 0.0f }, color, u0, v0 };
    b = (N3Vertex){ { x + size, y, 0.0f }, color, u1, v0 };
    c = (N3Vertex){ { x + size, y + size, 0.0f }, color, u1, v1 };
    d = (N3Vertex){ { x, y + size, 0.0f }, color, u0, v1 };
    n3_push_quad(a, b, c, d);
}

static void draw_texture_font_text_native(float x, float y, const char *text, int set, N3Color color)
{
    const float view_z = 3.0f;
    const float view_h = 2.0f * view_z * tanf(NEHE_FOV_Y_DEGREES * NEHE_DEG_TO_RAD * 0.5f);
    const float view_w = view_h * NEHE_ASPECT;
    const float size = 16.0f * view_h / (float)NEHE_SCREEN_H;
    const float advance = 10.0f * view_w / (float)NEHE_SCREEN_W;
    float cursor = ((x / (float)NEHE_SCREEN_W) - 0.5f) * view_w;
    float baseline = ((y / (float)NEHE_SCREEN_H) - 0.5f) * view_h;

    n3_bind_texture(&font_texture);
    n3_set_depth(false, false);
    n3_set_camera(0.0f, 0.0f, -view_z, 0.0f, 0.0f, 0.0f);
    for (const char *p = text; *p != '\0'; ++p) {
        push_texture_font_char(cursor, baseline, size, (unsigned char)*p, set, color);
        cursor += advance;
    }
}

static void lesson_17(float t)
{
    float cnt1 = t * 0.60f;
    float cnt2 = t * 0.486f;
    N3Color color_a = {
        fmaxf(0.0f, cosf(cnt1)),
        fmaxf(0.0f, sinf(cnt2)),
        fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(cnt1 + cnt2))),
        1.0f
    };
    N3Color color_b = {
        fmaxf(0.0f, sinf(cnt2)),
        fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(cnt1 + cnt2))),
        fmaxf(0.0f, cosf(cnt1)),
        1.0f
    };

    draw_lesson_17_object(t);
    n3_flush();
    draw_texture_font_text_native(280.0f + 250.0f * cosf(cnt1), 235.0f + 200.0f * sinf(cnt2), "NeHe", 0, color_a);
    draw_texture_font_text_native(280.0f + 230.0f * cosf(cnt2), 235.0f + 200.0f * sinf(cnt1), "OpenGL", 1, color_b);
    draw_texture_font_text_native(240.0f + 200.0f * cosf((cnt1 + cnt2) / 5.0f), 2.0f, "Giuseppe D'Agata", 0,
                                  (N3Color){ 0.0f, 0.0f, 1.0f, 1.0f });
    draw_texture_font_text_native(242.0f + 200.0f * cosf((cnt1 + cnt2) / 5.0f), 4.0f, "Giuseppe D'Agata", 0,
                                  (N3Color){ 1.0f, 1.0f, 1.0f, 1.0f });
}

#define NEHE_QUADRIC_PI 3.14159265358979323846f
#define NEHE_QUADRIC_SLICES 32
#define NEHE_QUADRIC_STACKS 24
#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f
#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

static int lesson_18_object(float t)
{
    int object = (int)(t / NEHE_QUADRIC_CYCLE_SECONDS) % 6;
    return object < 0 ? 0 : object;
}

static float lesson_18_local_time(float t)
{
    float local = fmodf(t, NEHE_QUADRIC_CYCLE_SECONDS);
    return local < 0.0f ? local + NEHE_QUADRIC_CYCLE_SECONDS : local;
}

static N3Vec3 rotate_lesson_18_vec(N3Vec3 p, float rx, float ry)
{
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float y = p.y * cx - p.z * sx;
    float z = p.y * sx + p.z * cx;
    float x = p.x * cy + z * sy;
    float z2 = -p.x * sy + z * cy;

    return (N3Vec3){ x, y, z2 };
}

static N3Color lesson_18_color(N3Vec3 normal)
{
    N3Vec3 light = { 0.0f, 0.0f, 1.0f };
    float diffuse = normal.x * light.x + normal.y * light.y + normal.z * light.z;
    float factor;

    if (diffuse < 0.0f) {
        diffuse = 0.0f;
    }
    factor = 0.45f + diffuse * 0.55f;
    return (N3Color){ factor, factor, factor, 1.0f };
}

static N3Vertex lesson_18_vertex(N3Vec3 pos, N3Vec3 normal, float u, float v, float rx, float ry)
{
    N3Vec3 world_normal = rotate_lesson_18_vec(normal, rx, ry);
    pos = (N3Vec3){
        pos.x * NEHE_QUADRIC_NATIVE_SCALE,
        pos.y * NEHE_QUADRIC_NATIVE_SCALE,
        pos.z * NEHE_QUADRIC_NATIVE_SCALE
    };
    return (N3Vertex){ rotate_lesson_18_vec(pos, rx, ry), lesson_18_color(world_normal), u, v };
}

static void lesson_18_push_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d,
                                N3Vec3 na, N3Vec3 nb, N3Vec3 nc, N3Vec3 nd,
                                float u0, float v0, float u1, float v1, float rx, float ry)
{
    N3Vertex va = lesson_18_vertex(a, na, u0, v0, rx, ry);
    N3Vertex vb = lesson_18_vertex(b, nb, u1, v0, rx, ry);
    N3Vertex vc = lesson_18_vertex(c, nc, u1, v1, rx, ry);
    N3Vertex vd = lesson_18_vertex(d, nd, u0, v1, rx, ry);

    n3_push_quad(va, vb, vc, vd);
}

static void lesson_18_cube(float rx, float ry)
{
    static const N3Vec3 faces[6][4] = {
        { {  1,  1, -1 }, { -1,  1, -1 }, { -1,  1,  1 }, {  1,  1,  1 } },
        { {  1, -1,  1 }, { -1, -1,  1 }, { -1, -1, -1 }, {  1, -1, -1 } },
        { {  1,  1,  1 }, { -1,  1,  1 }, { -1, -1,  1 }, {  1, -1,  1 } },
        { {  1, -1, -1 }, { -1, -1, -1 }, { -1,  1, -1 }, {  1,  1, -1 } },
        { { -1,  1,  1 }, { -1,  1, -1 }, { -1, -1, -1 }, { -1, -1,  1 } },
        { {  1,  1, -1 }, {  1,  1,  1 }, {  1, -1,  1 }, {  1, -1, -1 } },
    };
    static const N3Vec3 normals[6] = {
        { 0, 1, 0 }, { 0,-1, 0 }, { 0, 0, 1 }, { 0, 0,-1 }, {-1,0,0 }, { 1,0,0 },
    };

    for (int i = 0; i < 6; ++i) {
        lesson_18_push_quad(faces[i][0], faces[i][1], faces[i][2], faces[i][3],
                            normals[i], normals[i], normals[i], normals[i],
                            1.0f, 0.0f, 0.0f, 1.0f, rx, ry);
    }
}

static void lesson_18_cylinder(float base_radius, float top_radius, float height, float rx, float ry)
{
    float slope = (base_radius - top_radius) / height;

    for (int stack = 0; stack < NEHE_QUADRIC_STACKS; ++stack) {
        float t0 = (float)stack / (float)NEHE_QUADRIC_STACKS;
        float t1 = (float)(stack + 1) / (float)NEHE_QUADRIC_STACKS;
        float z0 = t0 * height - height * 0.5f;
        float z1 = t1 * height - height * 0.5f;
        float r0 = base_radius + (top_radius - base_radius) * t0;
        float r1 = base_radius + (top_radius - base_radius) * t1;

        for (int slice = 0; slice < NEHE_QUADRIC_SLICES; ++slice) {
            float u0 = (float)slice / (float)NEHE_QUADRIC_SLICES;
            float u1 = (float)(slice + 1) / (float)NEHE_QUADRIC_SLICES;
            float a0 = u0 * NEHE_QUADRIC_PI * 2.0f;
            float a1 = u1 * NEHE_QUADRIC_PI * 2.0f;
            N3Vec3 n0 = { cosf(a0), sinf(a0), slope };
            N3Vec3 n1 = { cosf(a1), sinf(a1), slope };
            float n0_len = sqrtf(n0.x * n0.x + n0.y * n0.y + n0.z * n0.z);
            float n1_len = sqrtf(n1.x * n1.x + n1.y * n1.y + n1.z * n1.z);
            N3Vec3 p0 = { cosf(a0) * r0, sinf(a0) * r0, z0 };
            N3Vec3 p1 = { cosf(a1) * r0, sinf(a1) * r0, z0 };
            N3Vec3 p2 = { cosf(a1) * r1, sinf(a1) * r1, z1 };
            N3Vec3 p3 = { cosf(a0) * r1, sinf(a0) * r1, z1 };

            n0 = (N3Vec3){ n0.x / n0_len, n0.y / n0_len, n0.z / n0_len };
            n1 = (N3Vec3){ n1.x / n1_len, n1.y / n1_len, n1.z / n1_len };
            lesson_18_push_quad(p0, p1, p2, p3, n0, n1, n1, n0, u0, t0, u1, t1, rx, ry);
        }
    }
}

static void lesson_18_disk(float inner_radius, float outer_radius, float start_deg, float sweep_deg, float rx, float ry)
{
    int slices = NEHE_QUADRIC_SLICES;
    float sweep = sweep_deg * NEHE_DEG_TO_RAD;
    float start = start_deg * NEHE_DEG_TO_RAD;

    if (sweep_deg <= 0.0f) {
        return;
    }
    if (sweep_deg < 359.9f) {
        slices = 24;
    }

    for (int ring = 0; ring < NEHE_QUADRIC_STACKS; ++ring) {
        float r0 = inner_radius + (outer_radius - inner_radius) * (float)ring / (float)NEHE_QUADRIC_STACKS;
        float r1 = inner_radius + (outer_radius - inner_radius) * (float)(ring + 1) / (float)NEHE_QUADRIC_STACKS;

        for (int slice = 0; slice < slices; ++slice) {
            float u0 = (float)slice / (float)slices;
            float u1 = (float)(slice + 1) / (float)slices;
            float a0 = start + sweep * u0;
            float a1 = start + sweep * u1;
            N3Vec3 normal = { 0.0f, 0.0f, 1.0f };
            N3Vec3 p0 = { cosf(a0) * r0, sinf(a0) * r0, 0.0f };
            N3Vec3 p1 = { cosf(a1) * r0, sinf(a1) * r0, 0.0f };
            N3Vec3 p2 = { cosf(a1) * r1, sinf(a1) * r1, 0.0f };
            N3Vec3 p3 = { cosf(a0) * r1, sinf(a0) * r1, 0.0f };

            lesson_18_push_quad(p0, p1, p2, p3, normal, normal, normal, normal,
                                u0, (float)ring / (float)NEHE_QUADRIC_STACKS,
                                u1, (float)(ring + 1) / (float)NEHE_QUADRIC_STACKS, rx, ry);
        }
    }
}

static void lesson_18_sphere(float radius, float rx, float ry)
{
    for (int stack = 0; stack < NEHE_QUADRIC_STACKS; ++stack) {
        float v0 = (float)stack / (float)NEHE_QUADRIC_STACKS;
        float v1 = (float)(stack + 1) / (float)NEHE_QUADRIC_STACKS;
        float phi0 = -NEHE_QUADRIC_PI * 0.5f + v0 * NEHE_QUADRIC_PI;
        float phi1 = -NEHE_QUADRIC_PI * 0.5f + v1 * NEHE_QUADRIC_PI;

        for (int slice = 0; slice < NEHE_QUADRIC_SLICES; ++slice) {
            float u0 = (float)slice / (float)NEHE_QUADRIC_SLICES;
            float u1 = (float)(slice + 1) / (float)NEHE_QUADRIC_SLICES;
            float a0 = u0 * NEHE_QUADRIC_PI * 2.0f;
            float a1 = u1 * NEHE_QUADRIC_PI * 2.0f;
            N3Vec3 n0 = { cosf(phi0) * cosf(a0), cosf(phi0) * sinf(a0), sinf(phi0) };
            N3Vec3 n1 = { cosf(phi0) * cosf(a1), cosf(phi0) * sinf(a1), sinf(phi0) };
            N3Vec3 n2 = { cosf(phi1) * cosf(a1), cosf(phi1) * sinf(a1), sinf(phi1) };
            N3Vec3 n3 = { cosf(phi1) * cosf(a0), cosf(phi1) * sinf(a0), sinf(phi1) };
            N3Vec3 p0 = { n0.x * radius, n0.y * radius, n0.z * radius };
            N3Vec3 p1 = { n1.x * radius, n1.y * radius, n1.z * radius };
            N3Vec3 p2 = { n2.x * radius, n2.y * radius, n2.z * radius };
            N3Vec3 p3 = { n3.x * radius, n3.y * radius, n3.z * radius };

            lesson_18_push_quad(p0, p1, p2, p3, n0, n1, n2, n3, u0, v0, u1, v1, rx, ry);
        }
    }
}

static void lesson_18(float t)
{
    int object = lesson_18_object(t);
    float local = lesson_18_local_time(t);
    float rx = t * 45.0f * NEHE_DEG_TO_RAD;
    float ry = t * 33.0f * NEHE_DEG_TO_RAD;

    n3_bind_texture(&crate_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);

    switch (object) {
    case 0: lesson_18_cube(rx, ry); break;
    case 1: lesson_18_cylinder(1.0f, 1.0f, 3.0f, rx, ry); break;
    case 2: lesson_18_disk(0.5f, 1.5f, 0.0f, 360.0f, rx, ry); break;
    case 3: lesson_18_sphere(1.3f, rx, ry); break;
    case 4: lesson_18_cylinder(1.0f, 0.0f, 3.0f, rx, ry); break;
    default: {
        float phase = local / NEHE_QUADRIC_CYCLE_SECONDS;
        float start = phase < 0.5f ? 0.0f : (phase - 0.5f) * 720.0f;
        float sweep = phase < 0.5f ? phase * 720.0f : 360.0f - start;
        lesson_18_disk(0.5f, 1.5f, start, sweep, rx, ry);
        break;
    }
    }
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
    case 15: lesson_16(t); break;
    case 16: lesson_17(t); break;
    case 17: lesson_18(t); break;
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
    return lesson == 7 || lesson == 8 || lesson == 16;
}

uint32_t nehe_lesson_clear_color(int lesson)
{
    lesson = clamp_lesson(lesson);
    if (lesson == 0) {
        return 0x00020A16;
    }
    if (lesson == 15) {
        return 0x007F7F7F;
    }
    return 0x00070B14;
}
