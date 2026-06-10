#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 1

static void lesson_02(float t)
{
    (void)t;
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
    n3_draw_triangle(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, white, white, white);
    n3_draw_quad(1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, white);
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
    lesson_02(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 02 - first polygons";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Triangle and quad placement";
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
