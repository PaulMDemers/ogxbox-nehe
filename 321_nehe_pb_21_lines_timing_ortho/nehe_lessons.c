#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 20

#define NEHE_QUADRIC_PI 3.14159265358979323846f

#define NEHE_QUADRIC_SLICES 32

#define NEHE_QUADRIC_STACKS 24

#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f

#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

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

static void lesson_21(float t)
{
    n3_bind_texture(NULL);
    n3_set_depth(false, false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);
    n3_set_camera(0.0f, 0.0f, -4.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < 24; ++i) {
        float a = t * 0.7f + (float)i * NEHE_QUADRIC_PI / 12.0f;
        float r0 = 0.25f + 0.04f * (float)(i & 3);
        float r1 = 2.1f + 0.18f * sinf(t * 1.3f + (float)i);
        N3Color color = {
            0.35f + 0.65f * fabsf(sinf(a)),
            0.35f + 0.65f * fabsf(sinf(a + 2.1f)),
            0.35f + 0.65f * fabsf(sinf(a + 4.2f)),
            0.42f
        };
        lesson_21_line(cosf(a) * r0, sinf(a) * r0, cosf(a) * r1, sinf(a) * r1, 0.012f, color);
    }
    for (int i = 0; i < 18; ++i) {
        float x0 = -2.1f + (float)i * 0.25f;
        float x1 = x0 + 0.18f;
        float y = -1.4f + sinf(t * 3.2f + (float)i * 0.55f) * 0.23f;
        lesson_21_line(x0, y, x1, -y, 0.018f, (N3Color){ 0.9f, 0.9f, 1.0f, 0.72f });
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
    lesson_21(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 21 - lines/timing/ortho";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Timed antialiased line patterns";
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
