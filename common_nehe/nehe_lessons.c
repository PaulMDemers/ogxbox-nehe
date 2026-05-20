#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_bitmap_font.h"
#include "nehe_native.h"
#include "nehe_outline_font.h"
#include "nehe_scene.h"
#include "nehe_starfield.h"
#include "nehe_texture_font.h"
#include "nehe_world_data.h"

#include <math.h>
#include <pbkit/pbkit.h>

static const N3Color red = { 1.0f, 0.05f, 0.05f, 1.0f };
static const N3Color green = { 0.08f, 0.9f, 0.2f, 1.0f };
static const N3Color blue = { 0.1f, 0.25f, 1.0f, 1.0f };

static N3Color face[6] = {
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.5f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 1.0f },
};

static N3Texture nehe_texture;
static N3Texture crate_texture;
static N3Texture glass_texture;
static N3Texture star_texture;
static N3Texture mud_texture;
static N3Texture tim_texture;
static N3Texture cube_texture;
static N3Texture font_texture;
static N3Texture particle_texture;
static N3Texture mask_texture;
static N3Texture bump_texture;
static N3Texture sphere_texture;
static N3Texture raw_texture;
static N3Texture video_texture;
static uint8_t font_texture_pixels[NEHE_TEXTURE_FONT_SIZE * NEHE_TEXTURE_FONT_SIZE * 4];
static uint8_t particle_texture_pixels[32 * 32 * 4];
static uint8_t mask_texture_pixels[32 * 32 * 4];
static uint8_t bump_texture_pixels[64 * 64 * 4];
static uint8_t sphere_texture_pixels[64 * 64 * 4];
static uint8_t raw_texture_pixels[64 * 64 * 4];
static uint8_t video_texture_pixels[64 * 64 * 4];

static const char *titles[NEHE_LESSON_COUNT] = {
    "NeHe 01 - window / clear",
    "NeHe 02 - first polygons",
    "NeHe 03 - color",
    "NeHe 04 - rotation",
    "NeHe 05 - 3D shapes",
    "NeHe 06 - texture mapping",
    "NeHe 07 - light/filter/input",
    "NeHe 08 - blending",
    "NeHe 09 - moving bitmaps/starfield",
    "NeHe 10 - world walkthrough",
    "NeHe 11 - waving texture mesh",
    "NeHe 12 - display lists",
    "NeHe 13 - bitmap fonts",
    "NeHe 14 - outline fonts",
    "NeHe 15 - texture mapped outline fonts",
    "NeHe 16 - cool looking fog",
    "NeHe 17 - 2D texture font",
    "NeHe 18 - quadrics",
    "NeHe 19 - particle engine",
    "NeHe 20 - masking",
    "NeHe 21 - lines/timing/ortho",
    "NeHe 22 - bump mapping",
    "NeHe 23 - sphere mapping",
    "NeHe 24 - tokens/scissor/TGA",
    "NeHe 25 - morphing/loading objects",
    "NeHe 26 - stencil reflections",
    "NeHe 27 - shadows",
    "NeHe 28 - Bezier patches",
    "NeHe 29 - blitter/raw textures",
    "NeHe 30 - collision detection",
    "NeHe 31 - model loading",
    "NeHe 32 - picking/sorting",
    "NeHe 33 - TGA variants",
    "NeHe 34 - height-map terrain",
    "NeHe 35 - AVI texture playback",
    "NeHe 36 - radial blur",
    "NeHe 37 - cel shading",
    "NeHe 38 - resource textures",
    "NeHe 39 - physics simulation",
    "NeHe 40 - rope physics",
    "NeHe 41 - volumetric fog",
    "NeHe 42 - multiple viewports",
    "NeHe 43 - FreeType fonts",
    "NeHe 44 - lens flare",
    "NeHe 45 - vertex buffers",
    "NeHe 46 - antialiasing",
    "NeHe 47 - vertex shader",
    "NeHe 48 - arcball rotation"
};

static const char *details[NEHE_LESSON_COUNT] = {
    "Native pbkit/NV2A frame setup",
    "Triangle and quad placement",
    "Smooth vertex color and flat face color",
    "Animated transforms",
    "Pyramid and cube made from triangles",
    "Original NeHe.bmp texture on cube faces",
    "Original crate texture with lighting",
    "Original glass texture alpha cube",
    "Original star texture field",
    "Original mud texture world mesh",
    "Original tim texture waving mesh",
    "Original cube texture repeated stack",
    "Animated bitmap-style text",
    "3D outline-style text",
    "Texture mapped outline symbol",
    "Fog over textured crates",
    "Texture atlas font overlay",
    "Cube and GLU-style quadric shapes",
    "Additive blended particle system",
    "Two-pass texture masking",
    "Timed antialiased line patterns",
    "Embossed multitexture-style cube",
    "Environment mapped sphere",
    "Scissor panes and TGA-style texture",
    "Point-cloud morphing between objects",
    "Reflected cube over clipped floor",
    "Planar shadow projection",
    "Textured Bezier-style patch",
    "Procedural raw texture blits",
    "Moving sphere/cube collision volume",
    "Loaded-style low-poly model mesh",
    "Sorted transparent selection panes",
    "Texture loader format comparison",
    "Procedural landscape height field",
    "Procedural video texture stream",
    "Layered render-to-texture style blur",
    "Quantized lighting and outlines",
    "Packed resource texture atlas",
    "Spring-mass field simulation",
    "Segmented rope simulation",
    "Layered translucent fog volume",
    "Four isolated viewport scenes",
    "Smooth glyph extrusion stand-in",
    "Billboard flare and occluder pass",
    "Streaming vertex field batches",
    "Subpixel line and edge smoothing",
    "Animated shader-style deformation",
    "Trackball-controlled object rotation"
};

static int clamp_lesson(int lesson)
{
    if (lesson < 0) {
        return 0;
    }
    if (lesson >= NEHE_LESSON_COUNT) {
        return NEHE_LESSON_COUNT - 1;
    }
    return lesson;
}

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

int nehe_lessons_init(void)
{
    fill_video_texture_pixels(0.0f);
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
            mask_texture_pixels[i + 0] = glow > 0.42f ? 0 : 255;
            mask_texture_pixels[i + 1] = mask_texture_pixels[i + 0];
            mask_texture_pixels[i + 2] = mask_texture_pixels[i + 0];
            mask_texture_pixels[i + 3] = 255;
        }
    }

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

            sphere_texture_pixels[i + 0] = (uint8_t)(20 + sphere / 5);
            sphere_texture_pixels[i + 1] = (uint8_t)(80 + sphere / 2);
            sphere_texture_pixels[i + 2] = (uint8_t)(140 + sphere / 3);
            sphere_texture_pixels[i + 3] = 255;

            raw_texture_pixels[i + 0] = (uint8_t)((x * 4) ^ (y * 2));
            raw_texture_pixels[i + 1] = (uint8_t)(40 + ((x * y) & 191));
            raw_texture_pixels[i + 2] = (uint8_t)(255 - ((x * 3 + y * 5) & 255));
            raw_texture_pixels[i + 3] = 255;
        }
    }

    if (n3_texture_create_rgba(&nehe_texture, NEHE_ASSET_NEHE_W, NEHE_ASSET_NEHE_H, nehe_asset_nehe_rgba) != 0) {
        return -1;
    }
    if (n3_texture_create_rgba(&crate_texture, NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba) != 0 ||
        n3_texture_create_rgba(&glass_texture, NEHE_ASSET_GLASS_W, NEHE_ASSET_GLASS_H, nehe_asset_glass_rgba) != 0 ||
        n3_texture_create_rgba(&star_texture, NEHE_ASSET_STAR_W, NEHE_ASSET_STAR_H, nehe_asset_star_rgba) != 0 ||
        n3_texture_create_rgba(&mud_texture, NEHE_ASSET_MUD_W, NEHE_ASSET_MUD_H, nehe_asset_mud_rgba) != 0 ||
        n3_texture_create_rgba(&tim_texture, NEHE_ASSET_TIM_W, NEHE_ASSET_TIM_H, nehe_asset_tim_rgba) != 0 ||
        n3_texture_create_rgba(&cube_texture, NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba) != 0) {
        n3_texture_destroy(&nehe_texture);
        n3_texture_destroy(&crate_texture);
        n3_texture_destroy(&glass_texture);
        n3_texture_destroy(&star_texture);
        n3_texture_destroy(&mud_texture);
        n3_texture_destroy(&tim_texture);
        n3_texture_destroy(&cube_texture);
        return -1;
    }
    nehe_texture_font_fill_rgba(font_texture_pixels);
    if (n3_texture_create_rgba(&font_texture, NEHE_TEXTURE_FONT_SIZE, NEHE_TEXTURE_FONT_SIZE, font_texture_pixels) != 0 ||
        n3_texture_create_rgba(&particle_texture, 32, 32, particle_texture_pixels) != 0 ||
        n3_texture_create_rgba(&mask_texture, 32, 32, mask_texture_pixels) != 0 ||
        n3_texture_create_rgba(&bump_texture, 64, 64, bump_texture_pixels) != 0 ||
        n3_texture_create_rgba(&sphere_texture, 64, 64, sphere_texture_pixels) != 0 ||
        n3_texture_create_rgba(&raw_texture, 64, 64, raw_texture_pixels) != 0 ||
        n3_texture_create_rgba(&video_texture, 64, 64, video_texture_pixels) != 0) {
        n3_texture_destroy(&nehe_texture);
        n3_texture_destroy(&crate_texture);
        n3_texture_destroy(&glass_texture);
        n3_texture_destroy(&star_texture);
        n3_texture_destroy(&mud_texture);
        n3_texture_destroy(&tim_texture);
        n3_texture_destroy(&cube_texture);
        n3_texture_destroy(&font_texture);
        n3_texture_destroy(&particle_texture);
        n3_texture_destroy(&mask_texture);
        n3_texture_destroy(&bump_texture);
        n3_texture_destroy(&sphere_texture);
        n3_texture_destroy(&raw_texture);
        n3_texture_destroy(&video_texture);
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&nehe_texture);
    n3_texture_destroy(&crate_texture);
    n3_texture_destroy(&glass_texture);
    n3_texture_destroy(&star_texture);
    n3_texture_destroy(&mud_texture);
    n3_texture_destroy(&tim_texture);
    n3_texture_destroy(&cube_texture);
    n3_texture_destroy(&font_texture);
    n3_texture_destroy(&particle_texture);
    n3_texture_destroy(&mask_texture);
    n3_texture_destroy(&bump_texture);
    n3_texture_destroy(&sphere_texture);
    n3_texture_destroy(&raw_texture);
    n3_texture_destroy(&video_texture);
}

