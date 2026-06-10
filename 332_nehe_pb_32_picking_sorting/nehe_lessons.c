#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 31

static N3Texture particle_texture;
static uint8_t particle_texture_pixels[32 * 32 * 4];

static void lesson_32(float t)
{
    float cursor = sinf(t * 0.8f);
    N3Color pane[4] = {
        { 1.0f, 0.25f, 0.15f, 0.48f },
        { 0.25f, 1.0f, 0.35f, 0.48f },
        { 0.15f, 0.45f, 1.0f, 0.48f },
        { 1.0f, 0.85f, 0.15f, 0.48f }
    };

    n3_bind_texture(&particle_texture);
    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);
    n3_set_camera(0.0f, 0.0f, -4.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 3; i >= 0; --i) {
        float x = -1.05f + (float)i * 0.7f;
        float y = sinf(t * 0.55f + (float)i) * 0.28f;
        N3Color color = pane[i];
        if (fabsf(cursor - ((float)i - 1.5f) * 0.45f) < 0.28f) {
            color.r = 1.0f;
            color.g = 1.0f;
            color.b = 1.0f;
            color.a = 0.72f;
        }
        n3_draw_textured_quad(x, y, (float)i * -0.12f, 0.46f, 0.72f, t * (0.25f + (float)i * 0.08f), color);
    }
    n3_bind_texture(NULL);
    n3_draw_billboard(cursor * 1.35f, -0.98f, 0.02f, 0.10f, 0.10f, (N3Color){ 1.0f, 1.0f, 1.0f, 0.95f });
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

    if (n3_texture_create_rgba(&particle_texture, 32, 32, particle_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&particle_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_32(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 32 - picking/sorting";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Sorted transparent selection panes";
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
