#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 45

static N3Color face[6] = {
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.5f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 1.0f },
};

static void lesson_21_line(float x0, float y0, float x1, float y1, float width, N3Color color)
{
    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = sqrtf(dx * dx + dy * dy);
    float nx;
    float ny;
    N3Vertex a, b, c, d;

    if (len < 0.0001f) {
        return;
    }
    nx = -dy / len * width;
    ny = dx / len * width;
    a = (N3Vertex){ { x0 + nx, y0 + ny, 0.0f }, color, 0.0f, 0.0f };
    b = (N3Vertex){ { x1 + nx, y1 + ny, 0.0f }, color, 1.0f, 0.0f };
    c = (N3Vertex){ { x1 - nx, y1 - ny, 0.0f }, color, 1.0f, 1.0f };
    d = (N3Vertex){ { x0 - nx, y0 - ny, 0.0f }, color, 0.0f, 1.0f };
    n3_push_quad(a, b, c, d);
}

static void lesson_46(float t)
{
    n3_set_depth(false, false);
    n3_bind_texture(NULL);
    for (int i = 0; i < 16; ++i) {
        float y = -1.15f + (float)i * 0.15f;
        float alpha = 0.18f + (float)i * 0.035f;
        lesson_21_line(-1.55f, y, 1.55f, y + 0.12f, 0.005f + (float)i * 0.0015f,
                       (N3Color){ 0.22f + (float)i * 0.035f, 0.72f, 1.0f, alpha });
    }
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_draw_cube_axis_angle(0.0f, 0.0f, -0.02f, 0.82f, 0.82f, 0.82f, t * 0.55f, 0.25f, 1.0f, 0.15f, face);
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
    lesson_46(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 46 - antialiasing";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Subpixel line and edge smoothing";
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
