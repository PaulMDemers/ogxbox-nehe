#include "nxgl.h"
#include "../common_nehe/nehe_assets_rgba.h"
#include "../common_nehe/nehe_bitmap_font.h"
#include "../common_nehe/nehe_capture_time.h"
#include "../common_nehe/nehe_outline_font.h"
#include "../common_nehe/nehe_scene.h"
#include "../common_nehe/nehe_starfield.h"
#include "../common_nehe/nehe_texture_font.h"
#include "../common_nehe/nehe_world_data.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#ifndef NEHE_STANDALONE_LESSON_INDEX
#include <usbh_lib.h>
#include <xid_driver.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef NEHE_STANDALONE_LESSON_INDEX
#define PAD_PACKET_SIZE 32
#define BUTTON_DEADZONE 0x20
#define TRIGGER_DEADZONE 0x80
#define STICK_DEADZONE 18000
#define XID_DPAD_LEFT (1u << 2)
#define XID_DPAD_RIGHT (1u << 3)
#endif

typedef struct Lesson Lesson;
typedef void (*LessonFn)(Lesson *lesson);
typedef void (*RenderFn)(Lesson *lesson, float t);

struct Lesson {
    const char *title;
    const char *detail;
    LessonFn init;
    RenderFn render;
    LessonFn shutdown;
    GLuint textures[4];
    GLuint lists[4];
};

#ifndef NEHE_STANDALONE_LESSON_INDEX
static xid_dev_t *active_pad;
static uint8_t pad_packet[PAD_PACKET_SIZE];
static bool pad_packet_valid;
static bool pad_read_queued;
#endif

static uint8_t checker_pixels[64 * 64 * 4];
static uint8_t crate_pixels[64 * 64 * 4];
static uint8_t star_pixels[32 * 32 * 4];
static uint8_t particle_pixels[32 * 32 * 4];
static uint8_t mask_pixels[32 * 32 * 4];
static uint8_t bump_pixels[64 * 64 * 4];
static uint8_t sphere_pixels[64 * 64 * 4];
static uint8_t raw_pixels[64 * 64 * 4];
static uint8_t video_pixels[64 * 64 * 4];
static uint8_t world_pixels[64 * 64 * 4];
static uint8_t font_pixels[NEHE_TEXTURE_FONT_SIZE * NEHE_TEXTURE_FONT_SIZE * 4];

#ifndef NEHE_STANDALONE_LESSON_INDEX
static int current_lesson = 1;
#endif
static DWORD lesson_start;

#ifndef NEHE_STANDALONE_LESSON_INDEX
static int16_t read_s16_le(const uint8_t *data)
{
    return (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static void pad_read_callback(UTR_T *utr)
{
    xid_dev_t *pad = (xid_dev_t *)utr->context;

    if (pad == active_pad && utr->status >= 0 && utr->buff != NULL && utr->xfer_len >= 20) {
        uint32_t len = utr->xfer_len;
        if (len > PAD_PACKET_SIZE) {
            len = PAD_PACKET_SIZE;
        }
        memcpy(pad_packet, utr->buff, len);
        pad_packet_valid = true;
    }

    if (pad == active_pad && utr->status >= 0) {
        utr->xfer_len = 0;
        utr->bIsTransferDone = 0;
        usbh_int_xfer(utr);
    } else if (pad == active_pad) {
        pad_read_queued = false;
        pad_packet_valid = false;
    }
}

static void input_init(void)
{
    usbh_core_init();
    usbh_xid_init();
    for (int i = 0; i < 500; ++i) {
        usbh_pooling_hubs();
        Sleep(1);
    }
}

static xid_dev_t *first_gamepad(void)
{
    xid_dev_t *pad = usbh_xid_get_device_list();
    while (pad != NULL) {
        if (pad->xid_desc.bType == XID_TYPE_GAMECONTROLLER) {
            return pad;
        }
        pad = pad->next;
    }
    return NULL;
}

static void input_poll(void)
{
    xid_dev_t *pad;

    usbh_pooling_hubs();
    pad = first_gamepad();
    if (pad != active_pad) {
        active_pad = pad;
        pad_read_queued = false;
        pad_packet_valid = false;
        memset(pad_packet, 0, sizeof(pad_packet));
    }
    if (active_pad != NULL && !pad_read_queued) {
        if (usbh_xid_read(active_pad, 0, pad_read_callback) == USBH_OK) {
            pad_read_queued = true;
        }
    }
}

static bool previous_pressed(void)
{
    uint16_t buttons;
    int16_t left_x;

    if (!pad_packet_valid) {
        return false;
    }
    buttons = (uint16_t)pad_packet[2] | ((uint16_t)pad_packet[3] << 8);
    left_x = read_s16_le(&pad_packet[12]);
    return (buttons & XID_DPAD_LEFT) != 0 ||
           left_x < -STICK_DEADZONE ||
           pad_packet[10] > TRIGGER_DEADZONE ||
           pad_packet[9] > BUTTON_DEADZONE;
}

static bool next_pressed(void)
{
    uint16_t buttons;
    int16_t left_x;

    if (!pad_packet_valid) {
        return false;
    }
    buttons = (uint16_t)pad_packet[2] | ((uint16_t)pad_packet[3] << 8);
    left_x = read_s16_le(&pad_packet[12]);
    return (buttons & XID_DPAD_RIGHT) != 0 ||
           left_x > STICK_DEADZONE ||
           pad_packet[11] > TRIGGER_DEADZONE ||
           pad_packet[8] > BUTTON_DEADZONE;
}
#endif

static void perspective(float fov_y, float aspect, float near_z, float far_z)
{
    float ymax = near_z * tanf(fov_y * (float)M_PI / 360.0f);
    float xmax = ymax * aspect;
    glFrustum(-xmax, xmax, -ymax, ymax, near_z, far_z);
}

static void drain_gl_errors(void)
{
    while (glGetError() != GL_NO_ERROR) {
    }
}

static void reset_matrix_stack(GLenum mode, GLenum depth_query)
{
    GLint depth = 1;

    glMatrixMode(mode);
    glGetIntegerv(depth_query, &depth);
    while (depth > 1) {
        glPopMatrix();
        --depth;
    }
    glLoadIdentity();
}

static void reset_attrib_stacks(void)
{
    GLint depth = 0;

    glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &depth);
    while (depth > 0) {
        glPopAttrib();
        --depth;
    }

    glGetIntegerv(GL_CLIENT_ATTRIB_STACK_DEPTH, &depth);
    while (depth > 0) {
        glPopClientAttrib();
        --depth;
    }
}

static void scene_reset_gl(void)
{
    static const GLenum disable_caps[] = {
        GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_GEN_S, GL_TEXTURE_GEN_T, GL_TEXTURE_GEN_R, GL_TEXTURE_GEN_Q,
        GL_BLEND, GL_ALPHA_TEST, GL_FOG, GL_LIGHTING, GL_COLOR_MATERIAL,
        GL_CULL_FACE, GL_SCISSOR_TEST, GL_STENCIL_TEST, GL_COLOR_LOGIC_OP,
        GL_LINE_STIPPLE, GL_POLYGON_OFFSET_POINT, GL_POLYGON_OFFSET_LINE,
        GL_POLYGON_OFFSET_FILL, GL_AUTO_NORMAL, GL_NORMALIZE, GL_RESCALE_NORMAL,
        GL_MULTISAMPLE, GL_SAMPLE_ALPHA_TO_COVERAGE, GL_SAMPLE_ALPHA_TO_ONE,
        GL_SAMPLE_COVERAGE
    };
    static const GLenum map_caps[] = {
        GL_MAP1_COLOR_4, GL_MAP1_NORMAL, GL_MAP1_TEXTURE_COORD_1,
        GL_MAP1_TEXTURE_COORD_2, GL_MAP1_TEXTURE_COORD_3,
        GL_MAP1_TEXTURE_COORD_4, GL_MAP1_VERTEX_3, GL_MAP1_VERTEX_4,
        GL_MAP1_INDEX, GL_MAP2_COLOR_4, GL_MAP2_NORMAL,
        GL_MAP2_TEXTURE_COORD_1, GL_MAP2_TEXTURE_COORD_2,
        GL_MAP2_TEXTURE_COORD_3, GL_MAP2_TEXTURE_COORD_4, GL_MAP2_VERTEX_3,
        GL_MAP2_VERTEX_4, GL_MAP2_INDEX
    };

    reset_attrib_stacks();
    drain_gl_errors();

    reset_matrix_stack(GL_MODELVIEW, GL_MODELVIEW_STACK_DEPTH);
    reset_matrix_stack(GL_PROJECTION, GL_PROJECTION_STACK_DEPTH);
    reset_matrix_stack(GL_TEXTURE, GL_TEXTURE_STACK_DEPTH);

    for (int unit = 0; unit < 4; ++unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glClientActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_1D, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_3D, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_3D);
        glDisable(GL_TEXTURE_CUBE_MAP);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_GEN_R);
        glDisable(GL_TEXTURE_GEN_Q);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(4, GL_FLOAT, 0, NULL);
    glColorPointer(4, GL_FLOAT, 0, NULL);
    glNormalPointer(GL_FLOAT, 0, NULL);
    glTexCoordPointer(4, GL_FLOAT, 0, NULL);

    for (unsigned int i = 0; i < sizeof(disable_caps) / sizeof(disable_caps[0]); ++i) {
        glDisable(disable_caps[i]);
    }
    for (unsigned int i = 0; i < sizeof(map_caps) / sizeof(map_caps[0]); ++i) {
        glDisable(map_caps[i]);
    }
    for (int i = 0; i < 8; ++i) {
        glDisable(GL_LIGHT0 + i);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glDepthRange(0.0f, 1.0f);
    glAlphaFunc(GL_ALWAYS, 0.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLogicOp(GL_COPY);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilFunc(GL_ALWAYS, 0, 0xffffffffu);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xffffffffu);
    glSampleCoverage(1.0f, GL_FALSE);
    glShadeModel(GL_SMOOTH);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPolygonOffset(0.0f, 0.0f);
    glPointSize(1.0f);
    glLineWidth(1.0f);
    glLineStipple(1, 0xffffu);
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord4f(0.0f, 0.0f, 0.0f, 1.0f);
    glRasterPos4f(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, NEHE_SCREEN_W, NEHE_SCREEN_H);
    glScissor(0, 0, NEHE_SCREEN_W, NEHE_SCREEN_H);
    glClearColor(0.02f, 0.025f, 0.04f, 1.0f);
    glClearIndex(0.0f);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClearAccum(0.0f, 0.0f, 0.0f, 0.0f);

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SKIP_IMAGES, 0);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
    glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelZoom(1.0f, 1.0f);
    glPixelTransferf(GL_RED_SCALE, 1.0f);
    glPixelTransferf(GL_GREEN_SCALE, 1.0f);
    glPixelTransferf(GL_BLUE_SCALE, 1.0f);
    glPixelTransferf(GL_ALPHA_SCALE, 1.0f);
    glPixelTransferf(GL_RED_BIAS, 0.0f);
    glPixelTransferf(GL_GREEN_BIAS, 0.0f);
    glPixelTransferf(GL_BLUE_BIAS, 0.0f);
    glPixelTransferf(GL_ALPHA_BIAS, 0.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    perspective(NEHE_FOV_Y_DEGREES, NEHE_ASPECT, NEHE_NEAR_Z, NEHE_FAR_Z);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    nxglSetCamera(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    drain_gl_errors();
}

static void frame_reset_gl(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    nxglSetCamera(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

static void flush_reset_viewport_boundary(void)
{
    scene_reset_gl();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, NEHE_SCREEN_W, NEHE_SCREEN_H);
    glScissor(0, 0, NEHE_SCREEN_W, NEHE_SCREEN_H);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    nxglSwapBuffers("NXGL scene reset", "view/state boundary");
    scene_reset_gl();
}

static void fill_video_pixels(float t)
{
    int frame = (int)(t * 12.0f);

    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            int i = (y * 64 + x) * 4;
            float nx = ((float)x - 31.5f) / 31.5f;
            float ny = ((float)y - 31.5f) / 31.5f;
            float rings = 0.5f + 0.5f * sinf((nx * nx + ny * ny) * 18.0f - t * 5.0f);
            float bars = (((x + frame * 3) / 8 + (y / 12)) & 1) ? 1.0f : 0.35f;

            video_pixels[i + 0] = (uint8_t)(40.0f + 200.0f * rings);
            video_pixels[i + 1] = (uint8_t)(50.0f + 180.0f * bars);
            video_pixels[i + 2] = (uint8_t)(90.0f + 120.0f * (1.0f - rings) + (float)((x + frame) & 15) * 2.0f);
            video_pixels[i + 3] = 255;
        }
    }
}

static void make_textures(void)
{
    nehe_texture_font_fill_rgba(font_pixels);
    fill_video_pixels(0.0f);

    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            int i = (y * 64 + x) * 4;
            int checker = (((x / 8) + (y / 8)) & 1) ? 230 : 48;
            checker_pixels[i + 0] = (uint8_t)checker;
            checker_pixels[i + 1] = (uint8_t)(checker == 230 ? 230 : 85);
            checker_pixels[i + 2] = (uint8_t)(checker == 230 ? 255 : 130);
            checker_pixels[i + 3] = 255;

            int border = (x < 4 || y < 4 || x > 59 || y > 59);
            int plank = ((x / 16) & 1) ? 126 : 88;
            crate_pixels[i + 0] = (uint8_t)(border ? 210 : plank);
            crate_pixels[i + 1] = (uint8_t)(border ? 165 : plank / 2);
            crate_pixels[i + 2] = (uint8_t)(border ? 82 : 28);
            crate_pixels[i + 3] = 255;

            world_pixels[i + 0] = (uint8_t)(30 + ((x * 3 + y) & 63));
            world_pixels[i + 1] = (uint8_t)(80 + ((x + y * 2) & 95));
            world_pixels[i + 2] = (uint8_t)(35 + ((x * y) & 47));
            world_pixels[i + 3] = 255;

            float nx = ((float)x - 31.5f) / 31.5f;
            float ny = ((float)y - 31.5f) / 31.5f;
            float radius = sqrtf(nx * nx + ny * ny);
            float light = 0.5f + 0.5f * sinf((float)x * 0.42f + sinf((float)y * 0.21f) * 2.5f);
            uint8_t bump = (uint8_t)(55.0f + light * 180.0f);
            uint8_t sphere = (uint8_t)((1.0f - fminf(radius, 1.0f)) * 255.0f);

            bump_pixels[i + 0] = (uint8_t)(bump / 3);
            bump_pixels[i + 1] = (uint8_t)(90 + bump / 2);
            bump_pixels[i + 2] = bump;
            bump_pixels[i + 3] = 255;

            sphere_pixels[i + 0] = (uint8_t)(20 + sphere / 5);
            sphere_pixels[i + 1] = (uint8_t)(80 + sphere / 2);
            sphere_pixels[i + 2] = (uint8_t)(140 + sphere / 3);
            sphere_pixels[i + 3] = 255;

            raw_pixels[i + 0] = (uint8_t)((x * 4) ^ (y * 2));
            raw_pixels[i + 1] = (uint8_t)(40 + ((x * y) & 191));
            raw_pixels[i + 2] = (uint8_t)(255 - ((x * 3 + y * 5) & 255));
            raw_pixels[i + 3] = 255;
        }
    }

    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            float dx = (float)x - 15.5f;
            float dy = (float)y - 15.5f;
            float dist = sqrtf(dx * dx + dy * dy);
            int a = (int)(255.0f - dist * 22.0f);
            if (a < 0) a = 0;
            int i = (y * 32 + x) * 4;
            star_pixels[i + 0] = 255;
            star_pixels[i + 1] = (uint8_t)(180 + (a / 4));
            star_pixels[i + 2] = 80;
            star_pixels[i + 3] = (uint8_t)a;

            float ndx = dx / 15.5f;
            float ndy = dy / 15.5f;
            float glow = 1.0f - sqrtf(ndx * ndx + ndy * ndy);
            uint8_t glow_byte;
            if (glow < 0.0f) glow = 0.0f;
            glow = glow * glow;
            glow_byte = (uint8_t)(glow * 255.0f + 0.5f);
            particle_pixels[i + 0] = glow_byte;
            particle_pixels[i + 1] = glow_byte;
            particle_pixels[i + 2] = glow_byte;
            particle_pixels[i + 3] = glow_byte;

            mask_pixels[i + 0] = glow > 0.42f ? 0 : 255;
            mask_pixels[i + 1] = mask_pixels[i + 0];
            mask_pixels[i + 2] = mask_pixels[i + 0];
            mask_pixels[i + 3] = 255;
        }
    }
}

