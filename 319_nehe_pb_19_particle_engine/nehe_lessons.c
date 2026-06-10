#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>

#define LOCAL_NEHE_LESSON_INDEX 18

static const N3Color particle_palette[12] = {
    { 1.0f, 0.2f, 0.2f, 1.0f },
    { 1.0f, 0.5f, 0.1f, 1.0f },
    { 1.0f, 0.9f, 0.1f, 1.0f },
    { 0.5f, 1.0f, 0.1f, 1.0f },
    { 0.1f, 1.0f, 0.3f, 1.0f },
    { 0.1f, 1.0f, 0.9f, 1.0f },
    { 0.1f, 0.6f, 1.0f, 1.0f },
    { 0.2f, 0.2f, 1.0f, 1.0f },
    { 0.6f, 0.2f, 1.0f, 1.0f },
    { 1.0f, 0.2f, 1.0f, 1.0f },
    { 1.0f, 0.2f, 0.6f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
};

#define NEHE_QUADRIC_PI 3.14159265358979323846f

#define NEHE_QUADRIC_SLICES 32

#define NEHE_QUADRIC_STACKS 24

#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f

#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

#define NEHE_PARTICLE_COUNT 220

#define NEHE_PARTICLE_LIFE 4.0f

static N3Texture particle_texture;
static uint8_t particle_texture_pixels[32 * 32 * 4];

static float lesson_19_unit(int index, uint32_t salt)
{
    uint32_t h = (uint32_t)index * 1664525u + salt * 1013904223u;

    h ^= h >> 16;
    h *= 2246822519u;
    h ^= h >> 13;
    return (float)(h & 0xffffu) / 65535.0f;
}

static void lesson_19_particle(int index, float t, N3Vec3 *pos, float *size, N3Color *color)
{
    float phase = lesson_19_unit(index, 1u) * NEHE_PARTICLE_LIFE;
    float age = fmodf(t + phase, NEHE_PARTICLE_LIFE);
    float u = age / NEHE_PARTICLE_LIFE;
    float fade = 1.0f - u;
    float spread = lesson_19_unit(index, 2u) * NEHE_QUADRIC_PI * 2.0f;
    float vx = cosf(spread) * (0.25f + lesson_19_unit(index, 3u) * 1.05f);
    float vz = sinf(spread) * (0.15f + lesson_19_unit(index, 4u) * 0.75f);
    float vy = 1.25f + lesson_19_unit(index, 5u) * 1.45f;
    N3Color base = particle_palette[(index + (int)(t * 9.0f)) % 12];

    pos->x = vx * age + sinf(t * 1.7f + (float)index * 0.37f) * 0.12f;
    pos->y = -1.35f + vy * age - 0.68f * age * age;
    pos->z = vz * age;
    *size = 0.11f + fade * 0.20f;
    color->r = base.r;
    color->g = base.g;
    color->b = base.b;
    color->a = fade * fade;
}

static void lesson_19_push_particle(N3Vec3 pos, float size, N3Color color)
{
    N3Vertex a = { { pos.x - size, pos.y - size, pos.z }, color, 0.0f, 0.0f };
    N3Vertex b = { { pos.x + size, pos.y - size, pos.z }, color, 1.0f, 0.0f };
    N3Vertex c = { { pos.x + size, pos.y + size, pos.z }, color, 1.0f, 1.0f };
    N3Vertex d = { { pos.x - size, pos.y + size, pos.z }, color, 0.0f, 1.0f };

    n3_push_quad(a, b, c, d);
}

static void lesson_19(float t)
{
    n3_bind_texture(&particle_texture);
    n3_set_depth(false, false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < NEHE_PARTICLE_COUNT; ++i) {
        N3Vec3 pos;
        float size;
        N3Color color;

        lesson_19_particle(i, t, &pos, &size, &color);
        lesson_19_push_particle(pos, size, color);
    }
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
    lesson_19(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 19 - particle engine";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Additive blended particle system";
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
