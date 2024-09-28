#ifndef COLOR_H_
#define COLOR_H_

#include "env.h"

#define DEFAULT_COLOR_PAIR WHITE_ON_DEFAULT

typedef struct {
    int **colors;
    size_t size;
} Color_Buffer;

typedef enum {
    BLACK_ON_DEFAULT = 0,
    RED_ON_DEFAULT, 
    GREEN_ON_DEFAULT, 
    YELLOW_ON_DEFAULT, 
    BLUE_ON_DEFAULT, 
    MAGENTA_ON_DEFAULT, 
    CYAN_ON_DEFAULT, 
    WHITE_ON_DEFAULT,
    RED_ON_BLACK, 
    GREEN_ON_BLACK, 
    YELLOW_ON_BLACK, 
    BLUE_ON_BLACK, 
    MAGENTA_ON_BLACK, 
    CYAN_ON_BLACK, 
    WHITE_ON_BLACK
} Color;

Color_Buffer color_buffer_init(size_t size);
void color_buffer_init_row(Color_Buffer *cb, size_t row, size_t size);
void color_buffer_clean(Color_Buffer *cb);

#endif