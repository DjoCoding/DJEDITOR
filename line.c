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
    size_t num_spaces = TAB_SIZE - *cursor % TAB_SIZE;
    for (int i = 0; i < num_spaces; ++i) {
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

void line_replace_text(Line *line, char *text, size_t text_size, size_t _size, size_t pos) {
    size_t nsize = line->count + text_size - _size;
    while(nsize > line->size) { line_resize(line); }

    memmove(&line->content[pos + text_size], &line->content[pos + _size], (line->count - pos - _size) * sizeof(char));
    memcpy(&line->content[pos], text, text_size * sizeof(char));
    line->count = nsize;
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

void line_render(Line *line, size_t row, size_t cursor, Screen *s) {
    size_t i = cursor;

    while(true) {
        if ((i >= line->count) || (i - cursor >= s->config.w)) { break; }
        wmove(s->window, (int)row, (int)(i - cursor));
        waddch(s->window, line->content[i]);
        ++i;
    }
}

int line_find_text(Line *line, char *text, size_t text_size, size_t from, size_t *pos) {
    if (text_size > line->count - from) { return 0; }
    size_t i = from;

    while (i < line->count) {
        size_t j = 0;
        while (j < text_size) {
            if (text[j] != line->content[i + j]) { break; }
            j += 1;
        }

        if (j == text_size) {  *pos = i + j; return 1; }
        i += 1;
    }

    return 0;
}

void line_clean(Line *line) {
    free(line->content);
}