#ifndef NEHE_LESSONS_H
#define NEHE_LESSONS_H

#include <stdbool.h>
#include <stdint.h>

#define NEHE_LESSON_COUNT 17

int nehe_lessons_init(void);
void nehe_lessons_shutdown(void);
void nehe_lesson_render(int lesson, float t);
const char *nehe_lesson_title(int lesson);
const char *nehe_lesson_detail(int lesson);
bool nehe_lesson_blend_enabled(int lesson);
uint32_t nehe_lesson_clear_color(int lesson);

#endif
