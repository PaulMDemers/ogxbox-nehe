#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>

#define LOCAL_NEHE_LESSON_INDEX 28

static N3Texture raw_texture;
static uint8_t raw_texture_pixels[64 * 64 * 4];

static void lesson_29(float t)
{
    (void)t;
    n3_bind_texture(&raw_texture);
    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);
    n3_set_camera(0.0f, 0.0f, -3.2f, 0.0f, 0.0f, 0.0f);
    n3_draw_textured_quad(-1.25f, 0.0f, 0.0f, 0.52f, 0.52f, 0.0f, (N3Color){ 1.0f, 1.0f, 1.0f, 1.0f });
    n3_draw_textured_quad(0.0f, 0.0f, 0.0f, 0.72f, 0.42f, 0.0f, (N3Color){ 0.75f, 1.0f, 0.85f, 0.92f });
    n3_draw_textured_quad(1.25f, 0.0f, 0.0f, 0.42f, 0.72f, 0.0f, (N3Color){ 1.0f, 0.75f, 0.95f, 0.92f });
}

int nehe_lessons_init(void)
{
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            float nx = ((float)x - 31.5f) / 31.5f;
            float ny = ((float)y - 31.5f) / 31.5f;
            float radius = sqrtf(nx * nx + ny * ny);
            float light = 0.5f + 0.5f * sinf((float)x * 0.42f + sinf((float)y * 0.21f) * 2.5f);
            int i = (y * 64 + x) * 4;
            uint8_t bump = (uint8_t)(55.0f + light * 180.0f);
            uint8_t sphere = (uint8_t)((1.0f - fminf(radius, 1.0f)) * 255.0f);

            raw_texture_pixels[i + 0] = (uint8_t)((x * 4) ^ (y * 2));
            raw_texture_pixels[i + 1] = (uint8_t)(40 + ((x * y) & 191));
            raw_texture_pixels[i + 2] = (uint8_t)(255 - ((x * 3 + y * 5) & 255));
            raw_texture_pixels[i + 3] = 255;
        }
    }

    if (n3_texture_create_rgba(&raw_texture, 64, 64, raw_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&raw_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_29(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 29 - blitter/raw textures";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Procedural raw texture blits";
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
