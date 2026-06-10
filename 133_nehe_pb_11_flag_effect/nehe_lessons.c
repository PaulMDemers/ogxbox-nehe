#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 10

static N3Texture tim_texture;

static void lesson_11(float t)
{
    N3Color color = { 1.0f, 1.0f, 1.0f, 1.0f };
    int cells = 22;
    float scale = 4.4f / (float)cells;

    n3_bind_texture(&tim_texture);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    for (int y = 0; y < cells; ++y) {
        for (int x = 0; x < cells; ++x) {
            float x0 = ((float)x - cells * 0.5f) * scale;
            float x1 = ((float)(x + 1) - cells * 0.5f) * scale;
            float y0 = ((float)y - cells * 0.5f) * scale;
            float y1 = ((float)(y + 1) - cells * 0.5f) * scale;
            float z00 = sinf(t * 2.2f + x0 * 2.0f + y0 * 1.4f) * 0.25f;
            float z10 = sinf(t * 2.2f + x1 * 2.0f + y0 * 1.4f) * 0.25f;
            float z11 = sinf(t * 2.2f + x1 * 2.0f + y1 * 1.4f) * 0.25f;
            float z01 = sinf(t * 2.2f + x0 * 2.0f + y1 * 1.4f) * 0.25f;
            float u0 = (float)x / (float)cells;
            float u1 = (float)(x + 1) / (float)cells;
            float v0 = (float)y / (float)cells;
            float v1 = (float)(y + 1) / (float)cells;
            N3Vertex a = { { x0, y0, z00 }, color, u0, v0 };
            N3Vertex b = { { x1, y0, z10 }, color, u1, v0 };
            N3Vertex c = { { x1, y1, z11 }, color, u1, v1 };
            N3Vertex d = { { x0, y1, z01 }, color, u0, v1 };
            n3_push_quad(a, b, c, d);
        }
    }
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&tim_texture, NEHE_ASSET_TIM_W, NEHE_ASSET_TIM_H, nehe_asset_tim_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&tim_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_11(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 11 - waving texture mesh";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Original tim texture waving mesh";
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
