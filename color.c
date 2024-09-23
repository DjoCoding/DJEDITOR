#include "headers/color.h"

Color_Buffer color_buffer_init(size_t size) {
    Color_Buffer cb = {0};
    cb.size = size;
    cb.colors = DJ_ALLOC(sizeof(int *) * size);
    memset(cb.colors, 0, sizeof(int *) * size);
    return cb;
}

void color_buffer_init_row(Color_Buffer *cb, size_t row, size_t size) {
    cb->colors[row] = DJ_ALLOC(sizeof(int) * size); 
}

void color_buffer_clean(Color_Buffer *cb) {
    for(size_t i = 0; i < cb->size; ++i) {
        if (cb->colors[i]) { free(cb->colors[i]); }
    }
    free(cb->colors);
}
