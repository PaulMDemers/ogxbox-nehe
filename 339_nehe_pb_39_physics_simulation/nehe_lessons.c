#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 38

static void push_plane_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };
    n3_push_quad(va, vb, vc, vd);
}

static float lesson_39_height(int x, int y, float t)
{
    return sinf((float)x * 0.8f + t * 1.6f) * 0.18f + cosf((float)y * 0.7f - t * 1.1f) * 0.14f;
}

static void lesson_39(float t)
{
    const int cols = 9;
    const int rows = 7;
    const float step = 0.34f;

    n3_bind_texture(NULL);
    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -4.4f, -0.18f, 0.0f, 0.0f);
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            float x0 = ((float)x - (float)(cols - 1) * 0.5f) * step;
            float y0 = ((float)y - (float)(rows - 1) * 0.5f) * step;
            float x1 = x0 + step;
            float y1 = y0 + step;
            N3Color color = { 0.18f + (float)x * 0.035f, 0.38f + (float)y * 0.045f, 0.90f, 0.72f };
            push_plane_quad((N3Vec3){ x0, y0 + lesson_39_height(x, y, t), 0.0f },
                            (N3Vec3){ x1, y0 + lesson_39_height(x + 1, y, t), 0.0f },
                            (N3Vec3){ x1, y1 + lesson_39_height(x + 1, y + 1, t), 0.0f },
                            (N3Vec3){ x0, y1 + lesson_39_height(x, y + 1, t), 0.0f }, color);
        }
    }
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            n3_draw_billboard(((float)x - (float)(cols - 1) * 0.5f) * step,
                              ((float)y - (float)(rows - 1) * 0.5f) * step + lesson_39_height(x, y, t),
                              0.04f, 0.035f, 0.035f, (N3Color){ 1.0f, 1.0f, 1.0f, 0.9f });
        }
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
    lesson_39(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 39 - physics simulation";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Spring-mass field simulation";
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
