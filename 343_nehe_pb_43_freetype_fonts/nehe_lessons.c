#include "nehe_lessons.h"
#include "nehe_texture_font.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stddef.h>

#define LOCAL_NEHE_LESSON_INDEX 42

static N3Texture font_texture;
static uint8_t font_texture_pixels[NEHE_TEXTURE_FONT_SIZE * NEHE_TEXTURE_FONT_SIZE * 4];

static void lesson_21_line(float x0, float y0, float x1, float y1, float width, N3Color color)
{
    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = sqrtf(dx * dx + dy * dy);
    float nx;
    float ny;
    N3Vertex a, b, c, d;

    if (len < 0.0001f) {
        return;
    }
    nx = -dy / len * width;
    ny = dx / len * width;
    a = (N3Vertex){ { x0 + nx, y0 + ny, 0.0f }, color, 0.0f, 0.0f };
    b = (N3Vertex){ { x1 + nx, y1 + ny, 0.0f }, color, 1.0f, 0.0f };
    c = (N3Vertex){ { x1 - nx, y1 - ny, 0.0f }, color, 1.0f, 1.0f };
    d = (N3Vertex){ { x0 - nx, y0 - ny, 0.0f }, color, 0.0f, 1.0f };
    n3_push_quad(a, b, c, d);
}

static void lesson_43(float t)
{
    n3_set_depth(true, true);
    n3_bind_texture(&font_texture);
    for (int i = 0; i < 7; ++i) {
        float x = -1.35f + (float)i * 0.45f;
        float y = sinf(t * 1.2f + (float)i * 0.7f) * 0.18f;
        float z = -0.05f * (float)i;
        N3Color tint = { 0.45f + 0.07f * (float)i, 0.78f, 1.0f, 1.0f };
        n3_draw_textured_quad(x, y + 0.20f, z, 0.17f, 0.22f, sinf(t + (float)i) * 0.08f, tint);
        n3_draw_textured_quad(x + 0.04f, y + 0.14f, z - 0.06f, 0.17f, 0.22f, sinf(t + (float)i) * 0.08f,
                              (N3Color){ 0.15f, 0.22f, 0.32f, 1.0f });
    }
    n3_bind_texture(NULL);
    lesson_21_line(-1.55f, -0.42f, 1.55f, -0.42f, 0.018f, (N3Color){ 0.78f, 0.88f, 1.0f, 1.0f });
}

int nehe_lessons_init(void)
{
    nehe_texture_font_fill_rgba(font_texture_pixels);
    if (n3_texture_create_rgba(&font_texture, NEHE_TEXTURE_FONT_SIZE, NEHE_TEXTURE_FONT_SIZE, font_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&font_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_43(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 43 - FreeType fonts";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Smooth glyph extrusion stand-in";
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
