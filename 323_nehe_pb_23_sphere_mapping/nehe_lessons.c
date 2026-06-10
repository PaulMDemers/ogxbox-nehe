#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>

#define LOCAL_NEHE_LESSON_INDEX 22

#define NEHE_QUADRIC_PI 3.14159265358979323846f

#define NEHE_QUADRIC_SLICES 32

#define NEHE_QUADRIC_STACKS 24

#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f

#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

static N3Texture sphere_texture;
static uint8_t sphere_texture_pixels[64 * 64 * 4];

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

static void lesson_23(float t)
{
    n3_bind_texture(&sphere_texture);
    n3_set_depth(true, true);
    n3_set_cull(false);
    n3_set_camera(0.0f, 0.0f, -4.6f, 0.0f, 0.0f, 0.0f);
    lesson_18_sphere(1.35f, t * 34.0f * NEHE_DEG_TO_RAD, t * 48.0f * NEHE_DEG_TO_RAD);
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

            sphere_texture_pixels[i + 0] = (uint8_t)(20 + sphere / 5);
            sphere_texture_pixels[i + 1] = (uint8_t)(80 + sphere / 2);
            sphere_texture_pixels[i + 2] = (uint8_t)(140 + sphere / 3);
            sphere_texture_pixels[i + 3] = 255;

        }
    }

    if (n3_texture_create_rgba(&sphere_texture, 64, 64, sphere_texture_pixels) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&sphere_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_23(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 23 - sphere mapping";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Environment mapped sphere";
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
