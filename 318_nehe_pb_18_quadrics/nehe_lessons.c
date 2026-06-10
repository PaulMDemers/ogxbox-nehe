#include "nehe_lessons.h"
#include "nehe_assets_rgba.h"
#include "nehe_native.h"
#include "nehe_scene.h"

#include <math.h>
#include <pbkit/pbkit.h>

#define LOCAL_NEHE_LESSON_INDEX 17

#define NEHE_QUADRIC_PI 3.14159265358979323846f

#define NEHE_QUADRIC_SLICES 32

#define NEHE_QUADRIC_STACKS 24

#define NEHE_QUADRIC_CYCLE_SECONDS 2.0f

#define NEHE_QUADRIC_NATIVE_SCALE 0.65f

static N3Texture crate_texture;

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

int nehe_lessons_init(void)
{
    if (n3_texture_create_rgba(&crate_texture, NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba) != 0) {
        nehe_lessons_shutdown();
        return -1;
    }
    return 0;
}

void nehe_lessons_shutdown(void)
{
    n3_texture_destroy(&crate_texture);
}

void nehe_lesson_render(int lesson, float t)
{
    (void)lesson;
    n3_set_projection(NEHE_FOV_Y_DEGREES, NEHE_NEAR_Z, NEHE_FAR_Z);
    n3_set_camera(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z, 0.0f, 0.0f, 0.0f);
    lesson_18(t);
}

const char *nehe_lesson_title(int lesson)
{
    (void)lesson;
    return "NeHe 18 - quadrics";
}

const char *nehe_lesson_detail(int lesson)
{
    (void)lesson;
    return "Cube and GLU-style quadric shapes";
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