static void lesson_01(float t)
{
    (void)t;
}

static void lesson_02(float t)
{
    (void)t;
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
    n3_draw_triangle(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, white, white, white);
    n3_draw_quad(1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, white);
}

static void lesson_03(float t)
{
    (void)t;
    n3_draw_triangle(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, red, green, blue);
    n3_draw_quad(1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, (N3Color){ 0.5f, 0.5f, 1.0f, 1.0f });
}

static void lesson_04(float t)
{
    n3_draw_triangle(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, t * 70.0f * NEHE_DEG_TO_RAD, red, green, blue);
    n3_draw_quad(1.5f, 0.0f, 0.0f, 1.0f, 1.0f, -t * 55.0f * NEHE_DEG_TO_RAD, (N3Color){ 0.5f, 0.5f, 1.0f, 1.0f });
}

static void lesson_05(float t)
{
    n3_draw_pyramid(-1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, t * 0.20944f, 0.0f);
    n3_set_cull(true);
    n3_draw_cube_axis_angle(1.5f, 0.0f, -1.0f, 2.0f, 2.0f, 2.0f, -t * 0.15708f, 1.0f, 1.0f, 1.0f, face);
    n3_set_cull(false);
}

static void lesson_06(float t)
{
    n3_bind_texture(&nehe_texture);
    n3_set_cull(true);
    n3_draw_textured_cube_axis_angle(0.0f, 0.0f, 0.0f, 1.0f, t * 45.0f * NEHE_DEG_TO_RAD, 1.0f, 1.0f, 0.0f);
    n3_set_cull(false);
}

static N3Vec3 rotate_axis_angle(N3Vec3 p, float angle, float ax, float ay, float az)
{
    float len = sqrtf(ax * ax + ay * ay + az * az);
    if (len <= 0.000001f) {
        return p;
    }

    ax /= len;
    ay /= len;
    az /= len;

    float c = cosf(angle);
    float s = sinf(angle);
    float one_c = 1.0f - c;
    return (N3Vec3){
        (c + ax * ax * one_c) * p.x + (ax * ay * one_c - az * s) * p.y + (ax * az * one_c + ay * s) * p.z,
        (ay * ax * one_c + az * s) * p.x + (c + ay * ay * one_c) * p.y + (ay * az * one_c - ax * s) * p.z,
        (az * ax * one_c - ay * s) * p.x + (az * ay * one_c + ax * s) * p.y + (c + az * az * one_c) * p.z,
    };
}

static void lesson_07(float t)
{
    static const N3Vec3 normals[6] = {
        { 0.0f, 1.0f, 0.0f }, { 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f,-1.0f }, {-1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
    };
    N3Color lit[6];
    float angle = t * 42.0f * NEHE_DEG_TO_RAD;

    for (int i = 0; i < 6; ++i) {
        N3Vec3 normal = rotate_axis_angle(normals[i], angle, 1.0f, 1.0f, 0.0f);
        float diffuse = normal.z > 0.0f ? normal.z : 0.0f;
        float factor = 0.2f + diffuse * 0.8f;
        lit[i] = (N3Color){ factor, factor, factor, 1.0f };
    }

    n3_bind_texture(&crate_texture);
    n3_set_cull(true);
    n3_draw_textured_cube_axis_angle_face_colors(0.0f, 0.0f, 0.0f, 1.0f, angle, 1.0f, 1.0f, 0.0f, lit);
    n3_set_cull(false);
}

static void lesson_08(float t)
{
    n3_bind_texture(&glass_texture);
    n3_draw_textured_cube_axis_angle(0.0f, 0.0f, 0.0f, 1.0f, t * 40.0f * NEHE_DEG_TO_RAD, 1.0f, 1.0f, 0.0f);
}

static void lesson_09(float t)
{
    n3_bind_texture(&star_texture);
    n3_set_depth(false, false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    n3_set_camera(0.0f, 0.0f, NEHE_STAR_Z, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < NEHE_STAR_COUNT; ++i) {
        float x;
        float y;
        float spin;
        N3Color color = { 1.0f, 1.0f, 1.0f, 1.0f };

        nehe_star_state(i, t, &x, &y, &spin);
        nehe_star_color(i, &color.r, &color.g, &color.b);
        n3_draw_textured_quad(x, y, 0.0f, 1.0f, 1.0f, spin * NEHE_DEG_TO_RAD, color);
    }
}

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

static void lesson_11(float t)
{
    N3Color color = { 1.0f, 1.0f, 1.0f, 1.0f };
    int cells = 22;
    float scale = 4.4f / (float)cells;

    n3_bind_texture(&tim_texture);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    for (int y = 0; y < cells; ++y) {
        for (int x = 0; x < cells; ++x) {
            float x0 = ((float)x - cells * 0.5f) * scale;
            float x1 = ((float)(x + 1) - cells * 0.5f) * scale;
            float y0 = ((float)y - cells * 0.5f) * scale;
            float y1 = ((float)(y + 1) - cells * 0.5f) * scale;
            float z00 = sinf(t * 2.2f + x0 * 2.0f + y0 * 1.4f) * 0.25f;
            float z10 = sinf(t * 2.2f + x1 * 2.0f + y0 * 1.4f) * 0.25f;
            float z11 = sinf(t * 2.2f + x1 * 2.0f + y1 * 1.4f) * 0.25f;
            float z01 = sinf(t * 2.2f + x0 * 2.0f + y1 * 1.4f) * 0.25f;
            float u0 = (float)x / (float)cells;
            float u1 = (float)(x + 1) / (float)cells;
            float v0 = (float)y / (float)cells;
            float v1 = (float)(y + 1) / (float)cells;
            N3Vertex a = { { x0, y0, z00 }, color, u0, v0 };
            N3Vertex b = { { x1, y0, z10 }, color, u1, v0 };
            N3Vertex c = { { x1, y1, z11 }, color, u1, v1 };
            N3Vertex d = { { x0, y1, z01 }, color, u0, v1 };
            n3_push_quad(a, b, c, d);
        }
    }
}

static void lesson_12(float t)
{
    static const N3Color boxcol[5] = {
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.5f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 1.0f, 1.0f },
    };
    n3_bind_texture(&cube_texture);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    n3_set_depth(true, true);
    for (int yloop = 5; yloop >= 1; --yloop) {
        for (int xloop = 0; xloop < yloop; ++xloop) {
            float px = ((float)xloop - ((float)yloop - 1.0f) * 0.5f) * 0.58f;
            float py = (3.0f - (float)yloop) * 0.42f;
            (void)t;
            n3_draw_textured_cube_tint(px, py, 0.0f, 0.26f, 0.49f, 0.61f, 0.0f, boxcol[yloop - 1]);
        }
    }
}

static void draw_bitmap_text_native(const char *text, float x, float y, float cell, N3Color color)
{
    float cursor = x;

    n3_set_depth(false, false);
    for (const char *p = text; *p != '\0'; ++p) {
        const uint8_t *rows = nehe_font_rows(*p);
        for (int row = 0; row < NEHE_FONT_ROWS; ++row) {
            for (int col = 0; col < NEHE_FONT_COLS; ++col) {
                if (nehe_font_row_has_pixel(rows, row, col)) {
                    n3_draw_quad(cursor + ((float)col + 0.5f) * cell,
                                 y - ((float)row + 0.5f) * cell,
                                 0.0f,
                                 cell * 0.43f,
                                 cell * 0.43f,
                                 0.0f,
                                 color);
                }
            }
        }
        cursor += cell * 6.0f;
    }
    n3_set_depth(true, true);
}

static void lesson_13(float t)
{
    float cnt1 = t * 0.60f;
    float cnt2 = t * 0.486f;
    N3Color color = {
        fmaxf(0.0f, cosf(cnt1)),
        fmaxf(0.0f, sinf(cnt2)),
        fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(cnt1 + cnt2))),
        1.0f
    };
    float x = -0.80f + 0.35f * cosf(cnt1);
    float y = 0.08f + 0.35f * sinf(cnt2);

    n3_set_camera(0.0f, 0.0f, -3.0f, 0.0f, 0.0f, 0.0f);
    draw_bitmap_text_native("OpenGL With NeHe", x, y, 0.017f, color);
}

