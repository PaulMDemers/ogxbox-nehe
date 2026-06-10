#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>

#define LOCAL_NEHE_LESSON_INDEX 19

static N3Texture nehe_texture;

static N3Texture particle_texture;
static uint8_t particle_texture_pixels[32 * 32 * 4];

static N3Texture mask_texture;
static uint8_t mask_texture_pixels[32 * 32 * 4];

static void lesson_20(float t)
{
    float wobble = sinf(t * 1.7f) * 0.28f;
    float roll = t * 70.0f * NEHE_DEG_TO_RAD;
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };

    n3_set_camera(0.0f, 0.0f, -2.8f, 0.0f, 0.0f, 0.0f);
    n3_set_depth(false, false);
    n3_bind_texture(&nehe_texture);
    n3_draw_textured_quad(0.0f, 0.0f, -0.04f, 1.3f, 0.825f, 0.0f, white);

    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_DST_COLOR, NV097_SET_BLEND_FUNC_DFACTOR_V_ZERO);
    n3_bind_texture(&mask_texture);
    n3_draw_textured_quad(wobble, 0.0f, 0.0f, 0.675f, 0.675f, roll, white);

    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    n3_bind_texture(&particle_texture);
    n3_draw_textured_quad(wobble, 0.0f, 0.01f, 0.675f, 0.675f, roll, (N3Color){ 0.35f, 0.9f, 1.0f, 1.0f });
}

int nehe_lessons_init(void)
{
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            float dx = ((float)x - 15.5f) / 15.5f;
            float dy = ((float)y - 15.5f) / 15.5f;
            float dist = sqrtf(dx * dx + dy * dy);
            float glow = 1.0f - dist;
            int i = (y * 32 + x) * 4;

            if (glow < 0.0f) glow = 0.0f;
            glow = glow * glow;
            particle_texture_pixels[i + 0] = 255;
            particle_texture_pixels[i + 1] = 255;
            particle_texture_pixels[i + 2] = 255;
            particle_texture_pixels[i + 3] = (uint8_t)(glow * 255.0f + 0.5f);
            mask_texture_pixels[i + 0] = glow > 0.42f ? 0 : 255;
            mask_texture_pixels[i + 1] = mask_texture_pixels[i + 0];
            mask_texture_pixels[i + 2] = mask_texture_pixels[i + 0];
            mask_texture_pixels[i + 3] = 255;
        }
    }

    if (n3_texture_create_rgba(&nehe_texture, NEHE_ASSET_NEHE_W, NEHE_ASSET_NEHE_H, nehe_asset_nehe_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    if (n3_texture_create_rgba(&particle_texture, 32, 32, particle_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    if (n3_texture_create_rgba(&mask_texture, 32, 32, mask_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&nehe_texture);
    n3_texture_destroy(&particle_texture);
    n3_texture_destroy(&mask_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_20(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 20 - masking";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Two-pass texture masking";
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
