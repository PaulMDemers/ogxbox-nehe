#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 5

static N3Texture nehe_texture;

static void lesson_06(float t)
{
    n3_bind_texture(&nehe_texture);
    n3_set_cull(true);
    n3_draw_textured_cube_axis_angle(0.0f, 0.0f, 0.0f, 1.0f, t * 45.0f * NEHE_DEG_TO_RAD, 1.0f, 1.0f, 0.0f);
    n3_set_cull(false);
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&nehe_texture, NEHE_ASSET_NEHE_W, NEHE_ASSET_NEHE_H, nehe_asset_nehe_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&nehe_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_06(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 06 - texture mapping";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Original NeHe.bmp texture on cube faces";
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