static N3Vec3 rotate_lesson_14_point(float x, float y, float z, float rot)
{
    float rx = rot * NEHE_DEG_TO_RAD;
    float ry = rot * 1.2f * NEHE_DEG_TO_RAD;
    float rz = rot * 1.4f * NEHE_DEG_TO_RAD;
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float sz = sinf(rz), cz = cosf(rz);
    float y1 = y * cx - z * sx;
    float z1 = y * sx + z * cx;
    float x2 = x * cy + z1 * sy;
    float z2 = -x * sy + z1 * cy;
    float x3 = x2 * cz - y1 * sz;
    float y3 = x2 * sz + y1 * cz;

    return (N3Vec3){ x3, y3, z2 };
}

static void push_outline_pixel(float x0, float y0, float x1, float y1, float z, float rot, N3Color color)
{
    N3Vertex a = { rotate_lesson_14_point(x0, y0, z, rot), color, 0.0f, 0.0f };
    N3Vertex b = { rotate_lesson_14_point(x1, y0, z, rot), color, 1.0f, 0.0f };
    N3Vertex c = { rotate_lesson_14_point(x1, y1, z, rot), color, 1.0f, 1.0f };
    N3Vertex d = { rotate_lesson_14_point(x0, y1, z, rot), color, 0.0f, 1.0f };

    n3_push_quad(a, b, c, d);
}

static void draw_outline_text_native(const char *text, float rot, N3Color color)
{
    const float cell = 0.080f;
    const float depth = 0.12f;
    float cursor = -nehe_outline_text_width(text, cell) * 0.5f;
    N3Color back = { color.r * 0.20f, color.g * 0.20f, color.b * 0.20f, 1.0f };

    n3_set_depth(true, true);
    n3_set_cull(false);
    for (const char *p = text; *p != '\0'; ++p) {
        const uint8_t *rows = nehe_font_rows(*p);
        for (int row = 0; row < NEHE_FONT_ROWS; ++row) {
            for (int col = 0; col < NEHE_FONT_COLS; ++col) {
                if (nehe_outline_font_pixel(rows, row, col)) {
                    float x0 = cursor + (float)col * cell;
                    float y0 = 0.38f - (float)row * cell;
                    float x1 = x0 + cell * 0.78f;
                    float y1 = y0 - cell * 0.78f;

                    push_outline_pixel(x0, y0, x1, y1, -depth, rot, back);
                    push_outline_pixel(x0, y0, x1, y1, 0.0f, rot, color);
                }
            }
        }
        cursor += cell * 6.0f;
    }
}

static void lesson_14(float t)
{
    float rot = t * 3.0f;
    N3Color color = {
        fmaxf(0.0f, cosf(rot / 20.0f)),
        fmaxf(0.0f, sinf(rot / 25.0f)),
        fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(rot / 17.0f))),
        1.0f
    };

    n3_set_camera(0.0f, 0.0f, -10.0f, 0.0f, 0.0f, 0.0f);
    draw_outline_text_native("OpenGL With NeHe", rot, color);
}

static void push_textured_symbol_pixel(float x0, float y0, float x1, float y1, float z, float rot,
                                       float u0, float v0, float u1, float v1)
{
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
    N3Vertex a = { rotate_lesson_14_point(x0, y0, z, rot), white, u0, v0 };
    N3Vertex b = { rotate_lesson_14_point(x1, y0, z, rot), white, u1, v0 };
    N3Vertex c = { rotate_lesson_14_point(x1, y1, z, rot), white, u1, v1 };
    N3Vertex d = { rotate_lesson_14_point(x0, y1, z, rot), white, u0, v1 };

    n3_push_quad(a, b, c, d);
}

static void draw_textured_skull_native(float ox, float oy, float rot)
{
    const float cell = 0.145f;
    const float depth = 0.10f;
    float left = ox - (float)NEHE_SYMBOL_COLS * cell * 0.5f;
    float top = oy + (float)NEHE_SYMBOL_ROWS * cell * 0.5f;

    n3_set_depth(true, true);
    n3_set_cull(false);
    for (int row = 0; row < NEHE_SYMBOL_ROWS; ++row) {
        for (int col = 0; col < NEHE_SYMBOL_COLS; ++col) {
            if (nehe_skull_outline_pixel(row, col)) {
                float x0 = left + (float)col * cell;
                float y0 = top - (float)row * cell;
                float x1 = x0 + cell * 0.94f;
                float y1 = y0 - cell * 0.94f;
                float u0 = (float)col / (float)NEHE_SYMBOL_COLS;
                float v0 = (float)row / (float)NEHE_SYMBOL_ROWS;
                float u1 = (float)(col + 1) / (float)NEHE_SYMBOL_COLS;
                float v1 = (float)(row + 1) / (float)NEHE_SYMBOL_ROWS;

                push_textured_symbol_pixel(x0, y0, x1, y1, -depth, rot, u0, v0, u1, v1);
                push_textured_symbol_pixel(x0, y0, x1, y1, 0.0f, rot, u0, v0, u1, v1);
            }
        }
    }
}

static void lesson_15(float t)
{
    float rot = t * 55.0f;
    float x = sinf(t * 0.7f) * 0.35f;
    float y = cosf(t * 0.5f) * 0.22f;

    n3_bind_texture(&star_texture);
    n3_set_camera(0.0f, 0.0f, -5.6f, 0.0f, 0.0f, 0.0f);
    draw_textured_skull_native(x, y, rot);
}

static float fog_factor_for_mode(int mode, float distance)
{
    const float density = 0.16f;
    const float start = 3.8f;
    const float end = 9.2f;

    if (mode == 0) {
        return expf(-density * distance);
    }
    if (mode == 1) {
        float f = density * distance;
        return expf(-(f * f));
    }
    return fmaxf(0.0f, fminf(1.0f, (end - distance) / (end - start)));
}

static N3Color fogged_lit_color(N3Vec3 normal, float angle, float distance, int mode)
{
    N3Color fog = { 0.50f, 0.50f, 0.50f, 1.0f };
    N3Vec3 rotated = rotate_axis_angle(normal, angle, 1.0f, 1.0f, 0.0f);
    float diffuse = rotated.z > 0.0f ? rotated.z : 0.0f;
    float lit = 0.22f + diffuse * 0.78f;
    float factor = fog_factor_for_mode(mode, distance);

    return (N3Color){
        fog.r + (lit - fog.r) * factor,
        fog.g + (lit - fog.g) * factor,
        fog.b + (lit - fog.b) * factor,
        1.0f
    };
}

static void draw_fogged_crate_native(float x, float z, float angle, int mode)
{
    static const N3Vec3 normals[6] = {
        { 0.0f, 1.0f, 0.0f }, { 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f,-1.0f }, {-1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
    };
    N3Color colors[6];
    float distance = -(z - 5.8f);

    for (int i = 0; i < 6; ++i) {
        colors[i] = fogged_lit_color(normals[i], angle, distance, mode);
    }
    n3_draw_textured_cube_axis_angle_face_colors(x, 0.0f, z, 0.62f, angle, 1.0f, 1.0f, 0.0f, colors);
}

static void lesson_16(float t)
{
    int mode = ((int)(t / 2.5f)) % 3;
    float angle = t * 42.0f * NEHE_DEG_TO_RAD;

    n3_bind_texture(&crate_texture);
    n3_set_camera(0.0f, 0.0f, -5.8f, 0.0f, 0.0f, 0.0f);
    n3_set_depth(true, true);
    n3_set_cull(true);
    draw_fogged_crate_native(-1.25f, 0.0f, angle, mode);
    draw_fogged_crate_native(0.0f, -1.45f, angle, mode);
    draw_fogged_crate_native(1.25f, -2.9f, angle, mode);
    n3_set_cull(false);
}

static N3Vec3 lesson_17_transform(N3Vec3 p, float angle, float pre_angle)
{
    float z45 = 45.0f * NEHE_DEG_TO_RAD;
    float sx = sinf(z45), cx = cosf(z45);
    N3Vec3 q = p;
    float x;
    float y;

    if (pre_angle != 0.0f) {
        q = rotate_axis_angle(q, pre_angle, 1.0f, 1.0f, 0.0f);
    }
    q = rotate_axis_angle(q, angle, 1.0f, 1.0f, 0.0f);

    x = q.x * cx - q.y * sx;
    y = q.x * sx + q.y * cx;
    return (N3Vec3){ x, y, q.z };
}

static void push_lesson_17_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 1.0f, 1.0f };
    N3Vertex vd = { d, color, 0.0f, 1.0f };

    n3_push_quad(va, vb, vc, vd);
}

