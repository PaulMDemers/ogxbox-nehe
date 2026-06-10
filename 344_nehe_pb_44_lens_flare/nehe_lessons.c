#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 43

static N3Texture particle_texture;
static uint8_t particle_texture_pixels[32 * 32 * 4];

static void push_plane_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };
    n3_push_quad(va, vb, vc, vd);
}

static void lesson_44(float t)
{
    N3Vec3 sun = { sinf(t * 0.45f) * 0.75f, 0.52f + cosf(t * 0.31f) * 0.18f, -0.2f };
    n3_set_depth(false, false);
    n3_bind_texture(&particle_texture);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    for (int i = 0; i < 9; ++i) {
        float u = (float)i / 8.0f;
        float x = sun.x * (1.0f - u) - sun.x * 0.75f * u;
        float y = sun.y * (1.0f - u) - sun.y * 0.55f * u;
        float size = 0.10f + 0.34f * (1.0f - fabsf(u - 0.5f));
        N3Color c = { 1.0f, 0.62f + 0.35f * u, 0.22f + 0.70f * (1.0f - u), 0.25f };
        n3_draw_textured_quad(x, y, (float)i * 0.002f, size, size, t * 0.16f + (float)i, c);
    }
    n3_bind_texture(NULL);
    push_plane_quad((N3Vec3){ -0.18f, -0.45f, 0.04f }, (N3Vec3){ 0.30f, -0.44f, 0.04f },
                    (N3Vec3){ 0.22f, 0.46f, 0.04f }, (N3Vec3){ -0.32f, 0.34f, 0.04f },
                    (N3Color){ 0.02f, 0.025f, 0.035f, 0.78f });
    n3_set_depth(true, true);
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
    lesson_44(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 44 - lens flare";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Billboard flare and occluder pass";
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
