#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>

#define LOCAL_NEHE_LESSON_INDEX 40

static N3Texture crate_texture;

static N3Texture particle_texture;
static uint8_t particle_texture_pixels[32 * 32 * 4];

static void lesson_41(float t)
{
    n3_set_depth(true, false);
    n3_bind_texture(&particle_texture);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    for (int i = 0; i < 22; ++i) {
        float z = -1.55f + (float)i * 0.14f;
        float sway = sinf(t * 0.9f + (float)i * 0.37f) * 0.18f;
        float size = 0.55f + (float)i * 0.035f;
        float alpha = 0.10f + 0.10f * (1.0f - (float)i / 22.0f);
        n3_draw_textured_quad(sway, 0.05f + sinf(t * 1.3f + (float)i) * 0.08f, z, size, size * 0.72f,
                              t * 0.18f + (float)i * 0.11f, (N3Color){ 0.36f, 0.62f, 1.0f, alpha });
    }
    n3_bind_texture(&crate_texture);
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);
    n3_draw_textured_cube_axis_angle_tint(0.0f, -0.12f, -0.35f, 0.74f, t * 0.65f, 0.4f, 1.0f, 0.2f,
                                          (N3Color){ 0.85f, 0.94f, 1.0f, 1.0f });
    n3_set_cull(false);
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
        }
    }

    if (n3_texture_create_rgba(&crate_texture, NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    if (n3_texture_create_rgba(&particle_texture, 32, 32, particle_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&crate_texture);
    n3_texture_destroy(&particle_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_41(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 41 - volumetric fog";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Layered translucent fog volume";
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
