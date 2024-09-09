#include "line.h"

Line line_init(void) {
    Line line = {0};
    line.size = LINE_INIT_SIZE;
    line.content = DJ_ALLOC(line.size * sizeof(char));
    memset(line.content, 0, line.size * sizeof(char));
    return line; 
}

void line_resize(Line *line) {
    if (line->size == 0) { line->size = LINE_INIT_SIZE; }
    else { line->size *= 2; }

    line->content = DJ_REALLOC(line->content, sizeof(char), line->size); 
}

void line_reset(Line *line) {
    if (line->size == 0) { *line = line_init();}
    line->count = 0;
}

void line_insert_text_after_cursor(Line *line, char *text, size_t text_size, size_t *cursor) {
    while(line->count + text_size >= line->size) { line_resize(line); }

    memmove(line->content + *cursor + text_size, line->content + *cursor, line->count - *cursor);
    memcpy(line->content + *cursor, text, text_size);

    *cursor += text_size;
    line->count += text_size;
}

void line_dump(FILE *f, Line *line) {
    fwrite(line->content, sizeof(char), line->count, f);
}

void line_clean(Line *line) {
    free(line->content);
}