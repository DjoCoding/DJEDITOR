#include "headers/buffer.h"

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

void buffer_remove_text_before_cursor(Buffer *b, size_t text_size, size_t *row, size_t *col) {
    if (text_size == 0) { return; }

    Line *line = &b->lines[*row];

    if (*col >= text_size) {
        return line_remove_text_before_cursor(line, text_size, col);
    }

    if (*row == 0) { return; }

    memmove(line->content, &line->content[*col], *col);
    line->count -= *col;
    text_size -= *col;

    Line *prev = &b->lines[*row - 1];
    while (prev->count + line->count >= prev->size) { line_resize(prev); }
    memcpy(&prev->content[prev->count], line->content, line->count);
    prev->count += line->count;
    text_size -= 1;   // for the new line char

    line_clean(line);
    
    for (size_t i = *row; i < b->count - 1; ++i) {
        b->lines[i] = b->lines[i + 1];
    }

    --b->count;

    *row -= 1;
    *col = prev->count;

    return buffer_remove_text_before_cursor(b, text_size, row, col);
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

void buffer_render(Buffer *b, size_t fline, size_t fchar, Screen *s) {
    size_t i = fline;
    while(true) {
        if ((i - fline >= s->config.h) || (i >= b->count)) { break; }
        Line *line = &b->lines[i];
        line_render(line, i - fline, fchar, s);
        ++i;
    }
}

void buffer_dump(FILE *f, Buffer *b) {
    for(size_t i = 0; i < b->count; ++i) {
        line_dump(f, &b->lines[i]);
        fprintf(f, "\n");
    }
}