static GLuint upload_texture(int width, int height, const uint8_t *pixels, GLint filter)
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    return texture;
}

static void draw_flat_triangle(void)
{
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 0.0f);
    glEnd();
}

static void draw_flat_quad(void)
{
    glBegin(GL_QUADS);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glEnd();
}

static void draw_colored_triangle(void)
{
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(1.0f, -1.0f, 0.0f);
    glEnd();
}

static void draw_pyramid(void)
{
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
    glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(1.0f, -1.0f, 1.0f);
    glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(1.0f, -1.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
    glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
    glColor3f(0.7f, 0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.7f, 0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
    glEnd();
}

static void draw_cube_shaded(bool textured, const GLfloat *face_shade)
{
    static const GLfloat faces[6][4][3] = {
        {{ 1, 1,-1}, {-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1}},
        {{ 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1}, { 1,-1,-1}},
        {{ 1, 1, 1}, {-1, 1, 1}, {-1,-1, 1}, { 1,-1, 1}},
        {{ 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1}},
        {{-1, 1, 1}, {-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1}},
        {{ 1, 1,-1}, { 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1}},
    };
    static const GLfloat colors[6][3] = {
        {0.0f, 0.8f, 0.0f}, {1.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f},
    };
    static const GLfloat normals[6][3] = {
        {0, 1, 0}, {0,-1, 0}, {0, 0, 1}, {0, 0,-1}, {-1,0,0}, {1,0,0},
    };
    static const GLfloat uv[4][2] = {
        {0.998f, 0.002f}, {0.002f, 0.002f}, {0.002f, 0.998f}, {0.998f, 0.998f}
    };

    glBegin(GL_QUADS);
    for (int f = 0; f < 6; ++f) {
        if (face_shade) {
            glColor3f(face_shade[f], face_shade[f], face_shade[f]);
        } else if (!textured) {
            glColor3fv(colors[f]);
        }
        glNormal3fv(normals[f]);
        for (int v = 0; v < 4; ++v) {
            if (textured) {
                glTexCoord2f(uv[v][0], uv[v][1]);
            }
            glVertex3fv(faces[f][v]);
        }
    }
    glEnd();
}

static void draw_cube(bool textured)
{
    draw_cube_shaded(textured, NULL);
}

static void draw_cube_colored(const GLfloat face_colors[6][3])
{
    static const GLfloat faces[6][4][3] = {
        {{ 1, 1,-1}, {-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1}},
        {{ 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1}, { 1,-1,-1}},
        {{ 1, 1, 1}, {-1, 1, 1}, {-1,-1, 1}, { 1,-1, 1}},
        {{ 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1}},
        {{-1, 1, 1}, {-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1}},
        {{ 1, 1,-1}, { 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1}},
    };

    glBegin(GL_QUADS);
    for (int f = 0; f < 6; ++f) {
        glColor3fv(face_colors[f]);
        for (int v = 0; v < 4; ++v) {
            glVertex3fv(faces[f][v]);
        }
    }
    glEnd();
}

static void rotate_axis_angle_vec3(const GLfloat in[3], GLfloat angle, GLfloat ax, GLfloat ay, GLfloat az, GLfloat out[3])
{
    GLfloat len = sqrtf(ax * ax + ay * ay + az * az);
    if (len <= 0.000001f) {
        out[0] = in[0];
        out[1] = in[1];
        out[2] = in[2];
        return;
    }

    ax /= len;
    ay /= len;
    az /= len;

    GLfloat c = cosf(angle);
    GLfloat s = sinf(angle);
    GLfloat one_c = 1.0f - c;

    out[0] = (c + ax * ax * one_c) * in[0] + (ax * ay * one_c - az * s) * in[1] + (ax * az * one_c + ay * s) * in[2];
    out[1] = (ay * ax * one_c + az * s) * in[0] + (c + ay * ay * one_c) * in[1] + (ay * az * one_c - ax * s) * in[2];
    out[2] = (az * ax * one_c - ay * s) * in[0] + (az * ay * one_c + ax * s) * in[1] + (c + az * az * one_c) * in[2];
}

static void lesson_07_face_shade(float t, GLfloat shade[6])
{
    static const GLfloat normals[6][3] = {
        {0.0f, 1.0f, 0.0f}, {0.0f,-1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f,-1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
    };
    GLfloat angle = t * 42.0f * NEHE_DEG_TO_RAD;

    for (int i = 0; i < 6; ++i) {
        GLfloat normal[3];
        rotate_axis_angle_vec3(normals[i], angle, 1.0f, 1.0f, 0.0f, normal);
        GLfloat diffuse = normal[2] > 0.0f ? normal[2] : 0.0f;
        shade[i] = 0.2f + diffuse * 0.8f;
    }
}

static void draw_textured_floor(float z)
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f, -1.0f, z);
    glTexCoord2f(4.0f, 0.0f); glVertex3f(4.0f, -1.0f, z);
    glTexCoord2f(4.0f, 4.0f); glVertex3f(4.0f, -1.0f, z - 6.0f);
    glTexCoord2f(0.0f, 4.0f); glVertex3f(-4.0f, -1.0f, z - 6.0f);
    glEnd();
}

static void draw_bitmap_text_quads(const char *text, float x, float y, float z, float cell)
{
    float cursor = x;

    glBegin(GL_QUADS);
    for (const char *p = text; *p != '\0'; ++p) {
        const uint8_t *rows = nehe_font_rows(*p);
        for (int row = 0; row < NEHE_FONT_ROWS; ++row) {
            for (int col = 0; col < NEHE_FONT_COLS; ++col) {
                if (nehe_font_row_has_pixel(rows, row, col)) {
                    float x0 = cursor + (float)col * cell;
                    float y0 = y - (float)row * cell;
                    float x1 = x0 + cell * 0.82f;
                    float y1 = y0 - cell * 0.82f;

                    glVertex3f(x0, y0, z);
                    glVertex3f(x1, y0, z);
                    glVertex3f(x1, y1, z);
                    glVertex3f(x0, y1, z);
                }
            }
        }
        cursor += cell * 6.0f;
    }
    glEnd();
}

static void outline_vertex_at(float x, float y, float z, float rot, float view_z)
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

    glVertex3f(x3, y3, z2 + view_z);
}

static void outline_vertex(float x, float y, float z, float rot)
{
    outline_vertex_at(x, y, z, rot, -16.0f);
}

static void draw_outline_pixel_quads(float x0, float y0, float x1, float y1, float z, float depth, float rot)
{
    (void)depth;
    outline_vertex(x0, y0, z, rot);
    outline_vertex(x1, y0, z, rot);
    outline_vertex(x1, y1, z, rot);
    outline_vertex(x0, y1, z, rot);
}

static void draw_outline_text_quads(const char *text, float cell, float depth, float rot)
{
    float cursor = -nehe_outline_text_width(text, cell) * 0.5f;

    glBegin(GL_QUADS);
    for (const char *p = text; *p != '\0'; ++p) {
        const uint8_t *rows = nehe_font_rows(*p);
        for (int row = 0; row < NEHE_FONT_ROWS; ++row) {
            for (int col = 0; col < NEHE_FONT_COLS; ++col) {
                if (nehe_outline_font_pixel(rows, row, col)) {
                    float x0 = cursor + (float)col * cell;
                    float y0 = 0.38f - (float)row * cell;
                    float x1 = x0 + cell * 0.78f;
                    float y1 = y0 - cell * 0.78f;

                    draw_outline_pixel_quads(x0, y0, x1, y1, 0.0f, depth, rot);
                }
            }
        }
        cursor += cell * 6.0f;
    }
    glEnd();
}

static void init_texture_checker(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_NEHE_W, NEHE_ASSET_NEHE_H, nehe_asset_nehe_rgba, GL_LINEAR);
}

static void init_texture_crate_filters(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba, GL_NEAREST);
    lesson->textures[1] = upload_texture(NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba, GL_LINEAR);
}

static void init_texture_crate_linear(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba, GL_LINEAR);
}

