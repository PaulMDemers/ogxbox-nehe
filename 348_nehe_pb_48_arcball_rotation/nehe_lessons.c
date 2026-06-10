#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 47

#define NEHE_QUADRIC_PI 3.14159265358979323846f

#define NEHE_QUADRIC_SLICES 32

#define NEHE_QUADRIC_STACKS 24

#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f

#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

static N3Texture crate_texture;

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

static void lesson_48(float t)
{
    float ax = sinf(t * 0.37f) * 0.6f + 0.35f;
    float ay = cosf(t * 0.29f) * 0.6f + 0.65f;
    float az = sinf(t * 0.23f) * 0.45f + 0.25f;
    n3_set_depth(true, true);
    n3_bind_texture(&crate_texture);
    n3_set_cull(true);
    n3_draw_textured_cube_axis_angle_tint(0.0f, 0.0f, 0.0f, 0.92f, t * 0.75f, ax, ay, az,
                                          (N3Color){ 0.95f, 0.98f, 1.0f, 1.0f });
    n3_set_cull(false);
    n3_bind_texture(NULL);
    for (int i = 0; i < 24; ++i) {
        float a0 = (float)i * NEHE_QUADRIC_PI / 12.0f;
        float a1 = (float)(i + 1) * NEHE_QUADRIC_PI / 12.0f;
        lesson_21_line(cosf(a0) * 1.35f, sinf(a0) * 1.35f,
                       cosf(a1) * 1.35f, sinf(a1) * 1.35f, 0.008f,
                       (N3Color){ 0.72f, 0.82f, 1.0f, 0.42f });
    }
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&crate_texture, NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&crate_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_48(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 48 - arcball rotation";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Trackball-controlled object rotation";
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
