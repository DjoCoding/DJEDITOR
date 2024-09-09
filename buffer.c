#include "buffer.h"

Buffer buffer_init(void) {
    Buffer b = {0};
    b.size = BUFFER_INIT_SIZE;
    b.lines = DJ_ALLOC(b.size * sizeof(Line));
    memset(b.lines, 0, sizeof(Line) * b.size);
    return b;
}

void buffer_resize(Buffer *b) {
    if (b->size == 0) { b->size = BUFFER_INIT_SIZE; }
    else { b->size *= 2; }

    b->lines = DJ_REALLOC(b->lines, sizeof(Line), b->size);
    memset(&b->lines[b->count], 0, sizeof(Line) * (b->size - b->count));
}

void buffer_insert_text_after_cursor(Buffer *b, char *text, size_t text_size, size_t row, size_t *col) {
    Line *line = &b->lines[row];
    line_insert_text_after_cursor(line, text, text_size, col);    
}

void buffer_insert_line_after_cursor(Buffer *b, size_t *row, size_t *col) {
    if (b->count + 1 >= b->size) { buffer_resize(b); }

    for (size_t i = b->count; i > *row + 1; --i) {
        b->lines[i] = b->lines[i - 1];
    }
    b->lines[*row + 1] = line_init(); // initialize the new line

    size_t text_size = b->lines[*row].count - *col;
    memcpy(b->lines[*row + 1].content, &b->lines[*row].content[*col], text_size);

    b->lines[*row + 1].count = text_size;
    b->lines[*row].count -= text_size;

    *row += 1;
    *col = 0;

    b->count += 1;
}

void buffer_init_line(Buffer *b, size_t row) {
    if (b->count >= b->size) { buffer_resize(b); }
    b->lines[row] = line_init();    
    b->count += 1;
}

void buffer_clean(Buffer *b) {
    for (size_t i = 0; i < b->count; ++i) {
        if (b->lines[i].size != 0) { line_clean(&b->lines[i]); }
    }

    free(b->lines);
}  

void buffer_dump(FILE *f, Buffer *b) {
    for(size_t i = 0; i < b->count; ++i) {
        line_dump(f, &b->lines[i]);
        fprintf(f, "\n");
    }
}
