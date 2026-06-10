#include "nehe_lessons.h"
#include "nehe_world_data.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 9

static N3Texture mud_texture;

static void lesson_10(float t)
{
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
    float yaw = sinf(t * 0.35f) * 0.12f;

    n3_bind_texture(&mud_texture);
    n3_set_camera(0.0f, -0.55f + sinf(t * 1.6f) * 0.04f, -6.0f, 0.0f, yaw, 0.0f);
    for (int i = 0; i < NEHE_WORLD_TRIANGLES; ++i) {
        N3Vertex a = {
            { nehe_world_vertices[i][0][0], nehe_world_vertices[i][0][1], nehe_world_vertices[i][0][2] },
            white, nehe_world_vertices[i][0][3], nehe_world_vertices[i][0][4]
        };
        N3Vertex b = {
            { nehe_world_vertices[i][1][0], nehe_world_vertices[i][1][1], nehe_world_vertices[i][1][2] },
            white, nehe_world_vertices[i][1][3], nehe_world_vertices[i][1][4]
        };
        N3Vertex c = {
            { nehe_world_vertices[i][2][0], nehe_world_vertices[i][2][1], nehe_world_vertices[i][2][2] },
            white, nehe_world_vertices[i][2][3], nehe_world_vertices[i][2][4]
        };
        n3_push_triangle(a, b, c);
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
    lesson_10(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 10 - world walkthrough";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Original mud texture world mesh";
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
