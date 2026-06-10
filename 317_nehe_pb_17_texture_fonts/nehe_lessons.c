#include "nehe_lessons.h"
#include "nehe_texture_font.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 16

static N3Texture cube_texture;

static N3Texture font_texture;
static uint8_t font_texture_pixels[NEHE_TEXTURE_FONT_SIZE * NEHE_TEXTURE_FONT_SIZE * 4];

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

int nehe_lessons_init(void)
{
    nehe_texture_font_fill_rgba(font_texture_pixels);
    if (n3_texture_create_rgba(&cube_texture, NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    if (n3_texture_create_rgba(&font_texture, NEHE_TEXTURE_FONT_SIZE, NEHE_TEXTURE_FONT_SIZE, font_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&cube_texture);
    n3_texture_destroy(&font_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_17(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 17 - 2D texture font";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Texture atlas font overlay";
}

bool nehe_lesson_blend_enabled(int lesson)
{
    (void)lesson;
    return true;
}

uint32_t nehe_lesson_clear_color(int lesson)
{
    (void)lesson;
    return 0x00070B14u;
}