static void init_texture_glass(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_GLASS_W, NEHE_ASSET_GLASS_H, nehe_asset_glass_rgba, GL_LINEAR);
}

static void init_star_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_STAR_W, NEHE_ASSET_STAR_H, nehe_asset_star_rgba, GL_LINEAR);
}

static void init_lights_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_STAR_W, NEHE_ASSET_STAR_H, nehe_asset_star_rgba, GL_LINEAR);
}

static void init_world_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_MUD_W, NEHE_ASSET_MUD_H, nehe_asset_mud_rgba, GL_LINEAR);
}

static void init_flag_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_TIM_W, NEHE_ASSET_TIM_H, nehe_asset_tim_rgba, GL_LINEAR);
}

static void init_display_lists(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba, GL_LINEAR);
    lesson->lists[0] = glGenLists(1);
    glNewList(lesson->lists[0], GL_COMPILE);
    draw_cube(true);
    glEndList();
}

static void init_texture_font(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_TEXTURE_FONT_SIZE, NEHE_TEXTURE_FONT_SIZE, font_pixels, GL_LINEAR);
    lesson->textures[1] = upload_texture(NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba, GL_LINEAR);
}

static void init_particle_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(32, 32, particle_pixels, GL_LINEAR);
}

static void init_crate_particle_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_CRATE_W, NEHE_ASSET_CRATE_H, nehe_asset_crate_rgba, GL_LINEAR);
    lesson->textures[1] = upload_texture(32, 32, particle_pixels, GL_LINEAR);
}

static void init_mask_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_NEHE_W, NEHE_ASSET_NEHE_H, nehe_asset_nehe_rgba, GL_LINEAR);
    lesson->textures[1] = upload_texture(32, 32, mask_pixels, GL_LINEAR);
    lesson->textures[2] = upload_texture(32, 32, particle_pixels, GL_LINEAR);
}

static void init_bump_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(64, 64, bump_pixels, GL_LINEAR);
}

static void init_sphere_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(64, 64, sphere_pixels, GL_LINEAR);
}

static void init_scissor_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba, GL_LINEAR);
}

static void init_raw_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(64, 64, raw_pixels, GL_LINEAR);
}

static void init_video_texture(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(64, 64, video_pixels, GL_LINEAR);
}

static void init_tga_variant_textures(Lesson *lesson)
{
    lesson->textures[0] = upload_texture(NEHE_ASSET_NEHE_W, NEHE_ASSET_NEHE_H, nehe_asset_nehe_rgba, GL_LINEAR);
    lesson->textures[1] = upload_texture(NEHE_ASSET_CUBE_W, NEHE_ASSET_CUBE_H, nehe_asset_cube_rgba, GL_LINEAR);
    lesson->textures[2] = upload_texture(NEHE_ASSET_TIM_W, NEHE_ASSET_TIM_H, nehe_asset_tim_rgba, GL_LINEAR);
    lesson->textures[3] = upload_texture(64, 64, raw_pixels, GL_LINEAR);
}

static void shutdown_default(Lesson *lesson)
{
    for (int i = 0; i < 4; ++i) {
        if (lesson->textures[i] != 0) {
            glDeleteTextures(1, &lesson->textures[i]);
            lesson->textures[i] = 0;
        }
        if (lesson->lists[i] != 0) {
            glDeleteLists(lesson->lists[i], i == 0 ? 2 : 1);
            lesson->lists[i] = 0;
        }
    }
}

static void shutdown_texture_font(Lesson *lesson)
{
    for (int i = 0; i < 4; ++i) {
        if (lesson->textures[i] != 0) {
            glDeleteTextures(1, &lesson->textures[i]);
            lesson->textures[i] = 0;
        }
    }
    if (lesson->lists[0] != 0) {
        glDeleteLists(lesson->lists[0], 256);
        lesson->lists[0] = 0;
    }
}

static void render_01(Lesson *lesson, float t)
{
    (void)lesson;
    (void)t;
    glClearColor(0.03f, 0.04f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void render_02(Lesson *lesson, float t)
{
    (void)lesson;
    (void)t;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(-1.5f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    glColor3f(1.0f, 1.0f, 1.0f);
    draw_flat_triangle();
    glTranslatef(3.0f, 0.0f, 0.0f);
    draw_flat_quad();
}

static void render_03(Lesson *lesson, float t)
{
    (void)lesson;
    (void)t;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(-1.5f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    draw_colored_triangle();
    glLoadIdentity();
    glTranslatef(1.5f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    glColor3f(0.2f, 0.45f, 1.0f);
    draw_flat_quad();
}

static void render_04(Lesson *lesson, float t)
{
    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(-1.5f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    glRotatef(t * 70.0f, 0.0f, 1.0f, 0.0f);
    draw_colored_triangle();
    glLoadIdentity();
    glTranslatef(1.5f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    glRotatef(t * 55.0f, 1.0f, 1.0f, 0.0f);
    glColor3f(0.2f, 0.45f, 1.0f);
    draw_flat_quad();
}

static void render_05(Lesson *lesson, float t)
{
    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(-1.5f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    glRotatef(t * 70.0f, 0.0f, 1.0f, 0.0f);
    draw_pyramid();
    glLoadIdentity();
    glTranslatef(1.5f, 0.0f, -7.0f);
    glRotatef(-t * 9.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    draw_cube(false);
    glDisable(GL_CULL_FACE);
}

static void render_06(Lesson *lesson, float t)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    glRotatef(t * 45.0f, 1.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    draw_cube(true);
    glDisable(GL_CULL_FACE);
}

static void render_07(Lesson *lesson, float t)
{
    GLfloat shade[6];
    int use_linear = ((int)(t * 1.25f) & 1);

    lesson_07_face_shade(t, shade);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[use_linear ? 1 : 0]);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    glRotatef(t * 42.0f, 1.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    draw_cube_shaded(true, shade);
    glDisable(GL_CULL_FACE);
}

static void render_08(Lesson *lesson, float t)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    glRotatef(t * 40.0f, 1.0f, 1.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.65f);
    draw_cube(true);
}

static void render_09(Lesson *lesson, float t)
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = 0; i < NEHE_STAR_COUNT; ++i) {
        float x;
        float y;
        float spin;
        float r;
        float g;
        float b;

        nehe_star_state(i, t, &x, &y, &spin);
        nehe_star_color(i, &r, &g, &b);
        glLoadIdentity();
        glTranslatef(x, y, NEHE_STAR_Z);
        glRotatef(spin, 0.0f, 0.0f, 1.0f);
        glColor4f(r, g, b, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-1.0f, -1.0f, 0);
        glTexCoord2f(1, 0); glVertex3f(1.0f, -1.0f, 0);
        glTexCoord2f(1, 1); glVertex3f(1.0f, 1.0f, 0);
        glTexCoord2f(0, 1); glVertex3f(-1.0f, 1.0f, 0);
        glEnd();
    }
}

static void render_10(Lesson *lesson, float t)
{
    float bob = sinf(t * 1.6f) * 0.04f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, -0.55f + bob, -6.0f);
    glRotatef(sinf(t * 0.35f) * 7.0f, 0.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < NEHE_WORLD_TRIANGLES; ++i) {
        for (int v = 0; v < 3; ++v) {
            glTexCoord2f(nehe_world_vertices[i][v][3], nehe_world_vertices[i][v][4]);
            glVertex3f(nehe_world_vertices[i][v][0], nehe_world_vertices[i][v][1], nehe_world_vertices[i][v][2]);
        }
    }
    glEnd();
}

static void render_11(Lesson *lesson, float t)
{
    const int cells = 22;
    const float scale = 4.4f / (float)cells;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (int y = 0; y < cells; ++y) {
        glBegin(GL_QUADS);
        for (int x = 0; x < cells; ++x) {
            float x0 = ((float)x - (float)cells * 0.5f) * scale;
            float x1 = ((float)(x + 1) - (float)cells * 0.5f) * scale;
            float y0 = ((float)y - (float)cells * 0.5f) * scale;
            float y1 = ((float)(y + 1) - (float)cells * 0.5f) * scale;
            float z00 = sinf(t * 2.2f + x0 * 2.0f + y0 * 1.4f) * 0.25f;
            float z10 = sinf(t * 2.2f + x1 * 2.0f + y0 * 1.4f) * 0.25f;
            float z11 = sinf(t * 2.2f + x1 * 2.0f + y1 * 1.4f) * 0.25f;
            float z01 = sinf(t * 2.2f + x0 * 2.0f + y1 * 1.4f) * 0.25f;
            float u0 = (float)x / (float)cells;
            float u1 = (float)(x + 1) / (float)cells;
            float v0 = (float)y / (float)cells;
            float v1 = (float)(y + 1) / (float)cells;

            glTexCoord2f(u0, v0); glVertex3f(x0, y0, z00);
            glTexCoord2f(u1, v0); glVertex3f(x1, y0, z10);
            glTexCoord2f(u1, v1); glVertex3f(x1, y1, z11);
            glTexCoord2f(u0, v1); glVertex3f(x0, y1, z01);
        }
        glEnd();
    }
}

static void render_12(Lesson *lesson, float t)
{
    static const GLfloat boxcol[5][3] = {
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.5f, 0.0f },
        { 1.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 1.0f },
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    for (int yloop = 5; yloop >= 1; --yloop) {
        for (int xloop = 0; xloop < yloop; ++xloop) {
            glLoadIdentity();
            glTranslatef(((float)xloop - ((float)yloop - 1.0f) * 0.5f) * 0.58f,
                         (3.0f - (float)yloop) * 0.42f,
                         -5.5f);
            glRotatef(28.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(35.0f, 0.0f, 1.0f, 0.0f);
            glScalef(0.26f, 0.26f, 0.26f);
            glColor3fv(boxcol[yloop - 1]);
            glCallList(lesson->lists[0]);
        }
    }
}

static void render_13(Lesson *lesson, float t)
{
    float cnt1 = t * 0.60f;
    float cnt2 = t * 0.486f;
    float r = fmaxf(0.0f, cosf(cnt1));
    float g = fmaxf(0.0f, sinf(cnt2));
    float b = fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(cnt1 + cnt2)));
    float x = -0.82f + 0.35f * cosf(cnt1);
    float y = 0.08f + 0.35f * sinf(cnt2);

    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glLoadIdentity();
    glRasterPos2f(-0.2f + 0.35f * cosf(cnt1), 0.35f * sinf(cnt2));
    glColor3f(r, g, b);
    draw_bitmap_text_quads("OpenGL With NeHe", x, y, -2.0f, 0.017f);
}

static void render_14(Lesson *lesson, float t)
{
    float rot = t * 3.0f;
    float r = fmaxf(0.0f, cosf(rot / 20.0f));
    float g = fmaxf(0.0f, sinf(rot / 25.0f));
    float b = fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(rot / 17.0f)));

    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
    glLoadIdentity();
    glColor3f(r, g, b);
    draw_outline_text_quads("OpenGL With NeHe", 0.080f, 0.12f, rot);
}

static void draw_textured_symbol_pixel(float x0, float y0, float x1, float y1, float z, float rot,
                                       float u0, float v0, float u1, float v1)
{
    glTexCoord2f(u0, v0);
    outline_vertex_at(x0, y0, z, rot, -5.6f);
    glTexCoord2f(u1, v0);
    outline_vertex_at(x1, y0, z, rot, -5.6f);
    glTexCoord2f(u1, v1);
    outline_vertex_at(x1, y1, z, rot, -5.6f);
    glTexCoord2f(u0, v1);
    outline_vertex_at(x0, y1, z, rot, -5.6f);
}

static void draw_textured_skull_quads(float ox, float oy, float rot)
{
    const float cell = 0.145f;
    float left = ox - (float)NEHE_SYMBOL_COLS * cell * 0.5f;
    float top = oy + (float)NEHE_SYMBOL_ROWS * cell * 0.5f;

    glBegin(GL_QUADS);
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

                draw_textured_symbol_pixel(x0, y0, x1, y1, -0.10f, rot, u0, v0, u1, v1);
                draw_textured_symbol_pixel(x0, y0, x1, y1, 0.0f, rot, u0, v0, u1, v1);
            }
        }
    }
    glEnd();
}