static void draw_lesson_17_object(float t)
{
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
    float angle = t * 30.0f * NEHE_DEG_TO_RAD;
    N3Vec3 a = lesson_17_transform((N3Vec3){ -1.0f,  1.0f, 0.0f }, angle, 0.0f);
    N3Vec3 b = lesson_17_transform((N3Vec3){  1.0f,  1.0f, 0.0f }, angle, 0.0f);
    N3Vec3 c = lesson_17_transform((N3Vec3){  1.0f, -1.0f, 0.0f }, angle, 0.0f);
    N3Vec3 d = lesson_17_transform((N3Vec3){ -1.0f, -1.0f, 0.0f }, angle, 0.0f);
    N3Vec3 e = lesson_17_transform((N3Vec3){ -1.0f,  1.0f, 0.0f }, angle, 90.0f * NEHE_DEG_TO_RAD);
    N3Vec3 f = lesson_17_transform((N3Vec3){  1.0f,  1.0f, 0.0f }, angle, 90.0f * NEHE_DEG_TO_RAD);
    N3Vec3 g = lesson_17_transform((N3Vec3){  1.0f, -1.0f, 0.0f }, angle, 90.0f * NEHE_DEG_TO_RAD);
    N3Vec3 h = lesson_17_transform((N3Vec3){ -1.0f, -1.0f, 0.0f }, angle, 90.0f * NEHE_DEG_TO_RAD);

    n3_bind_texture(&cube_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    push_lesson_17_quad(a, b, c, d, white);
    push_lesson_17_quad(e, f, g, h, white);
}

static void push_texture_font_char(float x, float y, float size, unsigned char ch, int set, N3Color color)
{
    float u0, v0, u1, v1;
    N3Vertex a;
    N3Vertex b;
    N3Vertex c;
    N3Vertex d;

    nehe_texture_font_uv(ch, set, &u0, &v0, &u1, &v1);
    a = (N3Vertex){ { x, y, 0.0f }, color, u0, v0 };
    b = (N3Vertex){ { x + size, y, 0.0f }, color, u1, v0 };
    c = (N3Vertex){ { x + size, y + size, 0.0f }, color, u1, v1 };
    d = (N3Vertex){ { x, y + size, 0.0f }, color, u0, v1 };
    n3_push_quad(a, b, c, d);
}

static void draw_texture_font_text_native(float x, float y, const char *text, int set, N3Color color)
{
    const float view_z = 3.0f;
    const float view_h = 2.0f * view_z * tanf(NEHE_FOV_Y_DEGREES * NEHE_DEG_TO_RAD * 0.5f);
    const float view_w = view_h * NEHE_ASPECT;
    const float size = 16.0f * view_h / (float)NEHE_SCREEN_H;
    const float advance = 10.0f * view_w / (float)NEHE_SCREEN_W;
    float cursor = ((x / (float)NEHE_SCREEN_W) - 0.5f) * view_w;
    float baseline = ((y / (float)NEHE_SCREEN_H) - 0.5f) * view_h;

    n3_bind_texture(&font_texture);
    n3_set_depth(false, false);
    n3_set_camera(0.0f, 0.0f, -view_z, 0.0f, 0.0f, 0.0f);
    for (const char *p = text; *p != '\0'; ++p) {
        push_texture_font_char(cursor, baseline, size, (unsigned char)*p, set, color);
        cursor += advance;
    }
}

static void lesson_17(float t)
{
    float cnt1 = t * 0.60f;
    float cnt2 = t * 0.486f;
    N3Color color_a = {
        fmaxf(0.0f, cosf(cnt1)),
        fmaxf(0.0f, sinf(cnt2)),
        fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(cnt1 + cnt2))),
        1.0f
    };
    N3Color color_b = {
        fmaxf(0.0f, sinf(cnt2)),
        fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(cnt1 + cnt2))),
        fmaxf(0.0f, cosf(cnt1)),
        1.0f
    };

    draw_lesson_17_object(t);
    n3_flush();
    draw_texture_font_text_native(280.0f + 250.0f * cosf(cnt1), 235.0f + 200.0f * sinf(cnt2), "NeHe", 0, color_a);
    draw_texture_font_text_native(280.0f + 230.0f * cosf(cnt2), 235.0f + 200.0f * sinf(cnt1), "OpenGL", 1, color_b);
    draw_texture_font_text_native(240.0f + 200.0f * cosf((cnt1 + cnt2) / 5.0f), 2.0f, "Giuseppe D'Agata", 0,
                                  (N3Color){ 0.0f, 0.0f, 1.0f, 1.0f });
    draw_texture_font_text_native(242.0f + 200.0f * cosf((cnt1 + cnt2) / 5.0f), 4.0f, "Giuseppe D'Agata", 0,
                                  (N3Color){ 1.0f, 1.0f, 1.0f, 1.0f });
}

#define NEHE_QUADRIC_PI 3.14159265358979323846f
#define NEHE_QUADRIC_SLICES 32
#define NEHE_QUADRIC_STACKS 24
#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f
#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

static int lesson_18_object(float t)
{
    int object = (int)(t / NEHE_QUADRIC_CYCLE_SECONDS) % 6;
    return object < 0 ? 0 : object;
}

static float lesson_18_local_time(float t)
{
    float local = fmodf(t, NEHE_QUADRIC_CYCLE_SECONDS);
    return local < 0.0f ? local + NEHE_QUADRIC_CYCLE_SECONDS : local;
}

static N3Vec3 rotate_lesson_18_vec(N3Vec3 p, float rx, float ry)
{
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float y = p.y * cx - p.z * sx;
    float z = p.y * sx + p.z * cx;
    float x = p.x * cy + z * sy;
    float z2 = -p.x * sy + z * cy;

    return (N3Vec3){ x, y, z2 };
}

static N3Color lesson_18_color(N3Vec3 normal)
{
    N3Vec3 light = { 0.0f, 0.0f, 1.0f };
    float diffuse = normal.x * light.x + normal.y * light.y + normal.z * light.z;
    float factor;

    if (diffuse < 0.0f) {
        diffuse = 0.0f;
    }
    factor = 0.45f + diffuse * 0.55f;
    return (N3Color){ factor, factor, factor, 1.0f };
}

static N3Vertex lesson_18_vertex(N3Vec3 pos, N3Vec3 normal, float u, float v, float rx, float ry)
{
    N3Vec3 world_normal = rotate_lesson_18_vec(normal, rx, ry);
    pos = (N3Vec3){
        pos.x * NEHE_QUADRIC_NATIVE_SCALE,
        pos.y * NEHE_QUADRIC_NATIVE_SCALE,
        pos.z * NEHE_QUADRIC_NATIVE_SCALE
    };
    return (N3Vertex){ rotate_lesson_18_vec(pos, rx, ry), lesson_18_color(world_normal), u, v };
}

static void lesson_18_push_quad(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Vec3 d,
                                N3Vec3 na, N3Vec3 nb, N3Vec3 nc, N3Vec3 nd,
                                float u0, float v0, float u1, float v1, float rx, float ry)
{
    N3Vertex va = lesson_18_vertex(a, na, u0, v0, rx, ry);
    N3Vertex vb = lesson_18_vertex(b, nb, u1, v0, rx, ry);
    N3Vertex vc = lesson_18_vertex(c, nc, u1, v1, rx, ry);
    N3Vertex vd = lesson_18_vertex(d, nd, u0, v1, rx, ry);

    n3_push_quad(va, vb, vc, vd);
}

static void lesson_18_cube(float rx, float ry)
{
    static const N3Vec3 faces[6][4] = {
        { {  1,  1, -1 }, { -1,  1, -1 }, { -1,  1,  1 }, {  1,  1,  1 } },
        { {  1, -1,  1 }, { -1, -1,  1 }, { -1, -1, -1 }, {  1, -1, -1 } },
        { {  1,  1,  1 }, { -1,  1,  1 }, { -1, -1,  1 }, {  1, -1,  1 } },
        { {  1, -1, -1 }, { -1, -1, -1 }, { -1,  1, -1 }, {  1,  1, -1 } },
        { { -1,  1,  1 }, { -1,  1, -1 }, { -1, -1, -1 }, { -1, -1,  1 } },
        { {  1,  1, -1 }, {  1,  1,  1 }, {  1, -1,  1 }, {  1, -1, -1 } },
    };
    static const N3Vec3 normals[6] = {
        { 0, 1, 0 }, { 0,-1, 0 }, { 0, 0, 1 }, { 0, 0,-1 }, {-1,0,0 }, { 1,0,0 },
    };

    for (int i = 0; i < 6; ++i) {
        lesson_18_push_quad(faces[i][0], faces[i][1], faces[i][2], faces[i][3],
                            normals[i], normals[i], normals[i], normals[i],
                            1.0f, 0.0f, 0.0f, 1.0f, rx, ry);
    }
}

static void lesson_18_cylinder(float base_radius, float top_radius, float height, float rx, float ry)
{
    float slope = (base_radius - top_radius) / height;

    for (int stack = 0; stack < NEHE_QUADRIC_STACKS; ++stack) {
        float t0 = (float)stack / (float)NEHE_QUADRIC_STACKS;
        float t1 = (float)(stack + 1) / (float)NEHE_QUADRIC_STACKS;
        float z0 = t0 * height - height * 0.5f;
        float z1 = t1 * height - height * 0.5f;
        float r0 = base_radius + (top_radius - base_radius) * t0;
        float r1 = base_radius + (top_radius - base_radius) * t1;

        for (int slice = 0; slice < NEHE_QUADRIC_SLICES; ++slice) {
            float u0 = (float)slice / (float)NEHE_QUADRIC_SLICES;
            float u1 = (float)(slice + 1) / (float)NEHE_QUADRIC_SLICES;
            float a0 = u0 * NEHE_QUADRIC_PI * 2.0f;
            float a1 = u1 * NEHE_QUADRIC_PI * 2.0f;
            N3Vec3 n0 = { cosf(a0), sinf(a0), slope };
            N3Vec3 n1 = { cosf(a1), sinf(a1), slope };
            float n0_len = sqrtf(n0.x * n0.x + n0.y * n0.y + n0.z * n0.z);
            float n1_len = sqrtf(n1.x * n1.x + n1.y * n1.y + n1.z * n1.z);
            N3Vec3 p0 = { cosf(a0) * r0, sinf(a0) * r0, z0 };
            N3Vec3 p1 = { cosf(a1) * r0, sinf(a1) * r0, z0 };
            N3Vec3 p2 = { cosf(a1) * r1, sinf(a1) * r1, z1 };
            N3Vec3 p3 = { cosf(a0) * r1, sinf(a0) * r1, z1 };

            n0 = (N3Vec3){ n0.x / n0_len, n0.y / n0_len, n0.z / n0_len };
            n1 = (N3Vec3){ n1.x / n1_len, n1.y / n1_len, n1.z / n1_len };
            lesson_18_push_quad(p0, p1, p2, p3, n0, n1, n1, n0, u0, t0, u1, t1, rx, ry);
        }
    }
}

