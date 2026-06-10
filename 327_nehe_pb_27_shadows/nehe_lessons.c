#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 26

static N3Color face[6] = {
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.5f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 1.0f },
};

static void push_plane_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };
    n3_push_quad(va, vb, vc, vd);
}

static void lesson_27(float t)
{
    float angle = t * 44.0f * NEHE_DEG_TO_RAD;

    n3_set_camera(0.0f, 0.2f, -6.0f, -0.18f, 0.0f, 0.0f);
    n3_set_cull(false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);

    n3_bind_texture(NULL);
    n3_set_depth(false, false);
    push_plane_quad((N3Vec3){ -2.6f, -1.15f, -2.0f }, (N3Vec3){ 2.6f, -1.15f, -2.0f },
                    (N3Vec3){ 2.6f, -1.15f, 2.0f }, (N3Vec3){ -2.6f, -1.15f, 2.0f },
                    (N3Color){ 0.42f, 0.42f, 0.46f, 1.0f });
    push_plane_quad((N3Vec3){ -2.6f, -1.15f, 2.0f }, (N3Vec3){ 2.6f, -1.15f, 2.0f },
                    (N3Vec3){ 2.6f, 1.95f, 2.0f }, (N3Vec3){ -2.6f, 1.95f, 2.0f },
                    (N3Color){ 0.24f, 0.28f, 0.36f, 1.0f });
    push_plane_quad((N3Vec3){ -1.1f, -1.13f, -0.65f }, (N3Vec3){ 1.65f, -1.13f, -0.25f },
                    (N3Vec3){ 1.05f, -1.13f, 0.35f }, (N3Vec3){ -1.65f, -1.13f, -0.05f },
                    (N3Color){ 0.0f, 0.0f, 0.0f, 0.45f });
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_draw_pyramid(-0.72f, 0.05f, 0.0f, 0.55f, 0.70f, 0.55f, 0.0f, angle, 0.0f);
    n3_draw_cube_axis_angle(0.82f, -0.02f, 0.0f, 0.85f, 0.85f, 0.85f, angle, 0.3f, 1.0f, 0.2f, face);
    n3_set_cull(false);
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
    lesson_27(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 27 - shadows";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Planar shadow projection";
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
