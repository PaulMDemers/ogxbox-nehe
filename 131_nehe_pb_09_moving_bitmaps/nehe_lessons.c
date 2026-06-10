#include "nehe_lessons.h"
#include "nehe_starfield.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 8

static N3Texture star_texture;

static void lesson_09(float t)
{
    n3_bind_texture(&star_texture);
    n3_set_depth(false, false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    n3_set_camera(0.0f, 0.0f, NEHE_STAR_Z, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < NEHE_STAR_COUNT; ++i) {
        float x;
        float y;
        float spin;
        N3Color color = { 1.0f, 1.0f, 1.0f, 1.0f };

        nehe_star_state(i, t, &x, &y, &spin);
        nehe_star_color(i, &color.r, &color.g, &color.b);
        n3_draw_textured_quad(x, y, 0.0f, 1.0f, 1.0f, spin * NEHE_DEG_TO_RAD, color);
    }
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&star_texture, NEHE_ASSET_STAR_W, NEHE_ASSET_STAR_H, nehe_asset_star_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&star_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_09(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 09 - moving bitmaps/starfield";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Original star texture field";
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