static void lesson_18_disk(float inner_radius, float outer_radius, float start_deg, float sweep_deg, float rx, float ry)
{
    int slices = NEHE_QUADRIC_SLICES;
    float sweep = sweep_deg * NEHE_DEG_TO_RAD;
    float start = start_deg * NEHE_DEG_TO_RAD;

    if (sweep_deg <= 0.0f) {
        return;
    }
    if (sweep_deg < 359.9f) {
        slices = 24;
    }

    for (int ring = 0; ring < NEHE_QUADRIC_STACKS; ++ring) {
        float r0 = inner_radius + (outer_radius - inner_radius) * (float)ring / (float)NEHE_QUADRIC_STACKS;
        float r1 = inner_radius + (outer_radius - inner_radius) * (float)(ring + 1) / (float)NEHE_QUADRIC_STACKS;

        for (int slice = 0; slice < slices; ++slice) {
            float u0 = (float)slice / (float)slices;
            float u1 = (float)(slice + 1) / (float)slices;
            float a0 = start + sweep * u0;
            float a1 = start + sweep * u1;
            N3Vec3 normal = { 0.0f, 0.0f, 1.0f };
            N3Vec3 p0 = { cosf(a0) * r0, sinf(a0) * r0, 0.0f };
            N3Vec3 p1 = { cosf(a1) * r0, sinf(a1) * r0, 0.0f };
            N3Vec3 p2 = { cosf(a1) * r1, sinf(a1) * r1, 0.0f };
            N3Vec3 p3 = { cosf(a0) * r1, sinf(a0) * r1, 0.0f };

            lesson_18_push_quad(p0, p1, p2, p3, normal, normal, normal, normal,
                                u0, (float)ring / (float)NEHE_QUADRIC_STACKS,
                                u1, (float)(ring + 1) / (float)NEHE_QUADRIC_STACKS, rx, ry);
        }
    }
}

static void lesson_18_sphere(float radius, float rx, float ry)
{
    for (int stack = 0; stack < NEHE_QUADRIC_STACKS; ++stack) {
        float v0 = (float)stack / (float)NEHE_QUADRIC_STACKS;
        float v1 = (float)(stack + 1) / (float)NEHE_QUADRIC_STACKS;
        float phi0 = -NEHE_QUADRIC_PI * 0.5f + v0 * NEHE_QUADRIC_PI;
        float phi1 = -NEHE_QUADRIC_PI * 0.5f + v1 * NEHE_QUADRIC_PI;

        for (int slice = 0; slice < NEHE_QUADRIC_SLICES; ++slice) {
            float u0 = (float)slice / (float)NEHE_QUADRIC_SLICES;
            float u1 = (float)(slice + 1) / (float)NEHE_QUADRIC_SLICES;
            float a0 = u0 * NEHE_QUADRIC_PI * 2.0f;
            float a1 = u1 * NEHE_QUADRIC_PI * 2.0f;
            N3Vec3 n0 = { cosf(phi0) * cosf(a0), cosf(phi0) * sinf(a0), sinf(phi0) };
            N3Vec3 n1 = { cosf(phi0) * cosf(a1), cosf(phi0) * sinf(a1), sinf(phi0) };
            N3Vec3 n2 = { cosf(phi1) * cosf(a1), cosf(phi1) * sinf(a1), sinf(phi1) };
            N3Vec3 n3 = { cosf(phi1) * cosf(a0), cosf(phi1) * sinf(a0), sinf(phi1) };
            N3Vec3 p0 = { n0.x * radius, n0.y * radius, n0.z * radius };
            N3Vec3 p1 = { n1.x * radius, n1.y * radius, n1.z * radius };
            N3Vec3 p2 = { n2.x * radius, n2.y * radius, n2.z * radius };
            N3Vec3 p3 = { n3.x * radius, n3.y * radius, n3.z * radius };

            lesson_18_push_quad(p0, p1, p2, p3, n0, n1, n2, n3, u0, v0, u1, v1, rx, ry);
        }
    }
}

static void lesson_18(float t)
{
    int object = lesson_18_object(t);
    float local = lesson_18_local_time(t);
    float rx = t * 45.0f * NEHE_DEG_TO_RAD;
    float ry = t * 33.0f * NEHE_DEG_TO_RAD;

    n3_bind_texture(&crate_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);

    switch (object) {
    case 0: lesson_18_cube(rx, ry); break;
    case 1: lesson_18_cylinder(1.0f, 1.0f, 3.0f, rx, ry); break;
    case 2: lesson_18_disk(0.5f, 1.5f, 0.0f, 360.0f, rx, ry); break;
    case 3: lesson_18_sphere(1.3f, rx, ry); break;
    case 4: lesson_18_cylinder(1.0f, 0.0f, 3.0f, rx, ry); break;
    default: {
        float phase = local / NEHE_QUADRIC_CYCLE_SECONDS;
        float start = phase < 0.5f ? 0.0f : (phase - 0.5f) * 720.0f;
        float sweep = phase < 0.5f ? phase * 720.0f : 360.0f - start;
        lesson_18_disk(0.5f, 1.5f, start, sweep, rx, ry);
        break;
    }
    }
}

#define NEHE_PARTICLE_COUNT 220
#define NEHE_PARTICLE_LIFE 4.0f

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

static void lesson_20(float t)
{
    float wobble = sinf(t * 1.7f) * 0.28f;
    float roll = t * 70.0f * NEHE_DEG_TO_RAD;
    N3Color white = { 1.0f, 1.0f, 1.0f, 1.0f };

    n3_set_camera(0.0f, 0.0f, -2.8f, 0.0f, 0.0f, 0.0f);
    n3_set_depth(false, false);
    n3_bind_texture(&nehe_texture);
    n3_draw_textured_quad(0.0f, 0.0f, -0.04f, 1.3f, 0.825f, 0.0f, white);

    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_DST_COLOR, NV097_SET_BLEND_FUNC_DFACTOR_V_ZERO);
    n3_bind_texture(&mask_texture);
    n3_draw_textured_quad(wobble, 0.0f, 0.0f, 0.675f, 0.675f, roll, white);

    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    n3_bind_texture(&particle_texture);
    n3_draw_textured_quad(wobble, 0.0f, 0.01f, 0.675f, 0.675f, roll, (N3Color){ 0.35f, 0.9f, 1.0f, 1.0f });
}

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

static void lesson_21(float t)
{
    n3_bind_texture(NULL);
    n3_set_depth(false, false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);
    n3_set_camera(0.0f, 0.0f, -4.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < 24; ++i) {
        float a = t * 0.7f + (float)i * NEHE_QUADRIC_PI / 12.0f;
        float r0 = 0.25f + 0.04f * (float)(i & 3);
        float r1 = 2.1f + 0.18f * sinf(t * 1.3f + (float)i);
        N3Color color = {
            0.35f + 0.65f * fabsf(sinf(a)),
            0.35f + 0.65f * fabsf(sinf(a + 2.1f)),
            0.35f + 0.65f * fabsf(sinf(a + 4.2f)),
            0.42f
        };
        lesson_21_line(cosf(a) * r0, sinf(a) * r0, cosf(a) * r1, sinf(a) * r1, 0.012f, color);
    }
    for (int i = 0; i < 18; ++i) {
        float x0 = -2.1f + (float)i * 0.25f;
        float x1 = x0 + 0.18f;
        float y = -1.4f + sinf(t * 3.2f + (float)i * 0.55f) * 0.23f;
        lesson_21_line(x0, y, x1, -y, 0.018f, (N3Color){ 0.9f, 0.9f, 1.0f, 0.72f });
    }
}

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

static void lesson_23(float t)
{
    n3_bind_texture(&sphere_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -4.6f, 0.0f, 0.0f, 0.0f);
    lesson_18_sphere(1.35f, t * 34.0f * NEHE_DEG_TO_RAD, t * 48.0f * NEHE_DEG_TO_RAD);
}

static void lesson_24(float t)
{
    N3Color colors[4] = {
        { 1.0f, 0.35f, 0.25f, 1.0f },
        { 0.25f, 1.0f, 0.45f, 1.0f },
        { 0.25f, 0.55f, 1.0f, 1.0f },
        { 1.0f, 0.9f, 0.25f, 1.0f },
    };
    int w = n3_back_buffer_width();
    int h = n3_back_buffer_height();

    n3_bind_texture(&cube_texture);
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_set_camera(0.0f, 0.0f, -4.2f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i) {
        int sx = (i & 1) ? w / 2 : 0;
        int sy = (i & 2) ? h / 2 : 0;
        n3_set_scissor(true, sx, sy, w / 2, h / 2);
        n3_draw_textured_cube_axis_angle_tint((i & 1) ? 0.85f : -0.85f,
                                              (i & 2) ? -0.62f : 0.62f,
                                              0.0f, 0.52f,
                                              t * (35.0f + (float)i * 11.0f) * NEHE_DEG_TO_RAD,
                                              0.35f + (float)i * 0.15f, 1.0f, 0.25f,
                                              colors[i]);
    }
    n3_set_scissor(false, 0, 0, w, h);
    n3_set_cull(false);
}

