#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 30

static void push_colored_triangle(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 0.5f, 1.0f };
    n3_push_triangle(va, vb, vc);
}

static void lesson_31(float t)
{
    float yaw = t * 0.72f;
    N3Color hull = { 0.58f, 0.72f, 0.88f, 1.0f };
    N3Color wing = { 0.28f, 0.42f, 0.68f, 1.0f };
    N3Color flame = { 1.0f, 0.42f + 0.25f * sinf(t * 8.0f), 0.06f, 1.0f };

    n3_bind_texture(NULL);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -5.4f, -0.08f, yaw, 0.0f);
    push_colored_triangle((N3Vec3){ 0.0f, 0.58f, -1.25f }, (N3Vec3){ -0.45f, -0.34f, 0.55f },
                          (N3Vec3){ 0.45f, -0.34f, 0.55f }, hull);
    push_colored_triangle((N3Vec3){ 0.0f, -0.18f, -1.05f }, (N3Vec3){ -1.55f, -0.44f, 0.35f },
                          (N3Vec3){ -0.45f, -0.34f, 0.55f }, wing);
    push_colored_triangle((N3Vec3){ 0.0f, -0.18f, -1.05f }, (N3Vec3){ 0.45f, -0.34f, 0.55f },
                          (N3Vec3){ 1.55f, -0.44f, 0.35f }, wing);
    push_colored_triangle((N3Vec3){ -0.45f, -0.34f, 0.55f }, (N3Vec3){ 0.0f, -0.04f, 1.05f },
                          (N3Vec3){ 0.45f, -0.34f, 0.55f }, (N3Color){ 0.42f, 0.55f, 0.78f, 1.0f });
    push_colored_triangle((N3Vec3){ -0.22f, -0.26f, 0.88f }, (N3Vec3){ 0.22f, -0.26f, 0.88f },
                          (N3Vec3){ 0.0f, -0.26f, 1.55f + 0.18f * sinf(t * 9.0f) }, flame);
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
    lesson_31(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 31 - model loading";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Loaded-style low-poly model mesh";
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
