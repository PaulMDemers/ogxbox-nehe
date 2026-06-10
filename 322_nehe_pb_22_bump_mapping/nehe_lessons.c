#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>

#define LOCAL_NEHE_LESSON_INDEX 21

static N3Texture bump_texture;
static uint8_t bump_texture_pixels[64 * 64 * 4];

static void lesson_22(float t)
{
    const N3Color bump_faces[6] = {
        { 0.85f, 0.95f, 1.15f, 1.0f }, { 0.45f, 0.55f, 0.75f, 1.0f },
        { 1.05f, 1.05f, 1.2f, 1.0f }, { 0.35f, 0.45f, 0.65f, 1.0f },
        { 0.65f, 0.85f, 1.1f, 1.0f }, { 0.5f, 0.65f, 0.9f, 1.0f },
    };
    n3_bind_texture(&bump_texture);
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    n3_draw_textured_cube_axis_angle_face_colors(0.0f, 0.0f, 0.0f, 1.0f,
                                                 t * 58.0f * NEHE_DEG_TO_RAD, 0.45f, 1.0f, 0.25f,
                                                 bump_faces);
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

            bump_texture_pixels[i + 0] = (uint8_t)(bump / 3);
            bump_texture_pixels[i + 1] = (uint8_t)(90 + bump / 2);
            bump_texture_pixels[i + 2] = bump;
            bump_texture_pixels[i + 3] = 255;

        }
    }

    if (n3_texture_create_rgba(&bump_texture, 64, 64, bump_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&bump_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_22(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 22 - bump mapping";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Embossed multitexture-style cube";
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