static void render_15(Lesson *lesson, float t)
{
    float rot = t * 55.0f;
    float x = sinf(t * 0.7f) * 0.35f;
    float y = cosf(t * 0.5f) * 0.22f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
    glLoadIdentity();
    glColor3f(1.0f, 1.0f, 1.0f);
    draw_textured_skull_quads(x, y, rot);
}

static void setup_lesson_16_fog(float t)
{
    static const GLenum modes[3] = { GL_EXP, GL_EXP2, GL_LINEAR };
    GLfloat fog_color[] = { 0.50f, 0.50f, 0.50f, 1.0f };
    int mode = ((int)(t / 2.5f)) % 3;

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, modes[mode]);
    glFogfv(GL_FOG_COLOR, fog_color);
    glFogf(GL_FOG_DENSITY, 0.16f);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_START, 3.8f);
    glFogf(GL_FOG_END, 9.2f);
}

static void draw_fogged_crate_gl(Lesson *lesson, float x, float z, float angle)
{
    glLoadIdentity();
    glTranslatef(x, 0.0f, z - 5.8f);
    glRotatef(angle, 1.0f, 1.0f, 0.0f);
    glScalef(0.62f, 0.62f, 0.62f);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    draw_cube(true);
}

static void render_16(Lesson *lesson, float t)
{
    GLfloat light_pos[] = { 0.0f, 0.0f, 2.0f, 1.0f };
    GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float angle = t * 42.0f;

    glClearColor(0.50f, 0.50f, 0.50f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setup_lesson_16_fog(t);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glColor3f(1.0f, 1.0f, 1.0f);
    draw_fogged_crate_gl(lesson, -1.25f, 0.0f, angle);
    draw_fogged_crate_gl(lesson, 0.0f, -1.45f, angle);
    draw_fogged_crate_gl(lesson, 1.25f, -2.9f, angle);
}

static void draw_lesson_17_object_gl(Lesson *lesson, float t)
{
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[1]);
    glEnable(GL_DEPTH_TEST);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(t * 30.0f, 1.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
    glEnd();

    glRotatef(90.0f, 1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
    glEnd();
}

static void print_texture_font_gl(Lesson *lesson, int x, int y, const char *text, int set)
{
    float cursor = (float)x;

    if (set > 1) {
        set = 1;
    }
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)NEHE_SCREEN_W, 0.0, (double)NEHE_SCREEN_H, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_QUADS);
    for (const char *p = text; *p != '\0'; ++p) {
        float u0, v0, u1, v1;

        nehe_texture_font_uv((unsigned char)*p, set, &u0, &v0, &u1, &v1);
        glTexCoord2f(u0, v0); glVertex3f(cursor, (float)y, 0.0f);
        glTexCoord2f(u1, v0); glVertex3f(cursor + 16.0f, (float)y, 0.0f);
        glTexCoord2f(u1, v1); glVertex3f(cursor + 16.0f, (float)y + 16.0f, 0.0f);
        glTexCoord2f(u0, v1); glVertex3f(cursor, (float)y + 16.0f, 0.0f);
        cursor += 10.0f;
    }
    glEnd();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

static void render_17(Lesson *lesson, float t)
{
    float cnt1 = t * 0.60f;
    float cnt2 = t * 0.486f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_lesson_17_object_gl(lesson, t);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLoadIdentity();

    glColor3f(fmaxf(0.0f, cosf(cnt1)),
              fmaxf(0.0f, sinf(cnt2)),
              fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(cnt1 + cnt2))));
    print_texture_font_gl(lesson, (int)(280.0f + 250.0f * cosf(cnt1)),
                          (int)(235.0f + 200.0f * sinf(cnt2)), "NeHe", 0);

    glColor3f(fmaxf(0.0f, sinf(cnt2)),
              fmaxf(0.0f, fminf(1.0f, 1.0f - 0.5f * cosf(cnt1 + cnt2))),
              fmaxf(0.0f, cosf(cnt1)));
    print_texture_font_gl(lesson, (int)(280.0f + 230.0f * cosf(cnt2)),
                          (int)(235.0f + 200.0f * sinf(cnt1)), "OpenGL", 1);

    glColor3f(0.0f, 0.0f, 1.0f);
    print_texture_font_gl(lesson, (int)(240.0f + 200.0f * cosf((cnt1 + cnt2) / 5.0f)), 2, "Giuseppe D'Agata", 0);
    glColor3f(1.0f, 1.0f, 1.0f);
    print_texture_font_gl(lesson, (int)(242.0f + 200.0f * cosf((cnt1 + cnt2) / 5.0f)), 4, "Giuseppe D'Agata", 0);
}

#define NEHE_QUADRIC_GL_PI 3.14159265358979323846f
#define NEHE_QUADRIC_GL_SLICES 32
#define NEHE_QUADRIC_GL_STACKS 24
#define NEHE_QUADRIC_GL_CYCLE_SECONDS 2.0f

static int lesson_18_object_gl(float t)
{
    int object = (int)(t / NEHE_QUADRIC_GL_CYCLE_SECONDS) % 6;
    return object < 0 ? 0 : object;
}

static float lesson_18_local_time_gl(float t)
{
    float local = fmodf(t, NEHE_QUADRIC_GL_CYCLE_SECONDS);
    return local < 0.0f ? local + NEHE_QUADRIC_GL_CYCLE_SECONDS : local;
}

static void draw_cylinder_quadric_gl(float base_radius, float top_radius, float height)
{
    float slope = (base_radius - top_radius) / height;

    glBegin(GL_QUADS);
    for (int stack = 0; stack < NEHE_QUADRIC_GL_STACKS; ++stack) {
        float t0 = (float)stack / (float)NEHE_QUADRIC_GL_STACKS;
        float t1 = (float)(stack + 1) / (float)NEHE_QUADRIC_GL_STACKS;
        float z0 = t0 * height - height * 0.5f;
        float z1 = t1 * height - height * 0.5f;
        float r0 = base_radius + (top_radius - base_radius) * t0;
        float r1 = base_radius + (top_radius - base_radius) * t1;

        for (int slice = 0; slice < NEHE_QUADRIC_GL_SLICES; ++slice) {
            float u0 = (float)slice / (float)NEHE_QUADRIC_GL_SLICES;
            float u1 = (float)(slice + 1) / (float)NEHE_QUADRIC_GL_SLICES;
            float a0 = u0 * NEHE_QUADRIC_GL_PI * 2.0f;
            float a1 = u1 * NEHE_QUADRIC_GL_PI * 2.0f;
            float n0_len = sqrtf(1.0f + slope * slope);
            float n1_len = n0_len;
            float n0x = cosf(a0) / n0_len, n0y = sinf(a0) / n0_len, n0z = slope / n0_len;
            float n1x = cosf(a1) / n1_len, n1y = sinf(a1) / n1_len, n1z = slope / n1_len;

            glNormal3f(n0x, n0y, n0z); glTexCoord2f(u0, t0); glVertex3f(cosf(a0) * r0, sinf(a0) * r0, z0);
            glNormal3f(n1x, n1y, n1z); glTexCoord2f(u1, t0); glVertex3f(cosf(a1) * r0, sinf(a1) * r0, z0);
            glNormal3f(n1x, n1y, n1z); glTexCoord2f(u1, t1); glVertex3f(cosf(a1) * r1, sinf(a1) * r1, z1);
            glNormal3f(n0x, n0y, n0z); glTexCoord2f(u0, t1); glVertex3f(cosf(a0) * r1, sinf(a0) * r1, z1);
        }
    }
    glEnd();
}

static void draw_disk_quadric_gl(float inner_radius, float outer_radius, float start_deg, float sweep_deg)
{
    int slices = sweep_deg < 359.9f ? 24 : NEHE_QUADRIC_GL_SLICES;
    float start = start_deg * NEHE_DEG_TO_RAD;
    float sweep = sweep_deg * NEHE_DEG_TO_RAD;

    if (sweep_deg <= 0.0f) {
        return;
    }

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int ring = 0; ring < NEHE_QUADRIC_GL_STACKS; ++ring) {
        float r0 = inner_radius + (outer_radius - inner_radius) * (float)ring / (float)NEHE_QUADRIC_GL_STACKS;
        float r1 = inner_radius + (outer_radius - inner_radius) * (float)(ring + 1) / (float)NEHE_QUADRIC_GL_STACKS;

        for (int slice = 0; slice < slices; ++slice) {
            float u0 = (float)slice / (float)slices;
            float u1 = (float)(slice + 1) / (float)slices;
            float v0 = (float)ring / (float)NEHE_QUADRIC_GL_STACKS;
            float v1 = (float)(ring + 1) / (float)NEHE_QUADRIC_GL_STACKS;
            float a0 = start + sweep * u0;
            float a1 = start + sweep * u1;

            glTexCoord2f(u0, v0); glVertex3f(cosf(a0) * r0, sinf(a0) * r0, 0.0f);
            glTexCoord2f(u1, v0); glVertex3f(cosf(a1) * r0, sinf(a1) * r0, 0.0f);
            glTexCoord2f(u1, v1); glVertex3f(cosf(a1) * r1, sinf(a1) * r1, 0.0f);
            glTexCoord2f(u0, v1); glVertex3f(cosf(a0) * r1, sinf(a0) * r1, 0.0f);
        }
    }
    glEnd();
}

static void draw_sphere_quadric_gl(float radius)
{
    glBegin(GL_QUADS);
    for (int stack = 0; stack < NEHE_QUADRIC_GL_STACKS; ++stack) {
        float v0 = (float)stack / (float)NEHE_QUADRIC_GL_STACKS;
        float v1 = (float)(stack + 1) / (float)NEHE_QUADRIC_GL_STACKS;
        float phi0 = -NEHE_QUADRIC_GL_PI * 0.5f + v0 * NEHE_QUADRIC_GL_PI;
        float phi1 = -NEHE_QUADRIC_GL_PI * 0.5f + v1 * NEHE_QUADRIC_GL_PI;

        for (int slice = 0; slice < NEHE_QUADRIC_GL_SLICES; ++slice) {
            float u0 = (float)slice / (float)NEHE_QUADRIC_GL_SLICES;
            float u1 = (float)(slice + 1) / (float)NEHE_QUADRIC_GL_SLICES;
            float a0 = u0 * NEHE_QUADRIC_GL_PI * 2.0f;
            float a1 = u1 * NEHE_QUADRIC_GL_PI * 2.0f;
            float n0x = cosf(phi0) * cosf(a0), n0y = cosf(phi0) * sinf(a0), n0z = sinf(phi0);
            float n1x = cosf(phi0) * cosf(a1), n1y = cosf(phi0) * sinf(a1), n1z = sinf(phi0);
            float n2x = cosf(phi1) * cosf(a1), n2y = cosf(phi1) * sinf(a1), n2z = sinf(phi1);
            float n3x = cosf(phi1) * cosf(a0), n3y = cosf(phi1) * sinf(a0), n3z = sinf(phi1);

            glNormal3f(n0x, n0y, n0z); glTexCoord2f(u0, v0); glVertex3f(n0x * radius, n0y * radius, n0z * radius);
            glNormal3f(n1x, n1y, n1z); glTexCoord2f(u1, v0); glVertex3f(n1x * radius, n1y * radius, n1z * radius);
            glNormal3f(n2x, n2y, n2z); glTexCoord2f(u1, v1); glVertex3f(n2x * radius, n2y * radius, n2z * radius);
            glNormal3f(n3x, n3y, n3z); glTexCoord2f(u0, v1); glVertex3f(n3x * radius, n3y * radius, n3z * radius);
        }
    }
    glEnd();
}

