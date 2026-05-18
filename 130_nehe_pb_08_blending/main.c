#include "nehe_lessons.h"
#include "nehe_native.h"
#include "nehe_capture_time.h"

#include <stdio.h>
#include <windows.h>

#define STANDALONE_LESSON_INDEX 7

int main(void)
{
    int start;

    if (n3_init() != 0) {
        Sleep(3000);
        return 1;
    }
    if (nehe_lessons_init() != 0) {
        Sleep(3000);
        return 1;
    }

    start = GetTickCount();
    while (1) {
        char title[96];
        float t = nehe_frame_seconds(start);

        snprintf(title, sizeof(title), "pb NeHe %02d standalone", STANDALONE_LESSON_INDEX + 1);
        n3_begin(nehe_lesson_clear_color(STANDALONE_LESSON_INDEX), nehe_lesson_blend_enabled(STANDALONE_LESSON_INDEX));
        nehe_lesson_render(STANDALONE_LESSON_INDEX, t);
        n3_finish(title, nehe_lesson_detail(STANDALONE_LESSON_INDEX));
    }

    nehe_lessons_shutdown();
    n3_shutdown();
    return 0;
}
