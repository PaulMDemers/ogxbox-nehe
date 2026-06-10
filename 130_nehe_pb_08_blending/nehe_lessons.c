#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 7

static N3Texture glass_texture;

static void lesson_08(float t)
{
    n3_bind_texture(&glass_texture);
    n3_draw_textured_cube_axis_angle(0.0f, 0.0f, 0.0f, 1.0f, t * 40.0f * NEHE_DEG_TO_RAD, 1.0f, 1.0f, 0.0f);
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&glass_texture, NEHE_ASSET_GLASS_W, NEHE_ASSET_GLASS_H, nehe_asset_glass_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&glass_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_08(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 08 - blending";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Original glass texture alpha cube";
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
