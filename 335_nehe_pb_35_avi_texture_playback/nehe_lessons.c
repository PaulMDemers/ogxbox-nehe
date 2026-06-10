#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 34

static N3Texture video_texture;
static uint8_t video_texture_pixels[64 * 64 * 4];

static void fill_video_texture_pixels(float t)
{
    int frame = (int)(t * 12.0f);

    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            int i = (y * 64 + x) * 4;
            float nx = ((float)x - 31.5f) / 31.5f;
            float ny = ((float)y - 31.5f) / 31.5f;
            float rings = 0.5f + 0.5f * sinf((nx * nx + ny * ny) * 18.0f - t * 5.0f);
            float bars = (((x + frame * 3) / 8 + (y / 12)) & 1) ? 1.0f : 0.35f;
            uint8_t r = (uint8_t)(40.0f + 200.0f * rings);
            uint8_t g = (uint8_t)(50.0f + 180.0f * bars);
            uint8_t b = (uint8_t)(90.0f + 120.0f * (1.0f - rings) + (float)((x + frame) & 15) * 2.0f);

            video_texture_pixels[i + 0] = r;
            video_texture_pixels[i + 1] = g;
            video_texture_pixels[i + 2] = b;
            video_texture_pixels[i + 3] = 255;
        }
    }
}

static void update_native_texture_from_rgba(N3Texture *texture, const uint8_t *rgba, uint16_t width, uint16_t height)
{
    if (texture == NULL || texture->addr == NULL || rgba == NULL) {
        return;
    }

    uint32_t *pixels = (uint32_t *)texture->addr;
    uint32_t native_width = texture->width;
    uint32_t native_height = texture->height;
    for (uint32_t y = 0; y < native_height; ++y) {
        uint32_t src_y = y < height ? y : (uint32_t)height - 1u;
        for (uint32_t x = 0; x < native_width; ++x) {
            uint32_t src_x = x < width ? x : (uint32_t)width - 1u;
            uint32_t src = (src_y * (uint32_t)width + src_x) * 4u;
            uint8_t r = rgba[src + 0];
            uint8_t g = rgba[src + 1];
            uint8_t b = rgba[src + 2];
            uint8_t a = rgba[src + 3];
            pixels[y * native_width + x] = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        }
    }
}

static void push_plane_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };
    n3_push_quad(va, vb, vc, vd);
}

static void lesson_35(float t)
{
    fill_video_texture_pixels(t);
    update_native_texture_from_rgba(&video_texture, video_texture_pixels, 64, 64);

    n3_bind_texture(&video_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -4.0f, 0.0f, 0.0f, 0.0f);
    n3_draw_textured_quad(0.0f, 0.10f, 0.0f, 1.35f, 1.0f, sinf(t * 0.7f) * 0.08f, (N3Color){ 1.0f, 1.0f, 1.0f, 1.0f });
    n3_bind_texture(NULL);
    push_plane_quad((N3Vec3){ -1.55f, -1.05f, 0.03f }, (N3Vec3){ 1.55f, -1.05f, 0.03f },
                    (N3Vec3){ 1.55f, -0.92f, 0.03f }, (N3Vec3){ -1.55f, -0.92f, 0.03f },
                    (N3Color){ 0.12f, 0.18f, 0.22f, 1.0f });
    push_plane_quad((N3Vec3){ -1.42f, -1.02f, 0.04f },
                    (N3Vec3){ -1.42f + fmodf(t * 0.42f, 2.84f), -1.02f, 0.04f },
                    (N3Vec3){ -1.42f + fmodf(t * 0.42f, 2.84f), -0.95f, 0.04f },
                    (N3Vec3){ -1.42f, -0.95f, 0.04f },
                    (N3Color){ 0.15f, 0.75f, 1.0f, 1.0f });
}

int nehe_lessons_init(void)
{
    fill_video_texture_pixels(0.0f);
    if (n3_texture_create_rgba(&video_texture, 64, 64, video_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&video_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_35(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 35 - AVI texture playback";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Procedural video texture stream";
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
