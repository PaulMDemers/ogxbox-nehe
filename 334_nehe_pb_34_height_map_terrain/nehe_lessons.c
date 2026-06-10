#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 33

static N3Texture mud_texture;

static float lesson_34_height(float x, float z, float t)
{
    return sinf(x * 2.3f + t * 0.35f) * 0.22f + cosf(z * 2.1f - t * 0.25f) * 0.18f;
}

static void lesson_34(float t)
{
    const int cells = 18;
    const float span = 4.2f;
    const float step = span / (float)cells;

    n3_bind_texture(&mud_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.05f, -5.6f, -0.48f, t * 0.12f, 0.0f);
    for (int z = 0; z < cells; ++z) {
        for (int x = 0; x < cells; ++x) {
            float x0 = -span * 0.5f + (float)x * step;
            float x1 = x0 + step;
            float z0 = -span * 0.5f + (float)z * step;
            float z1 = z0 + step;
            float h0 = lesson_34_height(x0, z0, t);
            float h1 = lesson_34_height(x1, z0, t);
            float h2 = lesson_34_height(x1, z1, t);
            float h3 = lesson_34_height(x0, z1, t);
            float shade = 0.55f + (h0 + h1 + h2 + h3) * 0.28f;
            N3Color color = { shade * 0.82f, shade, shade * 0.72f, 1.0f };
            N3Vertex a = { { x0, h0 - 0.65f, z0 }, color, (float)x / 3.0f, (float)z / 3.0f };
            N3Vertex b = { { x1, h1 - 0.65f, z0 }, color, (float)(x + 1) / 3.0f, (float)z / 3.0f };
            N3Vertex c = { { x1, h2 - 0.65f, z1 }, color, (float)(x + 1) / 3.0f, (float)(z + 1) / 3.0f };
            N3Vertex d = { { x0, h3 - 0.65f, z1 }, color, (float)x / 3.0f, (float)(z + 1) / 3.0f };
            n3_push_quad(a, b, c, d);
        }
    }
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&mud_texture, NEHE_ASSET_MUD_W, NEHE_ASSET_MUD_H, nehe_asset_mud_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&mud_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_34(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 34 - height-map terrain";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Procedural landscape height field";
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
