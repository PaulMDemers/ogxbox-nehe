#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 37

static N3Texture nehe_texture;

static N3Texture tim_texture;

static N3Texture cube_texture;

static N3Texture raw_texture;
static uint8_t raw_texture_pixels[64 * 64 * 4];

static void push_plane_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };
    n3_push_quad(va, vb, vc, vd);
}

static void lesson_38(float t)
{
    float spin = t * 0.18f;
    N3Texture *textures[4] = { &nehe_texture, &cube_texture, &tim_texture, &raw_texture };
    N3Color colors[4] = {
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.82f, 0.82f, 1.0f },
        { 0.82f, 1.0f, 0.90f, 1.0f },
        { 0.82f, 0.90f, 1.0f, 1.0f }
    };

    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -3.7f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i) {
        float x = (i & 1) ? 0.82f : -0.82f;
        float y = (i & 2) ? -0.58f : 0.58f;
        n3_bind_texture(textures[i]);
        n3_draw_textured_quad(x, y, 0.0f, 0.48f, 0.36f, spin * (float)(i + 1), colors[i]);
    }
    n3_bind_texture(NULL);
    push_plane_quad((N3Vec3){ -1.75f, 0.0f, 0.04f }, (N3Vec3){ 1.75f, 0.0f, 0.04f },
                    (N3Vec3){ 1.75f, 0.05f, 0.04f }, (N3Vec3){ -1.75f, 0.05f, 0.04f },
                    (N3Color){ 0.24f, 0.28f, 0.34f, 1.0f });
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

    if (n3_texture_create_rgba(&nehe_texture, NEHE_ASSET_NEHE_W, NEHE_ASSET_NEHE_H, nehe_asset_nehe_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    if (n3_texture_create_rgba(&tim_texture, NEHE_ASSET_TIM_W, NEHE_ASSET_TIM_H, nehe_asset_tim_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    if (n3_texture_create_rgba(&cube_texture, NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    if (n3_texture_create_rgba(&raw_texture, 64, 64, raw_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&nehe_texture);
    n3_texture_destroy(&tim_texture);
    n3_texture_destroy(&cube_texture);
    n3_texture_destroy(&raw_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_38(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 38 - resource textures";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Packed resource texture atlas";
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
