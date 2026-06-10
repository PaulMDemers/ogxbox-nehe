#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 27

#define NEHE_QUADRIC_PI 3.14159265358979323846f

#define NEHE_QUADRIC_SLICES 32

#define NEHE_QUADRIC_STACKS 24

#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f

#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

static N3Texture tim_texture;

static float bezier1(float p0, float p1, float p2, float p3, float t)
{
    float it = 1.0f - t;
    return p0 * it * it * it + 3.0f * p1 * t * it * it + 3.0f * p2 * t * t * it + p3 * t * t * t;
}

static N3Vec3 lesson_28_patch_point(float u, float v, float phase)
{
    float x = bezier1(-1.8f, -0.8f, 0.8f, 1.8f, u);
    float z = bezier1(-1.5f, -0.55f, 0.55f, 1.5f, v);
    float arch_u = sinf(u * NEHE_QUADRIC_PI);
    float arch_v = sinf(v * NEHE_QUADRIC_PI);
    return (N3Vec3){ x, (arch_u * arch_v - 0.45f) * 1.1f + sinf(phase + x * 1.6f + z) * 0.18f, z };
}

static void lesson_28(float t)
{
    const int divs = 14;
    float phase = t * 1.4f;

    n3_bind_texture(&tim_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.15f, -5.6f, -0.22f, 0.0f, 0.0f);
    for (int y = 0; y < divs; ++y) {
        for (int x = 0; x < divs; ++x) {
            float u0 = (float)x / (float)divs;
            float u1 = (float)(x + 1) / (float)divs;
            float v0 = (float)y / (float)divs;
            float v1 = (float)(y + 1) / (float)divs;
            N3Color color = { 0.55f + 0.45f * v0, 0.65f + 0.25f * u0, 1.0f, 1.0f };
            N3Vertex a = { lesson_28_patch_point(u0, v0, phase), color, u0, v0 };
            N3Vertex b = { lesson_28_patch_point(u1, v0, phase), color, u1, v0 };
            N3Vertex c = { lesson_28_patch_point(u1, v1, phase), color, u1, v1 };
            N3Vertex d = { lesson_28_patch_point(u0, v1, phase), color, u0, v1 };
            n3_push_quad(a, b, c, d);
        }
    }
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&tim_texture, NEHE_ASSET_TIM_W, NEHE_ASSET_TIM_H, nehe_asset_tim_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&tim_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_28(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 28 - Bezier patches";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Textured Bezier-style patch";
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
