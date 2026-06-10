#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 11

static N3Texture cube_texture;

static void lesson_12(float t)
{
    static const N3Color boxcol[5] = {
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.5f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 1.0f, 1.0f },
    };
    n3_bind_texture(&cube_texture);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    n3_set_depth(true, true);
    for (int yloop = 5; yloop >= 1; --yloop) {
        for (int xloop = 0; xloop < yloop; ++xloop) {
            float px = ((float)xloop - ((float)yloop - 1.0f) * 0.5f) * 0.58f;
            float py = (3.0f - (float)yloop) * 0.42f;
            (void)t;
            n3_draw_textured_cube_tint(px, py, 0.0f, 0.26f, 0.49f, 0.61f, 0.0f, boxcol[yloop - 1]);
        }
    }
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
    lesson_12(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 12 - display lists";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Original cube texture repeated stack";
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