static void render_18(Lesson *lesson, float t)
{
    int object = lesson_18_object_gl(t);
    float local = lesson_18_local_time_gl(t);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glColor3f(1.0f, 1.0f, 1.0f);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(t * 45.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(t * 33.0f, 0.0f, 1.0f, 0.0f);

    switch (object) {
    case 0: draw_cube(true); break;
    case 1: draw_cylinder_quadric_gl(1.0f, 1.0f, 3.0f); break;
    case 2: draw_disk_quadric_gl(0.5f, 1.5f, 0.0f, 360.0f); break;
    case 3: draw_sphere_quadric_gl(1.3f); break;
    case 4: draw_cylinder_quadric_gl(1.0f, 0.0f, 3.0f); break;
    default: {
        float phase = local / NEHE_QUADRIC_GL_CYCLE_SECONDS;
        float start = phase < 0.5f ? 0.0f : (phase - 0.5f) * 720.0f;
        float sweep = phase < 0.5f ? phase * 720.0f : 360.0f - start;
        draw_disk_quadric_gl(0.5f, 1.5f, start, sweep);
        break;
    }
    }
}

#define NEHE_PARTICLE_GL_COUNT 220
#define NEHE_PARTICLE_GL_LIFE 4.0f

static const GLfloat particle_palette_gl[12][3] = {
    { 1.0f, 0.2f, 0.2f },
    { 1.0f, 0.5f, 0.1f },
    { 1.0f, 0.9f, 0.1f },
    { 0.5f, 1.0f, 0.1f },
    { 0.1f, 1.0f, 0.3f },
    { 0.1f, 1.0f, 0.9f },
    { 0.1f, 0.6f, 1.0f },
    { 0.2f, 0.2f, 1.0f },
    { 0.6f, 0.2f, 1.0f },
    { 1.0f, 0.2f, 1.0f },
    { 1.0f, 0.2f, 0.6f },
    { 1.0f, 1.0f, 1.0f },
};

static float lesson_19_unit_gl(int index, uint32_t salt)
{
    uint32_t h = (uint32_t)index * 1664525u + salt * 1013904223u;

    h ^= h >> 16;
    h *= 2246822519u;
    h ^= h >> 13;
    return (float)(h & 0xffffu) / 65535.0f;
}

static void render_19(Lesson *lesson, float t)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);

    glBegin(GL_QUADS);
    for (int i = 0; i < NEHE_PARTICLE_GL_COUNT; ++i) {
        float phase = lesson_19_unit_gl(i, 1u) * NEHE_PARTICLE_GL_LIFE;
        float age = fmodf(t + phase, NEHE_PARTICLE_GL_LIFE);
        float u = age / NEHE_PARTICLE_GL_LIFE;
        float fade = 1.0f - u;
        float spread = lesson_19_unit_gl(i, 2u) * (float)M_PI * 2.0f;
        float vx = cosf(spread) * (0.25f + lesson_19_unit_gl(i, 3u) * 1.05f);
        float vz = sinf(spread) * (0.15f + lesson_19_unit_gl(i, 4u) * 0.75f);
        float vy = 1.25f + lesson_19_unit_gl(i, 5u) * 1.45f;
        float x = vx * age + sinf(t * 1.7f + (float)i * 0.37f) * 0.12f;
        float y = -1.35f + vy * age - 0.68f * age * age;
        float z = vz * age;
        float size = 0.11f + fade * 0.20f;
        const GLfloat *color = particle_palette_gl[(i + (int)(t * 9.0f)) % 12];

        glColor4f(color[0], color[1], color[2], fade * fade);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(x - size, y - size, z);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(x + size, y - size, z);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(x + size, y + size, z);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(x - size, y + size, z);
    }
    glEnd();
    glDepthMask(GL_TRUE);
}

static void draw_textured_quad_xy(float cx, float cy, float z, float w, float h, float rot)
{
    float c = cosf(rot);
    float s = sinf(rot);
    float hw = w * 0.5f;
    float hh = h * 0.5f;
    static const GLfloat uv[4][2] = {
        {0.002f, 0.002f}, {0.998f, 0.002f}, {0.998f, 0.998f}, {0.002f, 0.998f}
    };
    GLfloat local[4][2] = {
        {-hw, -hh}, { hw, -hh}, { hw,  hh}, {-hw,  hh},
    };

    glBegin(GL_QUADS);
    for (int i = 0; i < 4; ++i) {
        float x = local[i][0];
        float y = local[i][1];
        glTexCoord2f(uv[i][0], uv[i][1]);
        glVertex3f(cx + x * c - y * s, cy + x * s + y * c, z);
    }
    glEnd();
}

static void render_20(Lesson *lesson, float t)
{
    float wobble = sinf(t * 1.7f) * 0.28f;
    float roll = t * 70.0f * NEHE_DEG_TO_RAD;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -2.8f);

    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    draw_textured_quad_xy(0.0f, 0.0f, -0.04f, 2.6f, 1.65f, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[1]);
    draw_textured_quad_xy(wobble, 0.0f, 0.0f, 1.35f, 1.35f, roll);

    glBlendFunc(GL_ONE, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[2]);
    glColor4f(0.35f, 0.9f, 1.0f, 1.0f);
    draw_textured_quad_xy(wobble, 0.0f, 0.01f, 1.35f, 1.35f, roll);
    glDepthMask(GL_TRUE);
}

static void draw_lesson_21_line(float x0, float y0, float x1, float y1, float width)
{
    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = sqrtf(dx * dx + dy * dy);
    float nx;
    float ny;

    if (len < 0.0001f) {
        return;
    }
    nx = -dy / len * width;
    ny = dx / len * width;
    glBegin(GL_QUADS);
    glVertex3f(x0 + nx, y0 + ny, 0.0f);
    glVertex3f(x1 + nx, y1 + ny, 0.0f);
    glVertex3f(x1 - nx, y1 - ny, 0.0f);
    glVertex3f(x0 - nx, y0 - ny, 0.0f);
    glEnd();
}

static void render_21(Lesson *lesson, float t)
{
    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.0f);

    for (int i = 0; i < 24; ++i) {
        float a = t * 0.7f + (float)i * (float)M_PI / 12.0f;
        float r0 = 0.25f + 0.04f * (float)(i & 3);
        float r1 = 2.1f + 0.18f * sinf(t * 1.3f + (float)i);
        glColor4f(0.35f + 0.65f * fabsf(sinf(a)),
                  0.35f + 0.65f * fabsf(sinf(a + 2.1f)),
                  0.35f + 0.65f * fabsf(sinf(a + 4.2f)),
                  0.42f);
        draw_lesson_21_line(cosf(a) * r0, sinf(a) * r0, cosf(a) * r1, sinf(a) * r1, 0.012f);
    }
    for (int i = 0; i < 18; ++i) {
        float x0 = -2.1f + (float)i * 0.25f;
        float x1 = x0 + 0.18f;
        float y = -1.4f + sinf(t * 3.2f + (float)i * 0.55f) * 0.23f;
        glColor4f(0.9f, 0.9f, 1.0f, 0.72f);
        draw_lesson_21_line(x0, y, x1, -y, 0.018f);
    }
    glDepthMask(GL_TRUE);
}

static void render_22(Lesson *lesson, float t)
{
    static const GLfloat bump_faces[6] = { 1.15f, 0.65f, 1.20f, 0.55f, 0.95f, 0.75f };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(t * 58.0f, 0.45f, 1.0f, 0.25f);
    glScalef(1.0f, 1.0f, 1.0f);
    draw_cube_shaded(true, bump_faces);
}

static void render_23(Lesson *lesson, float t)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.6f);
    glRotatef(t * 34.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(t * 48.0f, 0.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    draw_sphere_quadric_gl(1.35f);
}

static void render_24(Lesson *lesson, float t)
{
    static const GLfloat colors[4][3] = {
        { 1.0f, 0.35f, 0.25f },
        { 0.25f, 1.0f, 0.45f },
        { 0.25f, 0.55f, 1.0f },
        { 1.0f, 0.9f, 0.25f },
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_SCISSOR_TEST);

    for (int i = 0; i < 4; ++i) {
        glLoadIdentity();
        glTranslatef((i & 1) ? 0.85f : -0.85f, (i & 2) ? -0.62f : 0.62f, -4.2f);
        glRotatef(t * (35.0f + (float)i * 11.0f), 0.35f + (float)i * 0.15f, 1.0f, 0.25f);
        glScalef(0.52f, 0.52f, 0.52f);
        glColor3fv(colors[i]);
        draw_cube(true);
    }
    glScissor(0, 0, NEHE_SCREEN_W, NEHE_SCREEN_H);
    glViewport(0, 0, NEHE_SCREEN_W, NEHE_SCREEN_H);
}

static void lesson_25_target_gl(int shape, int index, int count, GLfloat out[3])
{
    float u = (float)index / (float)count;
    float a = u * (float)M_PI * 2.0f;
    float b = (float)((index * 37) % count) / (float)count * (float)M_PI * 2.0f;

    switch (shape & 3) {
    case 0:
        out[0] = cosf(a) * sinf(b) * 1.4f;
        out[1] = sinf(a) * sinf(b) * 1.4f;
        out[2] = cosf(b) * 1.4f;
        break;
    case 1: {
        float r = 1.0f + 0.35f * cosf(b * 3.0f);
        out[0] = cosf(a) * r;
        out[1] = sinf(b * 2.0f) * 0.55f;
        out[2] = sinf(a) * r;
        break;
    }
    case 2:
        out[0] = cosf(a) * 0.75f;
        out[1] = sinf(a * 2.0f) * 1.35f;
        out[2] = sinf(a) * 0.75f;
        break;
    default:
        out[0] = lesson_19_unit_gl(index, 21u) * 3.4f - 1.7f;
        out[1] = lesson_19_unit_gl(index, 22u) * 3.0f - 1.5f;
        out[2] = lesson_19_unit_gl(index, 23u) * 3.4f - 1.7f;
        break;
    }
}

static void render_25(Lesson *lesson, float t)
{
    const int count = 260;
    float cycle = t * 0.45f;
    int from = (int)floorf(cycle);
    float mix = cycle - floorf(cycle);
    float smooth = mix * mix * (3.0f - 2.0f * mix);

    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -7.5f);
    glRotatef(t * 25.8f, 0.2f, 1.0f, 0.1f);

    glBegin(GL_QUADS);
    for (int i = 0; i < count; ++i) {
        GLfloat a[3], b[3];
        float x, y, z;
        float s = 0.025f;

        lesson_25_target_gl(from, i, count, a);
        lesson_25_target_gl(from + 1, i, count, b);
        x = a[0] + (b[0] - a[0]) * smooth;
        y = a[1] + (b[1] - a[1]) * smooth;
        z = a[2] + (b[2] - a[2]) * smooth;
        glColor4f(0.35f + 0.65f * lesson_19_unit_gl(i, 30u),
                  0.35f + 0.65f * lesson_19_unit_gl(i, 31u),
                  0.65f + 0.35f * lesson_19_unit_gl(i, 32u),
                  0.78f);
        glVertex3f(x - s, y - s, z);
        glVertex3f(x + s, y - s, z);
        glVertex3f(x + s, y + s, z);
        glVertex3f(x - s, y + s, z);
    }
    glEnd();
}

static void draw_xz_plane(float y, float z0, float z1, float x0, float x1)
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y, z1);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y, z1);
    glEnd();
}

static void draw_wall_plane(float z, float x0, float x1, float y0, float y1)
{
    glBegin(GL_QUADS);
    glVertex3f(x0, y0, z);
    glVertex3f(x1, y0, z);
    glVertex3f(x1, y1, z);
    glVertex3f(x0, y1, z);
    glEnd();
}

static void render_26(Lesson *lesson, float t)
{
    float angle = t * 38.0f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glTranslatef(0.0f, 0.35f, -6.2f);
    glRotatef(-11.0f, 1.0f, 0.0f, 0.0f);

    glPushMatrix();
    glTranslatef(0.0f, -1.55f, 0.0f);
    glRotatef(-angle, 0.4f, 1.0f, 0.0f);
    glScalef(0.72f, 0.72f, 0.72f);
    glColor4f(0.35f, 0.55f, 0.75f, 0.38f);
    draw_cube(true);
    glPopMatrix();

    glColor4f(0.45f, 0.62f, 0.76f, 0.58f);
    draw_xz_plane(-0.85f, -1.9f, 1.9f, -2.4f, 2.4f);

    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.0f);
    glRotatef(angle, 0.4f, 1.0f, 0.0f);
    glScalef(0.72f, 0.72f, 0.72f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    draw_cube(true);
    glPopMatrix();
}

