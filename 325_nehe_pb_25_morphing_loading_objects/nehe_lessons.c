#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 24

#define NEHE_QUADRIC_PI 3.14159265358979323846f

#define NEHE_QUADRIC_SLICES 32

#define NEHE_QUADRIC_STACKS 24

#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f

#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

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

static float lesson_19_unit(int index, uint32_t salt)
{
    uint32_t h = (uint32_t)index * 1664525u + salt * 1013904223u;

    h ^= h >> 16;
    h *= 2246822519u;
    h ^= h >> 13;
    return (float)(h & 0xffffu) / 65535.0f;
}

static N3Vec3 lesson_25_target(int shape, int index, int count)
{
    float u = (float)index / (float)count;
    float a = u * NEHE_QUADRIC_PI * 2.0f;
    float b = (float)((index * 37) % count) / (float)count * NEHE_QUADRIC_PI * 2.0f;

    switch (shape & 3) {
    case 0:
        return (N3Vec3){ cosf(a) * sinf(b) * 1.4f, sinf(a) * sinf(b) * 1.4f, cosf(b) * 1.4f };
    case 1: {
        float r = 1.0f + 0.35f * cosf(b * 3.0f);
        return (N3Vec3){ cosf(a) * r, sinf(b * 2.0f) * 0.55f, sinf(a) * r };
    }
    case 2:
        return (N3Vec3){ cosf(a) * 0.75f, sinf(a * 2.0f) * 1.35f, sinf(a) * 0.75f };
    default:
        return (N3Vec3){
            lesson_19_unit(index, 21u) * 3.4f - 1.7f,
            lesson_19_unit(index, 22u) * 3.0f - 1.5f,
            lesson_19_unit(index, 23u) * 3.4f - 1.7f
        };
    }
}

static void lesson_25(float t)
{
    const int count = 260;
    float cycle = t * 0.45f;
    int from = (int)floorf(cycle);
    float mix = cycle - floorf(cycle);
    float smooth = mix * mix * (3.0f - 2.0f * mix);

    n3_bind_texture(NULL);
    n3_set_depth(true, true);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    n3_set_camera(0.0f, 0.0f, -7.5f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < count; ++i) {
        N3Vec3 a = lesson_25_target(from, i, count);
        N3Vec3 b = lesson_25_target(from + 1, i, count);
        N3Vec3 p = {
            a.x + (b.x - a.x) * smooth,
            a.y + (b.y - a.y) * smooth,
            a.z + (b.z - a.z) * smooth
        };
        N3Color c = {
            0.35f + 0.65f * lesson_19_unit(i, 30u),
            0.35f + 0.65f * lesson_19_unit(i, 31u),
            0.65f + 0.35f * lesson_19_unit(i, 32u),
            0.78f
        };
        p = rotate_axis_angle(p, t * 0.45f, 0.2f, 1.0f, 0.1f);
        n3_draw_billboard(p.x, p.y, p.z, 0.025f, 0.025f, c);
    }
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
    lesson_25(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 25 - morphing/loading objects";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Point-cloud morphing between objects";
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
