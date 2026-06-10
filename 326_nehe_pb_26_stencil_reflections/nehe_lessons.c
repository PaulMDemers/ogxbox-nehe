#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 25

static N3Texture cube_texture;

static void push_plane_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };
    n3_push_quad(va, vb, vc, vd);
}

static void lesson_26(float t)
{
    float angle = t * 38.0f * NEHE_DEG_TO_RAD;
    N3Color floor_color = { 0.45f, 0.62f, 0.76f, 0.58f };

    n3_set_camera(0.0f, 0.35f, -6.2f, -0.20f, 0.0f, 0.0f);
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);

    n3_bind_texture(&cube_texture);
    n3_draw_textured_cube_axis_angle_tint(0.0f, -1.55f, 0.0f, 0.72f, -angle, 0.4f, 1.0f, 0.0f,
                                          (N3Color){ 0.35f, 0.55f, 0.75f, 0.38f });
    n3_set_cull(false);
    push_plane_quad((N3Vec3){ -2.4f, -0.85f, -1.9f }, (N3Vec3){ 2.4f, -0.85f, -1.9f },
                    (N3Vec3){ 2.4f, -0.85f, 1.9f }, (N3Vec3){ -2.4f, -0.85f, 1.9f }, floor_color);
    n3_set_cull(true);
    n3_draw_textured_cube_axis_angle_tint(0.0f, 0.35f, 0.0f, 0.72f, angle, 0.4f, 1.0f, 0.0f,
                                          (N3Color){ 1.0f, 1.0f, 1.0f, 1.0f });
    n3_set_cull(false);
}

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&cube_texture, NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&cube_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_26(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 26 - stencil reflections";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Reflected cube over clipped floor";
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