static void render_27(Lesson *lesson, float t)
{
    float angle = t * 44.0f;

    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glTranslatef(0.0f, 0.2f, -6.0f);
    glRotatef(-10.0f, 1.0f, 0.0f, 0.0f);

    glColor4f(0.42f, 0.42f, 0.46f, 1.0f);
    draw_xz_plane(-1.15f, -2.0f, 2.0f, -2.6f, 2.6f);
    glColor4f(0.24f, 0.28f, 0.36f, 1.0f);
    draw_wall_plane(2.0f, -2.6f, 2.6f, -1.15f, 1.95f);

    glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
    glBegin(GL_QUADS);
    glVertex3f(-1.1f, -1.13f, -0.65f);
    glVertex3f(1.65f, -1.13f, -0.25f);
    glVertex3f(1.05f, -1.13f, 0.35f);
    glVertex3f(-1.65f, -1.13f, -0.05f);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glPushMatrix();
    glTranslatef(-0.72f, 0.05f, 0.0f);
    glScalef(0.55f, 0.70f, 0.55f);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    draw_pyramid();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.82f, -0.02f, 0.0f);
    glRotatef(angle, 0.3f, 1.0f, 0.2f);
    glScalef(0.42f, 0.42f, 0.42f);
    draw_cube(false);
    glPopMatrix();
}

static float bezier1_gl(float p0, float p1, float p2, float p3, float t)
{
    float it = 1.0f - t;
    return p0 * it * it * it + 3.0f * p1 * t * it * it + 3.0f * p2 * t * t * it + p3 * t * t * t;
}

static void lesson_28_patch_point_gl(float u, float v, float phase, GLfloat out[3])
{
    float x = bezier1_gl(-1.8f, -0.8f, 0.8f, 1.8f, u);
    float z = bezier1_gl(-1.5f, -0.55f, 0.55f, 1.5f, v);
    float arch_u = sinf(u * (float)M_PI);
    float arch_v = sinf(v * (float)M_PI);
    out[0] = x;
    out[1] = (arch_u * arch_v - 0.45f) * 1.1f + sinf(phase + x * 1.6f + z) * 0.18f;
    out[2] = z;
}

static void render_28(Lesson *lesson, float t)
{
    const int divs = 14;
    float phase = t * 1.4f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.15f, -5.6f);
    glRotatef(-13.0f, 1.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    for (int y = 0; y < divs; ++y) {
        for (int x = 0; x < divs; ++x) {
            float u0 = (float)x / (float)divs;
            float u1 = (float)(x + 1) / (float)divs;
            float v0 = (float)y / (float)divs;
            float v1 = (float)(y + 1) / (float)divs;
            GLfloat p[4][3];
            lesson_28_patch_point_gl(u0, v0, phase, p[0]);
            lesson_28_patch_point_gl(u1, v0, phase, p[1]);
            lesson_28_patch_point_gl(u1, v1, phase, p[2]);
            lesson_28_patch_point_gl(u0, v1, phase, p[3]);
            glColor4f(0.55f + 0.45f * v0, 0.65f + 0.25f * u0, 1.0f, 1.0f);
            glTexCoord2f(u0, v0); glVertex3fv(p[0]);
            glTexCoord2f(u1, v0); glVertex3fv(p[1]);
            glTexCoord2f(u1, v1); glVertex3fv(p[2]);
            glTexCoord2f(u0, v1); glVertex3fv(p[3]);
        }
    }
    glEnd();
}

static void render_29(Lesson *lesson, float t)
{
    (void)t;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.2f);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    draw_textured_quad_xy(-1.25f, 0.0f, 0.0f, 1.04f, 1.04f, 0.0f);
    glColor4f(0.75f, 1.0f, 0.85f, 0.92f);
    draw_textured_quad_xy(0.0f, 0.0f, 0.0f, 1.44f, 0.84f, 0.0f);
    glColor4f(1.0f, 0.75f, 0.95f, 0.92f);
    draw_textured_quad_xy(1.25f, 0.0f, 0.0f, 0.84f, 1.44f, 0.0f);
}

static float lesson_30_distance_gl(float ax, float ay, float az, float bx, float by, float bz)
{
    float dx = ax - bx;
    float dy = ay - by;
    float dz = az - bz;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

static void render_30(Lesson *lesson, float t)
{
    float sx = sinf(t * 0.85f) * 1.25f;
    float sy = cosf(t * 1.15f) * 0.52f;
    float sz = sinf(t * 0.53f) * 0.75f;
    bool hit = lesson_30_distance_gl(sx, sy, sz, 0.0f, 0.0f, 0.0f) < 0.98f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -6.2f);
    glRotatef(-7.0f, 1.0f, 0.0f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glPushMatrix();
    glRotatef(t * 37.0f, 0.3f, 1.0f, 0.2f);
    glScalef(0.92f, 0.92f, 0.92f);
    if (hit) {
        glColor4f(1.0f, 0.52f, 0.50f, 1.0f);
    } else {
        glColor4f(0.88f, 0.94f, 1.0f, 1.0f);
    }
    draw_cube(true);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(hit ? 1.0f : 0.1f, hit ? 0.1f : 0.7f, hit ? 0.06f : 1.0f, hit ? 0.72f : 0.55f);
    draw_textured_quad_xy(sx, sy, sz, 1.1f, 1.1f, 0.0f);
    glColor4f(hit ? 1.0f : 0.18f, hit ? 0.18f : 0.55f, hit ? 0.12f : 1.0f, hit ? 0.85f : 0.65f);
    glBegin(GL_QUADS);
    for (int i = 0; i < 12; ++i) {
        float a = (float)i / 12.0f * (float)M_PI * 2.0f;
        float x = cosf(a) * 0.98f;
        float y = sinf(a) * 0.98f;
        float s = 0.035f;
        glVertex3f(x - s, y - s, 0.02f);
        glVertex3f(x + s, y - s, 0.02f);
        glVertex3f(x + s, y + s, 0.02f);
        glVertex3f(x - s, y + s, 0.02f);
    }
    glEnd();
}

static void push_triangle_gl(float ax, float ay, float az, float bx, float by, float bz,
                             float cx, float cy, float cz, float r, float g, float b)
{
    glColor4f(r, g, b, 1.0f);
    glVertex3f(ax, ay, az);
    glVertex3f(bx, by, bz);
    glVertex3f(cx, cy, cz);
}

static void render_31(Lesson *lesson, float t)
{
    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.4f);
    glRotatef(-5.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(t * 41.0f, 0.0f, 1.0f, 0.0f);

    glBegin(GL_TRIANGLES);
    push_triangle_gl(0.0f, 0.58f, -1.25f, -0.45f, -0.34f, 0.55f, 0.45f, -0.34f, 0.55f, 0.58f, 0.72f, 0.88f);
    push_triangle_gl(0.0f, -0.18f, -1.05f, -1.55f, -0.44f, 0.35f, -0.45f, -0.34f, 0.55f, 0.28f, 0.42f, 0.68f);
    push_triangle_gl(0.0f, -0.18f, -1.05f, 0.45f, -0.34f, 0.55f, 1.55f, -0.44f, 0.35f, 0.28f, 0.42f, 0.68f);
    push_triangle_gl(-0.45f, -0.34f, 0.55f, 0.0f, -0.04f, 1.05f, 0.45f, -0.34f, 0.55f, 0.42f, 0.55f, 0.78f);
    push_triangle_gl(-0.22f, -0.26f, 0.88f, 0.22f, -0.26f, 0.88f, 0.0f, -0.26f, 1.55f + 0.18f * sinf(t * 9.0f),
                     1.0f, 0.42f + 0.25f * sinf(t * 8.0f), 0.06f);
    glEnd();
}

static void render_32(Lesson *lesson, float t)
{
    float cursor = sinf(t * 0.8f);
    const GLfloat pane[4][4] = {
        { 1.0f, 0.25f, 0.15f, 0.48f },
        { 0.25f, 1.0f, 0.35f, 0.48f },
        { 0.15f, 0.45f, 1.0f, 0.48f },
        { 1.0f, 0.85f, 0.15f, 0.48f }
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.0f);
    for (int i = 3; i >= 0; --i) {
        float x = -1.05f + (float)i * 0.7f;
        float y = sinf(t * 0.55f + (float)i) * 0.28f;
        bool selected = fabsf(cursor - ((float)i - 1.5f) * 0.45f) < 0.28f;
        if (selected) {
            glColor4f(1.0f, 1.0f, 1.0f, 0.72f);
        } else {
            glColor4fv(pane[i]);
        }
        draw_textured_quad_xy(x, y, (float)i * -0.12f, 0.92f, 1.44f, t * (0.25f + (float)i * 0.08f));
    }
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 0.95f);
    draw_textured_quad_xy(cursor * 1.35f, -0.98f, 0.02f, 0.20f, 0.20f, 0.0f);
}

static void render_33(Lesson *lesson, float t)
{
    (void)t;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.6f);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    draw_textured_quad_xy(-0.9f, 0.68f, 0.0f, 1.04f, 0.76f, 0.0f);
    glColor4f(1.0f, 0.82f, 0.82f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[1]);
    draw_textured_quad_xy(0.9f, 0.68f, 0.0f, 1.04f, 0.76f, 0.0f);
    glColor4f(0.82f, 1.0f, 0.90f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[1]);
    draw_textured_quad_xy(-0.9f, -0.52f, 0.0f, 1.04f, 0.76f, 0.0f);
    glColor4f(0.82f, 0.90f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[3]);
    draw_textured_quad_xy(0.9f, -0.52f, 0.0f, 1.04f, 0.76f, 0.0f);
}

static float lesson_34_height_gl(float x, float z, float t)
{
    return sinf(x * 2.3f + t * 0.35f) * 0.22f + cosf(z * 2.1f - t * 0.25f) * 0.18f;
}

static void render_34(Lesson *lesson, float t)
{
    const int cells = 18;
    const float span = 4.2f;
    const float step = span / (float)cells;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.05f, -5.6f);
    glRotatef(-27.5f, 1.0f, 0.0f, 0.0f);
    glRotatef(t * 6.8f, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    for (int z = 0; z < cells; ++z) {
        for (int x = 0; x < cells; ++x) {
            float x0 = -span * 0.5f + (float)x * step;
            float x1 = x0 + step;
            float z0 = -span * 0.5f + (float)z * step;
            float z1 = z0 + step;
            float h0 = lesson_34_height_gl(x0, z0, t);
            float h1 = lesson_34_height_gl(x1, z0, t);
            float h2 = lesson_34_height_gl(x1, z1, t);
            float h3 = lesson_34_height_gl(x0, z1, t);
            float shade = 0.55f + (h0 + h1 + h2 + h3) * 0.28f;
            glColor4f(shade * 0.82f, shade, shade * 0.72f, 1.0f);
            glTexCoord2f((float)x / 3.0f, (float)z / 3.0f); glVertex3f(x0, h0 - 0.65f, z0);
            glTexCoord2f((float)(x + 1) / 3.0f, (float)z / 3.0f); glVertex3f(x1, h1 - 0.65f, z0);
            glTexCoord2f((float)(x + 1) / 3.0f, (float)(z + 1) / 3.0f); glVertex3f(x1, h2 - 0.65f, z1);
            glTexCoord2f((float)x / 3.0f, (float)(z + 1) / 3.0f); glVertex3f(x0, h3 - 0.65f, z1);
        }
    }
    glEnd();
}

static void render_35(Lesson *lesson, float t)
{
    float progress = fmodf(t * 0.42f, 2.84f);

    fill_video_pixels(t);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, video_pixels);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    draw_textured_quad_xy(0.0f, 0.10f, 0.0f, 2.7f, 2.0f, sinf(t * 0.7f) * 4.6f);

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor4f(0.12f, 0.18f, 0.22f, 1.0f);
    glVertex3f(-1.55f, -1.05f, 0.03f);
    glVertex3f(1.55f, -1.05f, 0.03f);
    glVertex3f(1.55f, -0.92f, 0.03f);
    glVertex3f(-1.55f, -0.92f, 0.03f);
    glColor4f(0.15f, 0.75f, 1.0f, 1.0f);
    glVertex3f(-1.42f, -1.02f, 0.04f);
    glVertex3f(-1.42f + progress, -1.02f, 0.04f);
    glVertex3f(-1.42f + progress, -0.95f, 0.04f);
    glVertex3f(-1.42f, -0.95f, 0.04f);
    glEnd();
}

static void render_36(Lesson *lesson, float t)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glPushMatrix();
    glRotatef(t * 45.8f, 0.2f, 1.0f, 0.4f);
    glScalef(0.92f, 0.92f, 0.92f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    draw_cube(true);
    glPopMatrix();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[1]);
    for (int i = 0; i < 12; ++i) {
        float s = 1.24f + (float)i * 0.26f;
        float a = t * 25.0f + (float)i * 10.0f;
        glColor4f(0.2f, 0.6f, 1.0f, 0.13f * (1.0f - (float)i / 13.0f));
        draw_textured_quad_xy(cosf(a * NEHE_DEG_TO_RAD) * 0.05f, sinf(a * NEHE_DEG_TO_RAD) * 0.04f,
                              0.05f + (float)i * 0.002f, s, s, a);
    }
    glDepthMask(GL_TRUE);
}

