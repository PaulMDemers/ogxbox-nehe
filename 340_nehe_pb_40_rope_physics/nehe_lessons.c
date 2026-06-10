#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 39

#define NEHE_QUADRIC_PI 3.14159265358979323846f

#define NEHE_QUADRIC_SLICES 32

#define NEHE_QUADRIC_STACKS 24

#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f

#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

static void push_plane_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };
    n3_push_quad(va, vb, vc, vd);
}

static void lesson_40_segment(N3Vec3 a, N3Vec3 b, float width, N3Color color)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    float nx;
    float ny;

    if (len < 0.0001f) {
        return;
    }
    nx = -dy / len * width;
    ny = dx / len * width;
    push_plane_quad((N3Vec3){ a.x + nx, a.y + ny, a.z }, (N3Vec3){ b.x + nx, b.y + ny, b.z },
                    (N3Vec3){ b.x - nx, b.y - ny, b.z }, (N3Vec3){ a.x - nx, a.y - ny, a.z }, color);
}

static void lesson_40(float t)
{
    const int points = 18;
    N3Vec3 prev = { -1.55f, 0.72f, 0.0f };

    n3_bind_texture(NULL);
    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -4.2f, 0.0f, 0.0f, 0.0f);
    n3_draw_billboard(prev.x, prev.y, prev.z, 0.08f, 0.08f, (N3Color){ 1.0f, 0.95f, 0.45f, 1.0f });
    for (int i = 1; i < points; ++i) {
        float u = (float)i / (float)(points - 1);
        N3Vec3 p = {
            -1.55f + u * 3.1f,
            0.72f - sinf(u * NEHE_QUADRIC_PI) * (0.95f + 0.10f * sinf(t * 1.4f)) + sinf(t * 2.0f + u * 8.0f) * 0.08f,
            0.0f
        };
        lesson_40_segment(prev, p, 0.025f, (N3Color){ 0.90f, 0.72f, 0.36f, 1.0f });
        n3_draw_billboard(p.x, p.y, p.z + 0.02f, 0.045f, 0.045f, (N3Color){ 1.0f, 0.86f, 0.45f, 1.0f });
        prev = p;
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
    lesson_40(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 40 - rope physics";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Segmented rope simulation";
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
