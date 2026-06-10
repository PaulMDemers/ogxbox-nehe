#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 23

static N3Texture cube_texture;

static void lesson_24(float t)
{
    N3Color colors[4] = {
        { 1.0f, 0.35f, 0.25f, 1.0f },
        { 0.25f, 1.0f, 0.45f, 1.0f },
        { 0.25f, 0.55f, 1.0f, 1.0f },
        { 1.0f, 0.9f, 0.25f, 1.0f },
    };
    int w = n3_back_buffer_width();
    int h = n3_back_buffer_height();

    n3_bind_texture(&cube_texture);
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_set_camera(0.0f, 0.0f, -4.2f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i) {
        int sx = (i & 1) ? w / 2 : 0;
        int sy = (i & 2) ? h / 2 : 0;
        n3_set_scissor(true, sx, sy, w / 2, h / 2);
        n3_draw_textured_cube_axis_angle_tint((i & 1) ? 0.85f : -0.85f,
                                              (i & 2) ? -0.62f : 0.62f,
                                              0.0f, 0.52f,
                                              t * (35.0f + (float)i * 11.0f) * NEHE_DEG_TO_RAD,
                                              0.35f + (float)i * 0.15f, 1.0f, 0.25f,
                                              colors[i]);
    }
    n3_set_scissor(false, 0, 0, w, h);
    n3_set_cull(false);
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&cube_texture, NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&cube_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_24(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 24 - tokens/scissor/TGA";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Scissor panes and TGA-style texture";
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
