#ifndef BUFFER_H
#define BUFFER_H

#include "line.h"
#include "tokenizer.h"
#include "../tools/vec2.h"

typedef struct {
    Line *lines;
    size_t count;
    size_t size;
} Buffer;

#define BUFFER_INIT_SIZE  100

Buffer buffer_init(void);
void buffer_resize(Buffer *b);

void buffer_init_line(Buffer *b, size_t row);
void buffer_insert_text_after_cursor(Buffer *b, char *text, size_t text_size, size_t row, size_t *col);
void buffer_insert_line_after_cursor(Buffer *b, size_t *row, size_t *col);
void buffer_remove_text_before_cursor(Buffer *b, size_t text_size, size_t *row, size_t *col);

void buffer_render(Buffer *b, size_t fline, size_t fchar, Screen *s);
void buffer_render_highlighted(Buffer *b, size_t fline, size_t fchar, Color_Buffer *cb, Screen *s);

int buffer_find_text(Buffer *b, char *text, size_t text_size, uVec2 from, uVec2 *pos);
void buffer_replace_text(Buffer *b, char *text, size_t text_size, size_t _size, uVec2 pos);

Token_Container buffer_tokenize(Buffer *b, Tokenizer *tk);

void buffer_dump(FILE *f, Buffer *b);
void buffer_clean(Buffer *b);


#endif 