static void render_37(Lesson *lesson, float t)
{
    static const GLfloat black_faces[6][3] = {
        { 0.01f, 0.01f, 0.01f },
        { 0.01f, 0.01f, 0.01f },
        { 0.01f, 0.01f, 0.01f },
        { 0.01f, 0.01f, 0.01f },
        { 0.01f, 0.01f, 0.01f },
        { 0.01f, 0.01f, 0.01f }
    };
    static const GLfloat cel_faces[6][3] = {
        { 0.10f, 0.80f, 0.18f },
        { 0.06f, 0.38f, 0.10f },
        { 0.88f, 0.15f, 0.10f },
        { 0.54f, 0.08f, 0.06f },
        { 0.12f, 0.25f, 0.90f },
        { 0.05f, 0.09f, 0.42f }
    };

    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.2f);
    glRotatef(-6.0f, 1.0f, 0.0f, 0.0f);

    glPushMatrix();
    glRotatef(t * 43.0f, 0.3f, 1.0f, 0.2f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glScalef(0.56f, 0.56f, 0.56f);
    draw_cube_colored(black_faces);
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glScalef(0.875f, 0.875f, 0.875f);
    draw_cube_colored(cel_faces);
    glPopMatrix();
}

static void render_38(Lesson *lesson, float t)
{
    float spin = t * 10.0f;
    const GLfloat colors[4][4] = {
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.82f, 0.82f, 1.0f },
        { 0.82f, 1.0f, 0.90f, 1.0f },
        { 0.82f, 0.90f, 1.0f, 1.0f }
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.7f);
    for (int i = 0; i < 4; ++i) {
        float x = (i & 1) ? 0.82f : -0.82f;
        float y = (i & 2) ? -0.58f : 0.58f;
        glBindTexture(GL_TEXTURE_2D, lesson->textures[i]);
        glColor4fv(colors[i]);
        draw_textured_quad_xy(x, y, 0.0f, 0.96f, 0.72f, spin * (float)(i + 1));
    }
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor4f(0.24f, 0.28f, 0.34f, 1.0f);
    glVertex3f(-1.75f, 0.0f, 0.04f);
    glVertex3f(1.75f, 0.0f, 0.04f);
    glVertex3f(1.75f, 0.05f, 0.04f);
    glVertex3f(-1.75f, 0.05f, 0.04f);
    glEnd();
}

static float lesson_39_height_gl(int x, int y, float t)
{
    return sinf((float)x * 0.8f + t * 1.6f) * 0.18f + cosf((float)y * 0.7f - t * 1.1f) * 0.14f;
}

static void render_39(Lesson *lesson, float t)
{
    const int cols = 9;
    const int rows = 7;
    const float step = 0.34f;

    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.4f);
    glRotatef(-10.5f, 1.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            float x0 = ((float)x - (float)(cols - 1) * 0.5f) * step;
            float y0 = ((float)y - (float)(rows - 1) * 0.5f) * step;
            float x1 = x0 + step;
            float y1 = y0 + step;
            glColor4f(0.18f + (float)x * 0.035f, 0.38f + (float)y * 0.045f, 0.90f, 0.72f);
            glVertex3f(x0, y0 + lesson_39_height_gl(x, y, t), 0.0f);
            glVertex3f(x1, y0 + lesson_39_height_gl(x + 1, y, t), 0.0f);
            glVertex3f(x1, y1 + lesson_39_height_gl(x + 1, y + 1, t), 0.0f);
            glVertex3f(x0, y1 + lesson_39_height_gl(x, y + 1, t), 0.0f);
        }
    }
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            float px = ((float)x - (float)(cols - 1) * 0.5f) * step;
            float py = ((float)y - (float)(rows - 1) * 0.5f) * step + lesson_39_height_gl(x, y, t);
            float s = 0.035f;
            glVertex3f(px - s, py - s, 0.04f);
            glVertex3f(px + s, py - s, 0.04f);
            glVertex3f(px + s, py + s, 0.04f);
            glVertex3f(px - s, py + s, 0.04f);
        }
    }
    glEnd();
}

static void push_rope_segment_gl(float ax, float ay, float bx, float by, float width)
{
    float dx = bx - ax;
    float dy = by - ay;
    float len = sqrtf(dx * dx + dy * dy);
    float nx;
    float ny;

    if (len < 0.0001f) {
        return;
    }
    nx = -dy / len * width;
    ny = dx / len * width;
    glVertex3f(ax + nx, ay + ny, 0.0f);
    glVertex3f(bx + nx, by + ny, 0.0f);
    glVertex3f(bx - nx, by - ny, 0.0f);
    glVertex3f(ax - nx, ay - ny, 0.0f);
}

static void render_40(Lesson *lesson, float t)
{
    const int points = 18;
    float prev_x = -1.55f;
    float prev_y = 0.72f;

    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.2f);

    glBegin(GL_QUADS);
    glColor4f(1.0f, 0.95f, 0.45f, 1.0f);
    glVertex3f(prev_x - 0.08f, prev_y - 0.08f, 0.02f);
    glVertex3f(prev_x + 0.08f, prev_y - 0.08f, 0.02f);
    glVertex3f(prev_x + 0.08f, prev_y + 0.08f, 0.02f);
    glVertex3f(prev_x - 0.08f, prev_y + 0.08f, 0.02f);
    for (int i = 1; i < points; ++i) {
        float u = (float)i / (float)(points - 1);
        float x = -1.55f + u * 3.1f;
        float y = 0.72f - sinf(u * (float)M_PI) * (0.95f + 0.10f * sinf(t * 1.4f)) + sinf(t * 2.0f + u * 8.0f) * 0.08f;
        glColor4f(0.90f, 0.72f, 0.36f, 1.0f);
        push_rope_segment_gl(prev_x, prev_y, x, y, 0.025f);
        glColor4f(1.0f, 0.86f, 0.45f, 1.0f);
        glVertex3f(x - 0.045f, y - 0.045f, 0.02f);
        glVertex3f(x + 0.045f, y - 0.045f, 0.02f);
        glVertex3f(x + 0.045f, y + 0.045f, 0.02f);
        glVertex3f(x - 0.045f, y + 0.045f, 0.02f);
        prev_x = x;
        prev_y = y;
    }
    glEnd();
}

static void render_41(Lesson *lesson, float t)
{
    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[1]);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    for (int i = 0; i < 22; ++i) {
        float z = -1.55f + (float)i * 0.14f;
        float sway = sinf(t * 0.9f + (float)i * 0.37f) * 0.18f;
        float size = 1.1f + (float)i * 0.07f;
        glColor4f(0.36f, 0.62f, 1.0f, 0.10f + 0.10f * (1.0f - (float)i / 22.0f));
        draw_textured_quad_xy(sway, 0.05f + sinf(t * 1.3f + (float)i) * 0.08f, z, size, size * 0.72f,
                              t * 0.18f + (float)i * 0.11f);
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_CULL_FACE);
    glPushMatrix();
    glTranslatef(0.0f, -0.12f, -0.35f);
    glRotatef(t * 37.0f, 0.4f, 1.0f, 0.2f);
    glScalef(0.74f, 0.74f, 0.74f);
    glColor4f(0.85f, 0.94f, 1.0f, 1.0f);
    draw_cube(true);
    glPopMatrix();
}

static void render_42(Lesson *lesson, float t)
{
    static const GLint panes[4][4] = {
        { 0, 0, NEHE_SCREEN_W / 2, NEHE_SCREEN_H / 2 },
        { NEHE_SCREEN_W / 2, 0, NEHE_SCREEN_W / 2, NEHE_SCREEN_H / 2 },
        { 0, NEHE_SCREEN_H / 2, NEHE_SCREEN_W / 2, NEHE_SCREEN_H / 2 },
        { NEHE_SCREEN_W / 2, NEHE_SCREEN_H / 2, NEHE_SCREEN_W / 2, NEHE_SCREEN_H / 2 },
    };
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);
    for (int i = 0; i < 4; ++i) {
        glViewport(panes[i][0], panes[i][1], panes[i][2], panes[i][3]);
        glScissor(panes[i][0], panes[i][1], panes[i][2], panes[i][3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z);
        glPushMatrix();
        glRotatef(t * (28.0f + (float)i * 8.0f), 0.25f + (float)i * 0.1f, 1.0f, 0.35f);
        glColor4f(0.78f + 0.05f * (float)i, 0.88f, 1.0f - 0.08f * (float)i, 1.0f);
        glScalef(0.48f, 0.48f, 0.48f);
        draw_cube(true);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.12f, 0.10f, 1.0f); glVertex3f(0.0f, -0.58f, 0.02f);
        glColor4f(0.12f, 1.0f, 0.28f, 1.0f); glVertex3f(-0.25f, -0.98f, 0.02f);
        glColor4f(0.16f, 0.38f, 1.0f, 1.0f); glVertex3f(0.25f, -0.98f, 0.02f);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }
    glViewport(0, 0, NEHE_SCREEN_W, NEHE_SCREEN_H);
    glScissor(0, 0, NEHE_SCREEN_W, NEHE_SCREEN_H);
    glDisable(GL_SCISSOR_TEST);
}

static void render_43(Lesson *lesson, float t)
{
    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.0f);
    for (int i = 0; i < 7; ++i) {
        float x = -1.35f + (float)i * 0.45f;
        float y = sinf(t * 1.2f + (float)i * 0.7f) * 0.18f;
        glColor4f(0.45f + 0.07f * (float)i, 0.78f, 1.0f, 1.0f);
        draw_textured_quad_xy(x, y + 0.20f, -0.05f * (float)i, 0.34f, 0.44f, sinf(t + (float)i) * 0.08f);
        glColor4f(0.15f, 0.22f, 0.32f, 1.0f);
        draw_textured_quad_xy(x + 0.04f, y + 0.14f, -0.06f - 0.05f * (float)i, 0.34f, 0.44f, sinf(t + (float)i) * 0.08f);
    }
    glDisable(GL_TEXTURE_2D);
    draw_lesson_21_line(-1.55f, -0.42f, 1.55f, -0.42f, 0.018f);
}

static void render_44(Lesson *lesson, float t)
{
    (void)lesson;
    float sun_x = sinf(t * 0.45f) * 0.75f;
    float sun_y = 0.52f + cosf(t * 0.31f) * 0.18f;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.8f);
    for (int i = 0; i < 9; ++i) {
        float u = (float)i / 8.0f;
        float x = sun_x * (1.0f - u) - sun_x * 0.75f * u;
        float y = sun_y * (1.0f - u) - sun_y * 0.55f * u;
        float size = 0.20f + 0.68f * (1.0f - fabsf(u - 0.5f));
        glColor4f(1.0f, 0.62f + 0.35f * u, 0.22f + 0.70f * (1.0f - u), 0.25f);
        draw_textured_quad_xy(x, y, (float)i * 0.002f, size, size, t * 0.16f + (float)i);
    }
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glColor4f(0.02f, 0.025f, 0.035f, 0.78f);
    glVertex3f(-0.18f, -0.45f, 0.04f);
    glVertex3f(0.30f, -0.44f, 0.04f);
    glVertex3f(0.22f, 0.46f, 0.04f);
    glVertex3f(-0.32f, 0.34f, 0.04f);
    glEnd();
    glDepthMask(GL_TRUE);
}

static void render_45(Lesson *lesson, float t)
{
    (void)lesson;
    const int cols = 18;
    const int rows = 12;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.2f);
    glBegin(GL_QUADS);
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
            glColor4f(0.18f + (float)x * 0.025f, 0.36f + (float)y * 0.035f, 0.92f, 1.0f);
            glVertex3f(x0, y0, h0);
            glVertex3f(x1, y0, h1);
            glVertex3f(x1, y1, h2);
            glVertex3f(x0, y1, h3);
        }
    }
    glEnd();
}

