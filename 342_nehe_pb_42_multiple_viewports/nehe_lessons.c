#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 41

static N3Texture crate_texture;

static void lesson_42(float t)
{
    const int w = n3_back_buffer_width();
    const int h = n3_back_buffer_height();
    const int half_w = w / 2;
    const int half_h = h / 2;
    const int rects[4][4] = {
        { 0, 0, half_w, half_h },
        { half_w, 0, half_w, half_h },
        { 0, half_h, half_w, half_h },
        { half_w, half_h, half_w, half_h },
    };

    n3_set_depth(true, true);
    n3_bind_texture(&crate_texture);
    n3_set_cull(true);
    for (int i = 0; i < 4; ++i) {
        float x = 0.0f;
        float y = 0.0f;
        n3_set_viewport(rects[i][0], rects[i][1], rects[i][2], rects[i][3]);
        n3_set_scissor(true, rects[i][0], rects[i][1], rects[i][2], rects[i][3]);
        n3_draw_textured_cube_axis_angle_tint(x, y, 0.0f, 0.48f, t * (0.5f + (float)i * 0.16f),
                                              0.25f + (float)i * 0.1f, 1.0f, 0.35f,
                                              (N3Color){ 0.78f + 0.05f * (float)i, 0.88f, 1.0f - 0.08f * (float)i, 1.0f });
        n3_draw_triangle(x, y - 0.78f, 0.02f, 0.25f, 0.20f, t * 0.5f + (float)i,
                         (N3Color){ 1.0f, 0.12f, 0.10f, 1.0f },
                         (N3Color){ 0.12f, 1.0f, 0.28f, 1.0f },
                         (N3Color){ 0.16f, 0.38f, 1.0f, 1.0f });
    }
    n3_set_scissor(false, 0, 0, w, h);
    n3_set_viewport(0, 0, w, h);
    n3_set_cull(false);
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
    lesson_42(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 42 - multiple viewports";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Four isolated viewport scenes";
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
