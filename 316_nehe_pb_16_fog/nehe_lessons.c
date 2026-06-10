#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 15

static N3Texture crate_texture;

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

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&crate_texture, NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&crate_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_16(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 16 - cool looking fog";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Fog over textured crates";
}

bool nehe_lesson_blend_enabled(int lesson)
{
    (void)lesson;
    return false;
}

uint32_t nehe_lesson_clear_color(int lesson)
{
    (void)lesson;
    return 0x007F7F7Fu;
}
