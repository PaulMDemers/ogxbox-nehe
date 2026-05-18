#ifndef NEHE_CAPTURE_TIME_H
#define NEHE_CAPTURE_TIME_H

#include <windows.h>

static inline int nehe_frame_tick(int start_tick)
{
#ifdef NEHE_CAPTURE_FIXED_TIME_MS
    return start_tick + NEHE_CAPTURE_FIXED_TIME_MS;
#else
    (void)start_tick;
    return GetTickCount();
#endif
}

static inline float nehe_frame_seconds(int start_tick)
{
    return (float)(nehe_frame_tick(start_tick) - start_tick) / 1000.0f;
}

#endif
