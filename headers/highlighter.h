#ifndef HIGHLIGHTER_H_
#define HIGHLIGHTER_H_

#include "color.h"

typedef struct {
    uVec2 pointer;
} Highlighter;

Highlighter hl_init(size_t row, size_t col);
void hl_set_color(Highlighter *);

#endif