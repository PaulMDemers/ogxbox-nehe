#ifndef NEHE_CAPTURE_TIME_H
#define NEHE_CAPTURE_TIME_H

#include <windows.h>

static inline DWORD nehe_frame_tick(DWORD start_tick)
{
#ifdef NEHE_CAPTURE_FIXED_TIME_MS
    return start_tick + (DWORD)NEHE_CAPTURE_FIXED_TIME_MS;
#else
    (void)start_tick;
    return GetTickCount();
#endif
}

static inline float nehe_frame_seconds(DWORD start_tick)
{
    DWORD delta = nehe_frame_tick(start_tick) - start_tick;
    return (float)delta / 1000.0f;
}

#endif
