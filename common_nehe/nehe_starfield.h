#ifndef NEHE_STARFIELD_H
#define NEHE_STARFIELD_H

#include <math.h>
#include <stdint.h>

#include "nehe_scene.h"

#define NEHE_STAR_COUNT 50
#define NEHE_STAR_SPREAD 5.0f
#define NEHE_STAR_Z -15.0f
#define NEHE_STAR_FRAME_RATE 60.0f

static inline uint8_t nehe_star_rand_channel(int ordinal)
{
    uint32_t seed = 1u;
    int value = 0;
    for (int i = 0; i <= ordinal; ++i) {
        seed = seed * 214013u + 2531011u;
        value = (int)((seed >> 16) & 0x7fffu);
    }
    return (uint8_t)(value & 0xff);
}

static inline void nehe_star_color(int index, float *r, float *g, float *b)
{
    *r = (float)nehe_star_rand_channel(index * 3 + 0) / 255.0f;
    *g = (float)nehe_star_rand_channel(index * 3 + 1) / 255.0f;
    *b = (float)nehe_star_rand_channel(index * 3 + 2) / 255.0f;
}

static inline void nehe_star_state(int index, float t, float *x, float *y, float *spin_degrees)
{
    float frames = t * NEHE_STAR_FRAME_RATE;
    float dist = ((float)index / (float)NEHE_STAR_COUNT) * NEHE_STAR_SPREAD;
    float angle_degrees;
    float angle_radians;

    dist = fmodf(dist - frames * 0.01f, NEHE_STAR_SPREAD);
    if (dist < 0.0f) {
        dist += NEHE_STAR_SPREAD;
    }

    angle_degrees = frames * ((float)index / (float)NEHE_STAR_COUNT);
    angle_radians = angle_degrees * NEHE_DEG_TO_RAD;
    *x = cosf(angle_radians) * dist;
    *y = sinf(angle_radians) * dist;
    *spin_degrees = frames * ((float)NEHE_STAR_COUNT * 0.01f) + (float)index * 0.01f;
}

#endif