static N3Vec3 lesson_25_target(int shape, int index, int count)
{
    float u = (float)index / (float)count;
    float a = u * NEHE_QUADRIC_PI * 2.0f;
    float b = (float)((index * 37) % count) / (float)count * NEHE_QUADRIC_PI * 2.0f;

    switch (shape & 3) {
    case 0:
        return (N3Vec3){ cosf(a) * sinf(b) * 1.4f, sinf(a) * sinf(b) * 1.4f, cosf(b) * 1.4f };
    case 1: {
        float r = 1.0f + 0.35f * cosf(b * 3.0f);
        return (N3Vec3){ cosf(a) * r, sinf(b * 2.0f) * 0.55f, sinf(a) * r };
    }
    case 2:
        return (N3Vec3){ cosf(a) * 0.75f, sinf(a * 2.0f) * 1.35f, sinf(a) * 0.75f };
    default:
        return (N3Vec3){
            lesson_19_unit(index, 21u) * 3.4f - 1.7f,
            lesson_19_unit(index, 22u) * 3.0f - 1.5f,
            lesson_19_unit(index, 23u) * 3.4f - 1.7f
        };
    }
}

static void lesson_25(float t)
{
    const int count = 260;
    float cycle = t * 0.45f;
    int from = (int)floorf(cycle);
    float mix = cycle - floorf(cycle);
    float smooth = mix * mix * (3.0f - 2.0f * mix);

    n3_bind_texture(NULL);
    n3_set_depth(true, true);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    n3_set_camera(0.0f, 0.0f, -7.5f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < count; ++i) {
        N3Vec3 a = lesson_25_target(from, i, count);
        N3Vec3 b = lesson_25_target(from + 1, i, count);
        N3Vec3 p = {
            a.x + (b.x - a.x) * smooth,
            a.y + (b.y - a.y) * smooth,
            a.z + (b.z - a.z) * smooth
        };
        N3Color c = {
            0.35f + 0.65f * lesson_19_unit(i, 30u),
            0.35f + 0.65f * lesson_19_unit(i, 31u),
            0.65f + 0.35f * lesson_19_unit(i, 32u),
            0.78f
        };
        p = rotate_axis_angle(p, t * 0.45f, 0.2f, 1.0f, 0.1f);
        n3_draw_billboard(p.x, p.y, p.z, 0.025f, 0.025f, c);
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

static void lesson_27(float t)
{
    float angle = t * 44.0f * NEHE_DEG_TO_RAD;

    n3_set_camera(0.0f, 0.2f, -6.0f, -0.18f, 0.0f, 0.0f);
    n3_set_cull(false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);

    n3_bind_texture(NULL);
    n3_set_depth(false, false);
    push_plane_quad((N3Vec3){ -2.6f, -1.15f, -2.0f }, (N3Vec3){ 2.6f, -1.15f, -2.0f },
                    (N3Vec3){ 2.6f, -1.15f, 2.0f }, (N3Vec3){ -2.6f, -1.15f, 2.0f },
                    (N3Color){ 0.42f, 0.42f, 0.46f, 1.0f });
    push_plane_quad((N3Vec3){ -2.6f, -1.15f, 2.0f }, (N3Vec3){ 2.6f, -1.15f, 2.0f },
                    (N3Vec3){ 2.6f, 1.95f, 2.0f }, (N3Vec3){ -2.6f, 1.95f, 2.0f },
                    (N3Color){ 0.24f, 0.28f, 0.36f, 1.0f });
    push_plane_quad((N3Vec3){ -1.1f, -1.13f, -0.65f }, (N3Vec3){ 1.65f, -1.13f, -0.25f },
                    (N3Vec3){ 1.05f, -1.13f, 0.35f }, (N3Vec3){ -1.65f, -1.13f, -0.05f },
                    (N3Color){ 0.0f, 0.0f, 0.0f, 0.45f });
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_draw_pyramid(-0.72f, 0.05f, 0.0f, 0.55f, 0.70f, 0.55f, 0.0f, angle, 0.0f);
    n3_draw_cube_axis_angle(0.82f, -0.02f, 0.0f, 0.85f, 0.85f, 0.85f, angle, 0.3f, 1.0f, 0.2f, face);
    n3_set_cull(false);
}

static float bezier1(float p0, float p1, float p2, float p3, float t)
{
    float it = 1.0f - t;
    return p0 * it * it * it + 3.0f * p1 * t * it * it + 3.0f * p2 * t * t * it + p3 * t * t * t;
}

static N3Vec3 lesson_28_patch_point(float u, float v, float phase)
{
    float x = bezier1(-1.8f, -0.8f, 0.8f, 1.8f, u);
    float z = bezier1(-1.5f, -0.55f, 0.55f, 1.5f, v);
    float arch_u = sinf(u * NEHE_QUADRIC_PI);
    float arch_v = sinf(v * NEHE_QUADRIC_PI);
    return (N3Vec3){ x, (arch_u * arch_v - 0.45f) * 1.1f + sinf(phase + x * 1.6f + z) * 0.18f, z };
}

static void lesson_28(float t)
{
    const int divs = 14;
    float phase = t * 1.4f;

    n3_bind_texture(&tim_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.15f, -5.6f, -0.22f, 0.0f, 0.0f);
    for (int y = 0; y < divs; ++y) {
        for (int x = 0; x < divs; ++x) {
            float u0 = (float)x / (float)divs;
            float u1 = (float)(x + 1) / (float)divs;
            float v0 = (float)y / (float)divs;
            float v1 = (float)(y + 1) / (float)divs;
            N3Color color = { 0.55f + 0.45f * v0, 0.65f + 0.25f * u0, 1.0f, 1.0f };
            N3Vertex a = { lesson_28_patch_point(u0, v0, phase), color, u0, v0 };
            N3Vertex b = { lesson_28_patch_point(u1, v0, phase), color, u1, v0 };
            N3Vertex c = { lesson_28_patch_point(u1, v1, phase), color, u1, v1 };
            N3Vertex d = { lesson_28_patch_point(u0, v1, phase), color, u0, v1 };
            n3_push_quad(a, b, c, d);
        }
    }
}

static void lesson_29(float t)
{
    (void)t;
    n3_bind_texture(&raw_texture);
    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);
    n3_set_camera(0.0f, 0.0f, -3.2f, 0.0f, 0.0f, 0.0f);
    n3_draw_textured_quad(-1.25f, 0.0f, 0.0f, 0.52f, 0.52f, 0.0f, (N3Color){ 1.0f, 1.0f, 1.0f, 1.0f });
    n3_draw_textured_quad(0.0f, 0.0f, 0.0f, 0.72f, 0.42f, 0.0f, (N3Color){ 0.75f, 1.0f, 0.85f, 0.92f });
    n3_draw_textured_quad(1.25f, 0.0f, 0.0f, 0.42f, 0.72f, 0.0f, (N3Color){ 1.0f, 0.75f, 0.95f, 0.92f });
}

static float lesson_30_distance(float ax, float ay, float az, float bx, float by, float bz)
{
    float dx = ax - bx;
    float dy = ay - by;
    float dz = az - bz;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

static void lesson_30(float t)
{
    float sx = sinf(t * 0.85f) * 1.25f;
    float sy = cosf(t * 1.15f) * 0.52f;
    float sz = sinf(t * 0.53f) * 0.75f;
    float dist = lesson_30_distance(sx, sy, sz, 0.0f, 0.0f, 0.0f);
    bool hit = dist < 0.98f;
    N3Color marker = hit ? (N3Color){ 1.0f, 0.10f, 0.06f, 0.72f } : (N3Color){ 0.10f, 0.70f, 1.0f, 0.55f };

    n3_set_camera(0.0f, 0.0f, -6.2f, -0.12f, 0.0f, 0.0f);
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_bind_texture(&crate_texture);
    n3_draw_textured_cube_axis_angle_tint(0.0f, 0.0f, 0.0f, 0.92f, t * 0.65f, 0.3f, 1.0f, 0.2f,
                                          hit ? (N3Color){ 1.0f, 0.52f, 0.50f, 1.0f } : (N3Color){ 0.88f, 0.94f, 1.0f, 1.0f });
    n3_set_cull(false);
    n3_bind_texture(&sphere_texture);
    lesson_18_sphere(0.34f, t * 1.2f, t * 0.8f);
    n3_bind_texture(NULL);
    n3_draw_billboard(sx, sy, sz, 0.55f, 0.55f, marker);
    for (int i = 0; i < 12; ++i) {
        float a = (float)i / 12.0f * NEHE_QUADRIC_PI * 2.0f;
        n3_draw_billboard(cosf(a) * 0.98f, sinf(a) * 0.98f, 0.02f, 0.035f, 0.035f,
                          hit ? (N3Color){ 1.0f, 0.18f, 0.12f, 0.85f } : (N3Color){ 0.18f, 0.55f, 1.0f, 0.65f });
    }
}

static void push_colored_triangle(N3Vec3 a, N3Vec3 b, N3Vec3 c, N3Color color)
{
    N3Vertex va = { a, color, 0.0f, 0.0f };
    N3Vertex vb = { b, color, 1.0f, 0.0f };
    N3Vertex vc = { c, color, 0.5f, 1.0f };
    n3_push_triangle(va, vb, vc);
}

static void lesson_31(float t)
{
    float yaw = t * 0.72f;
    N3Color hull = { 0.58f, 0.72f, 0.88f, 1.0f };
    N3Color wing = { 0.28f, 0.42f, 0.68f, 1.0f };
    N3Color flame = { 1.0f, 0.42f + 0.25f * sinf(t * 8.0f), 0.06f, 1.0f };

    n3_bind_texture(NULL);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -5.4f, -0.08f, yaw, 0.0f);
    push_colored_triangle((N3Vec3){ 0.0f, 0.58f, -1.25f }, (N3Vec3){ -0.45f, -0.34f, 0.55f },
                          (N3Vec3){ 0.45f, -0.34f, 0.55f }, hull);
    push_colored_triangle((N3Vec3){ 0.0f, -0.18f, -1.05f }, (N3Vec3){ -1.55f, -0.44f, 0.35f },
                          (N3Vec3){ -0.45f, -0.34f, 0.55f }, wing);
    push_colored_triangle((N3Vec3){ 0.0f, -0.18f, -1.05f }, (N3Vec3){ 0.45f, -0.34f, 0.55f },
                          (N3Vec3){ 1.55f, -0.44f, 0.35f }, wing);
    push_colored_triangle((N3Vec3){ -0.45f, -0.34f, 0.55f }, (N3Vec3){ 0.0f, -0.04f, 1.05f },
                          (N3Vec3){ 0.45f, -0.34f, 0.55f }, (N3Color){ 0.42f, 0.55f, 0.78f, 1.0f });
    push_colored_triangle((N3Vec3){ -0.22f, -0.26f, 0.88f }, (N3Vec3){ 0.22f, -0.26f, 0.88f },
                          (N3Vec3){ 0.0f, -0.26f, 1.55f + 0.18f * sinf(t * 9.0f) }, flame);
}

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

static void lesson_33(float t)
{
    (void)t;
    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -3.6f, 0.0f, 0.0f, 0.0f);
    n3_bind_texture(&nehe_texture);
    n3_draw_textured_quad(-0.9f, 0.68f, 0.0f, 0.52f, 0.38f, 0.0f, (N3Color){ 1.0f, 1.0f, 1.0f, 1.0f });
    n3_bind_texture(&cube_texture);
    n3_draw_textured_quad(0.9f, 0.68f, 0.0f, 0.52f, 0.38f, 0.0f, (N3Color){ 1.0f, 0.82f, 0.82f, 1.0f });
    n3_bind_texture(&tim_texture);
    n3_draw_textured_quad(-0.9f, -0.52f, 0.0f, 0.52f, 0.38f, 0.0f, (N3Color){ 0.82f, 1.0f, 0.90f, 1.0f });
    n3_bind_texture(&raw_texture);
    n3_draw_textured_quad(0.9f, -0.52f, 0.0f, 0.52f, 0.38f, 0.0f, (N3Color){ 0.82f, 0.90f, 1.0f, 1.0f });
}

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

