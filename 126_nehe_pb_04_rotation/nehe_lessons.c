#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 3

static const N3Color red = { 1.0f, 0.05f, 0.05f, 1.0f };

static const N3Color green = { 0.08f, 0.9f, 0.2f, 1.0f };

static const N3Color blue = { 0.1f, 0.25f, 1.0f, 1.0f };

static void lesson_04(float t)
{
    n3_draw_triangle(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, t * 70.0f * NEHE_DEG_TO_RAD, red, green, blue);
    n3_draw_quad(1.5f, 0.0f, 0.0f, 1.0f, 1.0f, -t * 55.0f * NEHE_DEG_TO_RAD, (N3Color){ 0.5f, 0.5f, 1.0f, 1.0f });
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
    lesson_04(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 04 - rotation";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Animated transforms";
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
