#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 6

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
    lesson_07(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 07 - light/filter/input";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Original crate texture with lighting";
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