static void render_46(Lesson *lesson, float t)
{
    (void)lesson;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    for (int i = 0; i < 16; ++i) {
        float y = -1.15f + (float)i * 0.15f;
        glColor4f(0.22f + (float)i * 0.035f, 0.72f, 1.0f, 0.18f + (float)i * 0.035f);
        draw_lesson_21_line(-1.55f, y, 1.55f, y + 0.12f, 0.005f + (float)i * 0.0015f);
    }
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glRotatef(t * 32.0f, 0.25f, 1.0f, 0.15f);
    glScalef(0.41f, 0.41f, 0.41f);
    draw_cube(false);
}

static void render_47(Lesson *lesson, float t)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.0f);
    for (int i = 0; i < 10; ++i) {
        float x = -1.35f + (float)i * 0.30f;
        float wave = sinf(t * 1.6f + (float)i * 0.55f) * 0.24f;
        glColor4f(0.45f + 0.04f * (float)i, 0.75f, 1.0f, 0.92f);
        draw_textured_quad_xy(x, wave, -0.02f * (float)i, 0.26f, 1.10f, t * 0.45f + (float)i * 0.22f);
    }
}

static void render_48(Lesson *lesson, float t)
{
    (void)lesson;
    float ax = sinf(t * 0.37f) * 0.6f + 0.35f;
    float ay = cosf(t * 0.29f) * 0.6f + 0.65f;
    float az = sinf(t * 0.23f) * 0.45f + 0.25f;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lesson->textures[0]);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, NEHE_DEFAULT_VIEW_Z);
    glPushMatrix();
    glRotatef(t * 43.0f, ax, ay, az);
    glScalef(0.92f, 0.92f, 0.92f);
    glColor4f(0.95f, 0.98f, 1.0f, 1.0f);
    draw_cube(true);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.72f, 0.82f, 1.0f, 0.42f);
    for (int i = 0; i < 24; ++i) {
        float a0 = (float)i * (float)M_PI / 12.0f;
        float a1 = (float)(i + 1) * (float)M_PI / 12.0f;
        draw_lesson_21_line(cosf(a0) * 1.35f, sinf(a0) * 1.35f,
                            cosf(a1) * 1.35f, sinf(a1) * 1.35f, 0.008f);
    }
}

static Lesson lessons[] = {
    {"NeHe 01 - OpenGL Window", "Clear/context setup", NULL, render_01, shutdown_default, {0}, {0}},
    {"NeHe 02 - First Polygons", "Triangle and quad", NULL, render_02, shutdown_default, {0}, {0}},
    {"NeHe 03 - Adding Color", "Vertex colors", NULL, render_03, shutdown_default, {0}, {0}},
    {"NeHe 04 - Rotation", "Spinning flat shapes", NULL, render_04, shutdown_default, {0}, {0}},
    {"NeHe 05 - 3D Shapes", "Pyramid and cube", NULL, render_05, shutdown_default, {0}, {0}},
    {"NeHe 06 - Texture Mapping", "glTexImage2D cube", init_texture_checker, render_06, shutdown_default, {0}, {0}},
    {"NeHe 07 - Filters/Lighting", "Texture filter and light", init_texture_crate_filters, render_07, shutdown_default, {0}, {0}},
    {"NeHe 08 - Blending", "Glass texture alpha cube", init_texture_glass, render_08, shutdown_default, {0}, {0}},
    {"NeHe 09 - Moving Bitmaps", "Blended star field", init_star_texture, render_09, shutdown_default, {0}, {0}},
    {"NeHe 10 - 3D World", "Textured hallway", init_world_texture, render_10, shutdown_default, {0}, {0}},
    {"NeHe 11 - Flag Effect", "Textured sine mesh", init_flag_texture, render_11, shutdown_default, {0}, {0}},
    {"NeHe 12 - Display Lists", "Textured cube list stack", init_display_lists, render_12, shutdown_default, {0}, {0}},
    {"NeHe 13 - Bitmap Fonts", "Animated bitmap text", NULL, render_13, shutdown_default, {0}, {0}},
    {"NeHe 14 - Outline Fonts", "Spinning outline text", NULL, render_14, shutdown_default, {0}, {0}},
    {"NeHe 15 - Texture Outline Fonts", "Textured outline symbol", init_lights_texture, render_15, shutdown_default, {0}, {0}},
    {"NeHe 16 - Cool Looking Fog", "Fog over textured crates", init_texture_crate_linear, render_16, shutdown_default, {0}, {0}},
    {"NeHe 17 - 2D Texture Font", "Texture atlas font overlay", init_texture_font, render_17, shutdown_texture_font, {0}, {0}},
    {"NeHe 18 - Quadrics", "Cube and GLU-style quadric shapes", init_texture_crate_linear, render_18, shutdown_default, {0}, {0}},
    {"NeHe 19 - Particle Engine", "Additive blended particle system", init_particle_texture, render_19, shutdown_default, {0}, {0}},
    {"NeHe 20 - Masking", "Two-pass texture masking", init_mask_texture, render_20, shutdown_default, {0}, {0}},
    {"NeHe 21 - Lines/Timing/Ortho", "Timed antialiased line patterns", NULL, render_21, shutdown_default, {0}, {0}},
    {"NeHe 22 - Bump Mapping", "Embossed multitexture-style cube", init_bump_texture, render_22, shutdown_default, {0}, {0}},
    {"NeHe 23 - Sphere Mapping", "Environment mapped sphere", init_sphere_texture, render_23, shutdown_default, {0}, {0}},
    {"NeHe 24 - Tokens/Scissor/TGA", "Scissor panes and TGA-style texture", init_scissor_texture, render_24, shutdown_default, {0}, {0}},
    {"NeHe 25 - Morphing/Loading Objects", "Point-cloud morphing between objects", NULL, render_25, shutdown_default, {0}, {0}},
    {"NeHe 26 - Stencil Reflections", "Reflected cube over clipped floor", init_scissor_texture, render_26, shutdown_default, {0}, {0}},
    {"NeHe 27 - Shadows", "Planar shadow projection", NULL, render_27, shutdown_default, {0}, {0}},
    {"NeHe 28 - Bezier Patches", "Textured Bezier-style patch", init_flag_texture, render_28, shutdown_default, {0}, {0}},
    {"NeHe 29 - Blitter/Raw Textures", "Procedural raw texture blits", init_raw_texture, render_29, shutdown_default, {0}, {0}},
    {"NeHe 30 - Collision Detection", "Moving sphere/cube collision volume", init_texture_crate_linear, render_30, shutdown_default, {0}, {0}},
    {"NeHe 31 - Model Loading", "Loaded-style low-poly model mesh", NULL, render_31, shutdown_default, {0}, {0}},
    {"NeHe 32 - Picking/Sorting", "Sorted transparent selection panes", init_particle_texture, render_32, shutdown_default, {0}, {0}},
    {"NeHe 33 - TGA Variants", "Texture loader format comparison", init_tga_variant_textures, render_33, shutdown_default, {0}, {0}},
    {"NeHe 34 - Height-map Terrain", "Procedural landscape height field", init_world_texture, render_34, shutdown_default, {0}, {0}},
    {"NeHe 35 - AVI Texture Playback", "Procedural video texture stream", init_video_texture, render_35, shutdown_default, {0}, {0}},
    {"NeHe 36 - Radial Blur", "Layered render-to-texture style blur", init_mask_texture, render_36, shutdown_default, {0}, {0}},
    {"NeHe 37 - Cel Shading", "Quantized lighting and outlines", NULL, render_37, shutdown_default, {0}, {0}},
    {"NeHe 38 - Resource Textures", "Packed resource texture atlas", init_tga_variant_textures, render_38, shutdown_default, {0}, {0}},
    {"NeHe 39 - Physics Simulation", "Spring-mass field simulation", NULL, render_39, shutdown_default, {0}, {0}},
    {"NeHe 40 - Rope Physics", "Segmented rope simulation", NULL, render_40, shutdown_default, {0}, {0}},
    {"NeHe 41 - Volumetric Fog", "Layered translucent fog volume", init_crate_particle_texture, render_41, shutdown_default, {0}, {0}},
    {"NeHe 42 - Multiple Viewports", "Four isolated viewport scenes", init_texture_crate_linear, render_42, shutdown_default, {0}, {0}},
    {"NeHe 43 - FreeType Fonts", "Smooth glyph extrusion stand-in", init_texture_font, render_43, shutdown_default, {0}, {0}},
    {"NeHe 44 - Lens Flare", "Billboard flare and occluder pass", init_particle_texture, render_44, shutdown_default, {0}, {0}},
    {"NeHe 45 - Vertex Buffers", "Streaming vertex field batches", NULL, render_45, shutdown_default, {0}, {0}},
    {"NeHe 46 - Antialiasing", "Subpixel line and edge smoothing", NULL, render_46, shutdown_default, {0}, {0}},
    {"NeHe 47 - Vertex Shader", "Animated shader-style deformation", init_raw_texture, render_47, shutdown_default, {0}, {0}},
    {"NeHe 48 - ArcBall Rotation", "Trackball-controlled object rotation", init_texture_crate_linear, render_48, shutdown_default, {0}, {0}},
};

static int lesson_count(void)
{
    return (int)(sizeof(lessons) / sizeof(lessons[0]));
}

static void enter_lesson(int index)
{
    scene_reset_gl();
    memset(lessons[index].textures, 0, sizeof(lessons[index].textures));
    memset(lessons[index].lists, 0, sizeof(lessons[index].lists));
    if (lessons[index].init != NULL) {
        lessons[index].init(&lessons[index]);
    }
    lesson_start = GetTickCount();
}

static void leave_lesson(int index)
{
    scene_reset_gl();
    if (lessons[index].shutdown != NULL) {
        lessons[index].shutdown(&lessons[index]);
    }
    flush_reset_viewport_boundary();
}

#ifndef NEHE_STANDALONE_LESSON_INDEX
static void switch_lesson(int delta)
{
    int next = current_lesson + delta;
    while (next < 0) {
        next += lesson_count();
    }
    while (next >= lesson_count()) {
        next -= lesson_count();
    }
    if (next == current_lesson) {
        return;
    }
    leave_lesson(current_lesson);
    current_lesson = next;
    enter_lesson(current_lesson);
}

int main(void)
{
    bool prev_was_down = false;
    bool next_was_down = false;

    if (nxglInit() != 0) {
        Sleep(3000);
        return 1;
    }
    nxglSetReadbackEnabled(GL_FALSE);
    make_textures();
    input_init();
    enter_lesson(current_lesson);

    while (1) {
        char title[96];
        char detail[128];
        float t = nehe_frame_seconds(lesson_start);
        bool prev_down;
        bool next_down;

        input_poll();
        prev_down = previous_pressed();
        next_down = next_pressed();
        if (prev_down && !prev_was_down) {
            switch_lesson(-1);
            t = 0.0f;
        }
        if (next_down && !next_was_down) {
            switch_lesson(1);
            t = 0.0f;
        }
        prev_was_down = prev_down;
        next_was_down = next_down;

        frame_reset_gl();
        lessons[current_lesson].render(&lessons[current_lesson], t);
        snprintf(title, sizeof(title), "NXGL NeHe %02d/%02d  White/Black", current_lesson + 1, lesson_count());
        snprintf(detail, sizeof(detail), "%s | %s", lessons[current_lesson].title, lessons[current_lesson].detail);
        nxglSwapBuffers(title, detail);
    }

    return 0;
}
#else
int main(void)
{
    const int lesson = NEHE_STANDALONE_LESSON_INDEX;

    if (nxglInit() != 0) {
        Sleep(3000);
        return 1;
    }
    nxglSetReadbackEnabled(GL_FALSE);
    make_textures();

    scene_reset_gl();
    memset(lessons[lesson].textures, 0, sizeof(lessons[lesson].textures));
    memset(lessons[lesson].lists, 0, sizeof(lessons[lesson].lists));
    if (lessons[lesson].init != NULL) {
        lessons[lesson].init(&lessons[lesson]);
    }

    lesson_start = GetTickCount();
    while (1) {
        char title[96];
        char detail[128];
        float t = nehe_frame_seconds(lesson_start);

        frame_reset_gl();
        lessons[lesson].render(&lessons[lesson], t);
        snprintf(title, sizeof(title), "NXGL NeHe %02d standalone", lesson + 1);
        snprintf(detail, sizeof(detail), "%s | %s", lessons[lesson].title, lessons[lesson].detail);
        nxglSwapBuffers(title, detail);
    }

    return 0;
}
#endif
