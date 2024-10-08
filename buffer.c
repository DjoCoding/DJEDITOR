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
    *col = prev->count;
    
    prev->count += line->count;
    text_size -= 1;   // for the new line char

    line_clean(line);

    for (size_t i = *row; i < b->count - 1; ++i) {
        b->lines[i] = b->lines[i + 1];
    }

    --b->count;

    *row -= 1;

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

void buffer_render_highlighted(Buffer *b, size_t fline, size_t fchar, Color_Buffer *cb, Screen *s) {
    size_t i = fline;
    while(true) {
        if ((i - fline >= s->config.h) || (i >= b->count)) { break; }
        Line *line = &b->lines[i];
        line_render_highlighted(line, i - fline, fchar, cb->colors[i], s);
        ++i;
    }
}

int buffer_find_text(Buffer *b, char *text, size_t text_size, uVec2 from, uVec2 *pos) {
    pos->y = from.y;
    Line *line = &b->lines[pos->y];
    
    // find the text in the line where the cursor is
    int isfound = line_find_text(line, text, text_size, from.x, line->count, &pos->x);
    if (isfound) { return 1; }

    ++pos->y;
    while(pos->y < b->count) {
        line = &b->lines[pos->y];
        isfound = line_find_text(line, text, text_size, 0, line->count, &pos->x);
        if (isfound) { return 1; }
        ++pos->y;
    }

    // if not found search from the top to the cursor pos
    pos->y = 0;
    while(pos->y < from.y) {
        line = &b->lines[pos->y];
        isfound = line_find_text(line, text, text_size, 0, line->count, &pos->x);
        if (isfound) { return 1; }
        ++pos->y;
    }


    // the line we began with 
    line = &b->lines[pos->y];
    isfound = line_find_text(line, text, text_size, 0, line->count, &pos->x);
    return isfound;
}

// _size for the original text that's already stored in the buffer
void buffer_replace_text(Buffer *b, char *text, size_t text_size, size_t _size, uVec2 pos) {
    Line *line = &b->lines[pos.y];
    line_replace_text(line, text, text_size, _size, pos.x);
}

Token_Container buffer_tokenize(Buffer *b, Tokenizer *tk) {
    Token_Container container = tkn_container_init(b->count);
    for(size_t i = 0; i < b->count; ++i) {
        Line line = b->lines[i];
        Tokens tks = line_tokenize(&line, tk);
        container.tks[i] = tks;
    }
    return container;
}

void buffer_dump(FILE *f, Buffer *b) {
    for(size_t i = 0; i < b->count; ++i) {
        line_dump(f, &b->lines[i]);
        fprintf(f, "\n");
    }
}
