#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 4

static N3Color face[6] = {
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.5f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 1.0f },
};

static void lesson_05(float t)
{
    n3_draw_pyramid(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, t * 0.20944f, 0.0f);
    n3_set_cull(true);
    n3_draw_cube_axis_angle(1.5f, 0.0f, -1.0f, 2.0f, 2.0f, 2.0f, -t * 0.15708f, 1.0f, 1.0f, 1.0f, face);
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
    lesson_05(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 05 - 3D shapes";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Pyramid and cube made from triangles";
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