static void lesson_36(float t)
{
    N3Color blur_color = { 0.2f, 0.6f, 1.0f, 0.12f };

    n3_set_camera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f);
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_bind_texture(&cube_texture);
    n3_draw_textured_cube_axis_angle_tint(0.0f, 0.0f, 0.0f, 0.92f, t * 0.8f, 0.2f, 1.0f, 0.4f,
                                          (N3Color){ 1.0f, 1.0f, 1.0f, 1.0f });

    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    n3_bind_texture(&particle_texture);
    for (int i = 0; i < 12; ++i) {
        float s = 0.62f + (float)i * 0.13f;
        float a = t * 0.45f + (float)i * 0.18f;
        blur_color.a = 0.13f * (1.0f - (float)i / 13.0f);
        n3_draw_textured_quad(cosf(a) * 0.05f, sinf(a) * 0.04f, 0.05f + (float)i * 0.002f, s, s, a, blur_color);
    }
}

static void lesson_37(float t)
{
    const N3Color cel_faces[6] = {
        { 0.10f, 0.80f, 0.18f, 1.0f },
        { 0.06f, 0.38f, 0.10f, 1.0f },
        { 0.88f, 0.15f, 0.10f, 1.0f },
        { 0.54f, 0.08f, 0.06f, 1.0f },
        { 0.12f, 0.25f, 0.90f, 1.0f },
        { 0.05f, 0.09f, 0.42f, 1.0f }
    };

    n3_bind_texture(NULL);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -5.2f, -0.10f, 0.0f, 0.0f);
    n3_draw_cube_axis_angle(0.0f, 0.0f, 0.0f, 1.12f, 1.12f, 1.12f, t * 0.75f, 0.3f, 1.0f, 0.2f,
                            (N3Color[6]){{0.01f,0.01f,0.01f,1.0f},{0.01f,0.01f,0.01f,1.0f},{0.01f,0.01f,0.01f,1.0f},
                                         {0.01f,0.01f,0.01f,1.0f},{0.01f,0.01f,0.01f,1.0f},{0.01f,0.01f,0.01f,1.0f}});
    n3_draw_cube_axis_angle(0.0f, 0.0f, 0.0f, 0.98f, 0.98f, 0.98f, t * 0.75f, 0.3f, 1.0f, 0.2f, cel_faces);
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

static float lesson_39_height(int x, int y, float t)
{
    return sinf((float)x * 0.8f + t * 1.6f) * 0.18f + cosf((float)y * 0.7f - t * 1.1f) * 0.14f;
}

static void lesson_39(float t)
{
    const int cols = 9;
    const int rows = 7;
    const float step = 0.34f;

    n3_bind_texture(NULL);
    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -4.4f, -0.18f, 0.0f, 0.0f);
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            float x0 = ((float)x - (float)(cols - 1) * 0.5f) * step;
            float y0 = ((float)y - (float)(rows - 1) * 0.5f) * step;
            float x1 = x0 + step;
            float y1 = y0 + step;
            N3Color color = { 0.18f + (float)x * 0.035f, 0.38f + (float)y * 0.045f, 0.90f, 0.72f };
            push_plane_quad((N3Vec3){ x0, y0 + lesson_39_height(x, y, t), 0.0f },
                            (N3Vec3){ x1, y0 + lesson_39_height(x + 1, y, t), 0.0f },
                            (N3Vec3){ x1, y1 + lesson_39_height(x + 1, y + 1, t), 0.0f },
                            (N3Vec3){ x0, y1 + lesson_39_height(x, y + 1, t), 0.0f }, color);
        }
    }
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            n3_draw_billboard(((float)x - (float)(cols - 1) * 0.5f) * step,
                              ((float)y - (float)(rows - 1) * 0.5f) * step + lesson_39_height(x, y, t),
                              0.04f, 0.035f, 0.035f, (N3Color){ 1.0f, 1.0f, 1.0f, 0.9f });
        }
    }
}

static void lesson_40_segment(N3Vec3 a, N3Vec3 b, float width, N3Color color)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    float nx;
    float ny;

    if (len < 0.0001f) {
        return;
    }
    nx = -dy / len * width;
    ny = dx / len * width;
    push_plane_quad((N3Vec3){ a.x + nx, a.y + ny, a.z }, (N3Vec3){ b.x + nx, b.y + ny, b.z },
                    (N3Vec3){ b.x - nx, b.y - ny, b.z }, (N3Vec3){ a.x - nx, a.y - ny, a.z }, color);
}

static void lesson_40(float t)
{
    const int points = 18;
    N3Vec3 prev = { -1.55f, 0.72f, 0.0f };

    n3_bind_texture(NULL);
    n3_set_depth(false, false);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -4.2f, 0.0f, 0.0f, 0.0f);
    n3_draw_billboard(prev.x, prev.y, prev.z, 0.08f, 0.08f, (N3Color){ 1.0f, 0.95f, 0.45f, 1.0f });
    for (int i = 1; i < points; ++i) {
        float u = (float)i / (float)(points - 1);
        N3Vec3 p = {
            -1.55f + u * 3.1f,
            0.72f - sinf(u * NEHE_QUADRIC_PI) * (0.95f + 0.10f * sinf(t * 1.4f)) + sinf(t * 2.0f + u * 8.0f) * 0.08f,
            0.0f
        };
        lesson_40_segment(prev, p, 0.025f, (N3Color){ 0.90f, 0.72f, 0.36f, 1.0f });
        n3_draw_billboard(p.x, p.y, p.z + 0.02f, 0.045f, 0.045f, (N3Color){ 1.0f, 0.86f, 0.45f, 1.0f });
        prev = p;
    }
}

