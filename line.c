#include "headers/line.h"

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

void line_insert_tab(Line *line, size_t *cursor) {    
    for (int i = 0; i < TAB_SIZE; ++i) {
        line_insert_char(line, ' ', cursor);
    }
}

void line_insert_char(Line *line, char c, size_t *cursor) {
    if (c == '\t') { return line_insert_tab(line, cursor); }

    line->content[*cursor] = c;
    
    *cursor += 1; 
    line->count += 1;
}

void line_insert_text_after_cursor(Line *line, char *text, size_t text_size, size_t *cursor) {
    size_t text_size_with_tabs = 0;
    for (size_t i = 0; i < text_size; ++i) {
        text_size_with_tabs += (text[i] == '\t') ? TAB_SIZE: 1;
    }

    while (line->count + text_size_with_tabs >= line->size) { line_resize(line); }

    memmove(line->content + *cursor + text_size_with_tabs, line->content + *cursor, line->count - *cursor);
    
    for (size_t i = 0; i < text_size; ++i) {
        line_insert_char(line, text[i], cursor);
    }
}

void line_remove_text_before_cursor(Line *line, size_t text_size, size_t *cursor) {
    DJ_ASSERT(*cursor >= text_size, "`line_remove_text_before_cursor` failed, found an unreachable situation");
    memmove(&line->content[*cursor] - text_size, &line->content[*cursor], sizeof(char) * (line->count - *cursor));
    line->count -= text_size;
    *cursor -= text_size;
}

void line_dump(FILE *f, Line *line) {
    fwrite(line->content, sizeof(char), line->count, f);
}

void line_clean(Line *line) {
    free(line->content);
}