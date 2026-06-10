#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 44

static void push_plane_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };
    n3_push_quad(va, vb, vc, vd);
}

static void lesson_45(float t)
{
    const int cols = 18;
    const int rows = 12;
    n3_set_depth(true, true);
    n3_bind_texture(NULL);
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            float x0 = ((float)x - 8.5f) * 0.16f;
            float x1 = ((float)x + 1.0f - 8.5f) * 0.16f;
            float y0 = ((float)y - 5.5f) * 0.16f;
            float y1 = ((float)y + 1.0f - 5.5f) * 0.16f;
            float h0 = sinf(t * 2.0f + (float)x * 0.42f + (float)y * 0.33f) * 0.12f;
            float h1 = sinf(t * 2.0f + (float)(x + 1) * 0.42f + (float)y * 0.33f) * 0.12f;
            float h2 = sinf(t * 2.0f + (float)(x + 1) * 0.42f + (float)(y + 1) * 0.33f) * 0.12f;
            float h3 = sinf(t * 2.0f + (float)x * 0.42f + (float)(y + 1) * 0.33f) * 0.12f;
            N3Color color = { 0.18f + (float)x * 0.025f, 0.36f + (float)y * 0.035f, 0.92f, 1.0f };
            push_plane_quad((N3Vec3){ x0, y0, h0 }, (N3Vec3){ x1, y0, h1 },
                            (N3Vec3){ x1, y1, h2 }, (N3Vec3){ x0, y1, h3 }, color);
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
    lesson_45(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 45 - vertex buffers";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Streaming vertex field batches";
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