static void lesson_41(float t)
{
    n3_set_depth(true, false);
    n3_bind_texture(&particle_texture);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE);
    for (int i = 0; i < 22; ++i) {
        float z = -1.55f + (float)i * 0.14f;
        float sway = sinf(t * 0.9f + (float)i * 0.37f) * 0.18f;
        float size = 0.55f + (float)i * 0.035f;
        float alpha = 0.10f + 0.10f * (1.0f - (float)i / 22.0f);
        n3_draw_textured_quad(sway, 0.05f + sinf(t * 1.3f + (float)i) * 0.08f, z, size, size * 0.72f,
                              t * 0.18f + (float)i * 0.11f, (N3Color){ 0.36f, 0.62f, 1.0f, alpha });
    }
    n3_bind_texture(&crate_texture);
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_set_blend_func(NV097_SET_BLEND_FUNC_SFACTOR_V_SRC_ALPHA, NV097_SET_BLEND_FUNC_DFACTOR_V_ONE_MINUS_SRC_ALPHA);
    n3_draw_textured_cube_axis_angle_tint(0.0f, -0.12f, -0.35f, 0.74f, t * 0.65f, 0.4f, 1.0f, 0.2f,
                                          (N3Color){ 0.85f, 0.94f, 1.0f, 1.0f });
    n3_set_cull(false);
}

static void lesson_42(float t)
{
    const int w = n3_back_buffer_width();
    const int h = n3_back_buffer_height();
    const int half_w = w / 2;
    const int half_h = h / 2;
    const int rects[4][4] = {
        { 0, 0, half_w, half_h },
        { half_w, 0, half_w, half_h },
        { 0, half_h, half_w, half_h },
        { half_w, half_h, half_w, half_h },
    };

    n3_set_depth(true, true);
    n3_bind_texture(&crate_texture);
    n3_set_cull(true);
    for (int i = 0; i < 4; ++i) {
        float x = 0.0f;
        float y = 0.0f;
        n3_set_viewport(rects[i][0], rects[i][1], rects[i][2], rects[i][3]);
        n3_set_scissor(true, rects[i][0], rects[i][1], rects[i][2], rects[i][3]);
        n3_draw_textured_cube_axis_angle_tint(x, y, 0.0f, 0.48f, t * (0.5f + (float)i * 0.16f),
                                              0.25f + (float)i * 0.1f, 1.0f, 0.35f,
                                              (N3Color){ 0.78f + 0.05f * (float)i, 0.88f, 1.0f - 0.08f * (float)i, 1.0f });
        n3_draw_triangle(x, y - 0.78f, 0.02f, 0.25f, 0.20f, t * 0.5f + (float)i,
                         (N3Color){ 1.0f, 0.12f, 0.10f, 1.0f },
                         (N3Color){ 0.12f, 1.0f, 0.28f, 1.0f },
                         (N3Color){ 0.16f, 0.38f, 1.0f, 1.0f });
    }
    n3_set_scissor(false, 0, 0, w, h);
    n3_set_viewport(0, 0, w, h);
    n3_set_cull(false);
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

static void lesson_45(float t)
{
    const int cols = 18;
    const int rows = 12;
    n3_set_depth(true, true);
    n3_bind_texture(NULL);
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            float x0 = ((float)x - 8.5f) * 0.16f;
            float x1 = ((float)x + 1.0f - 8.5f) * 0.16f;
            float y0 = ((float)y - 5.5f) * 0.16f;
            float y1 = ((float)y + 1.0f - 5.5f) * 0.16f;
            float h0 = sinf(t * 2.0f + (float)x * 0.42f + (float)y * 0.33f) * 0.12f;
            float h1 = sinf(t * 2.0f + (float)(x + 1) * 0.42f + (float)y * 0.33f) * 0.12f;
            float h2 = sinf(t * 2.0f + (float)(x + 1) * 0.42f + (float)(y + 1) * 0.33f) * 0.12f;
            float h3 = sinf(t * 2.0f + (float)x * 0.42f + (float)(y + 1) * 0.33f) * 0.12f;
            N3Color color = { 0.18f + (float)x * 0.025f, 0.36f + (float)y * 0.035f, 0.92f, 1.0f };
            push_plane_quad((N3Vec3){ x0, y0, h0 }, (N3Vec3){ x1, y0, h1 },
                            (N3Vec3){ x1, y1, h2 }, (N3Vec3){ x0, y1, h3 }, color);
        }
    }
}

static void lesson_46(float t)
{
    n3_set_depth(false, false);
    n3_bind_texture(NULL);
    for (int i = 0; i < 16; ++i) {
        float y = -1.15f + (float)i * 0.15f;
        float alpha = 0.18f + (float)i * 0.035f;
        lesson_21_line(-1.55f, y, 1.55f, y + 0.12f, 0.005f + (float)i * 0.0015f,
                       (N3Color){ 0.22f + (float)i * 0.035f, 0.72f, 1.0f, alpha });
    }
    n3_set_depth(true, true);
    n3_set_cull(true);
    n3_draw_cube_axis_angle(0.0f, 0.0f, -0.02f, 0.82f, 0.82f, 0.82f, t * 0.55f, 0.25f, 1.0f, 0.15f, face);
    n3_set_cull(false);
}

static void lesson_47(float t)
{
    n3_set_depth(true, true);
    n3_bind_texture(&raw_texture);
    for (int i = 0; i < 10; ++i) {
        float x = -1.35f + (float)i * 0.30f;
        float wave = sinf(t * 1.6f + (float)i * 0.55f) * 0.24f;
        float twist = t * 0.45f + (float)i * 0.22f;
        n3_draw_textured_quad(x, wave, -0.02f * (float)i, 0.13f, 0.55f, twist,
                              (N3Color){ 0.45f + 0.04f * (float)i, 0.75f, 1.0f, 0.92f });
    }
}

static void lesson_48(float t)
{
    float ax = sinf(t * 0.37f) * 0.6f + 0.35f;
    float ay = cosf(t * 0.29f) * 0.6f + 0.65f;
    float az = sinf(t * 0.23f) * 0.45f + 0.25f;
    n3_set_depth(true, true);
    n3_bind_texture(&crate_texture);
    n3_set_cull(true);
    n3_draw_textured_cube_axis_angle_tint(0.0f, 0.0f, 0.0f, 0.92f, t * 0.75f, ax, ay, az,
                                          (N3Color){ 0.95f, 0.98f, 1.0f, 1.0f });
    n3_set_cull(false);
    n3_bind_texture(NULL);
    for (int i = 0; i < 24; ++i) {
        float a0 = (float)i * NEHE_QUADRIC_PI / 12.0f;
        float a1 = (float)(i + 1) * NEHE_QUADRIC_PI / 12.0f;
        lesson_21_line(cosf(a0) * 1.35f, sinf(a0) * 1.35f,
                       cosf(a1) * 1.35f, sinf(a1) * 1.35f, 0.008f,
                       (N3Color){ 0.72f, 0.82f, 1.0f, 0.42f });
    }
}

void nehe_lesson_render(int lesson, float t)
{
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);

    switch (clamp_lesson(lesson)) {
    case 0: lesson_01(t); break;
    case 1: lesson_02(t); break;
    case 2: lesson_03(t); break;
    case 3: lesson_04(t); break;
    case 4: lesson_05(t); break;
    case 5: lesson_06(t); break;
    case 6: lesson_07(t); break;
    case 7: lesson_08(t); break;
    case 8: lesson_09(t); break;
    case 9: lesson_10(t); break;
    case 10: lesson_11(t); break;
    case 11: lesson_12(t); break;
    case 12: lesson_13(t); break;
    case 13: lesson_14(t); break;
    case 14: lesson_15(t); break;
    case 15: lesson_16(t); break;
    case 16: lesson_17(t); break;
    case 17: lesson_18(t); break;
    case 18: lesson_19(t); break;
    case 19: lesson_20(t); break;
    case 20: lesson_21(t); break;
    case 21: lesson_22(t); break;
    case 22: lesson_23(t); break;
    case 23: lesson_24(t); break;
    case 24: lesson_25(t); break;
    case 25: lesson_26(t); break;
    case 26: lesson_27(t); break;
    case 27: lesson_28(t); break;
    case 28: lesson_29(t); break;
    case 29: lesson_30(t); break;
    case 30: lesson_31(t); break;
    case 31: lesson_32(t); break;
    case 32: lesson_33(t); break;
    case 33: lesson_34(t); break;
    case 34: lesson_35(t); break;
    case 35: lesson_36(t); break;
    case 36: lesson_37(t); break;
    case 37: lesson_38(t); break;
    case 38: lesson_39(t); break;
    case 39: lesson_40(t); break;
    case 40: lesson_41(t); break;
    case 41: lesson_42(t); break;
    case 42: lesson_43(t); break;
    case 43: lesson_44(t); break;
    case 44: lesson_45(t); break;
    case 45: lesson_46(t); break;
    case 46: lesson_47(t); break;
    case 47: lesson_48(t); break;
    }
}

const char *nehe_lesson_title(int lesson)
{
    return titles[clamp_lesson(lesson)];
}

const char *nehe_lesson_detail(int lesson)
{
    return details[clamp_lesson(lesson)];
}

bool nehe_lesson_blend_enabled(int lesson)
{
    lesson = clamp_lesson(lesson);
    return lesson == 7 || lesson == 8 || lesson == 16 || lesson == 18 || lesson == 19 || lesson == 20 ||
           lesson == 24 || lesson == 25 || lesson == 26 || lesson == 28 || lesson == 31 ||
           lesson == 35 || lesson == 37 || lesson == 38 || lesson == 40 || lesson == 43 ||
           lesson == 45 || lesson == 46 || lesson == 47;
}

uint32_t nehe_lesson_clear_color(int lesson)
{
    lesson = clamp_lesson(lesson);
    if (lesson == 0) {
        return 0x00020A16;
    }
    if (lesson == 15) {
        return 0x007F7F7F;
    }
    return 0x00070B14;
}
