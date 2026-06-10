#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 36

static void lesson_37(float t)
{
    const N3Color cel_faces[6] = {
        { 0.10f, 0.80f, 0.18f, 1.0f },
        { 0.06f, 0.38f, 0.10f, 1.0f },
        { 0.88f, 0.15f, 0.10f, 1.0f },
        { 0.54f, 0.08f, 0.06f, 1.0f },
        { 0.12f, 0.25f, 0.90f, 1.0f },
        { 0.05f, 0.09f, 0.42f, 1.0f }
    };

    n3_bind_texture(NULL);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -5.2f, -0.10f, 0.0f, 0.0f);
    n3_draw_cube_axis_angle(0.0f, 0.0f, 0.0f, 1.12f, 1.12f, 1.12f, t * 0.75f, 0.3f, 1.0f, 0.2f,
                            (N3Color[6]){{0.01f,0.01f,0.01f,1.0f},{0.01f,0.01f,0.01f,1.0f},{0.01f,0.01f,0.01f,1.0f},
                                         {0.01f,0.01f,0.01f,1.0f},{0.01f,0.01f,0.01f,1.0f},{0.01f,0.01f,0.01f,1.0f}});
    n3_draw_cube_axis_angle(0.0f, 0.0f, 0.0f, 0.98f, 0.98f, 0.98f, t * 0.75f, 0.3f, 1.0f, 0.2f, cel_faces);
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
    lesson_37(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 37 - cel shading";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Quantized lighting and outlines";
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
