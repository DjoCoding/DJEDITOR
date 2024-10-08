#ifndef LINE_H
#define LINE_H

#include "env.h"
#include "color.h"
#include "tokenizer.h" 
#include "screen.h"

#define TAB_SIZE 3

typedef struct {
    char *content;
    size_t count;
    size_t size;
} Line;

#define LINE_INIT_SIZE 1024

Line line_init(void);
void line_reset(Line *line);
void line_resize(Line *line);

void line_insert_char(Line *line, char c, size_t *cursor);
void line_insert_tab(Line *line, size_t *cursor);
void line_insert_text_after_cursor(Line *line, char *text, size_t text_size, size_t *cursor);
void line_replace_text(Line *line, char *text, size_t text_size, size_t orgtext_size, size_t pos);
void line_remove_text_before_cursor(Line *line, size_t text_size, size_t *cursor);

void line_render(Line *line, size_t row, size_t cursor, Screen *s);
void line_render_highlighted(Line *line, size_t row, size_t cursor, int *colors, Screen *s);

int line_find_text(Line *line, char *text, size_t text_size, size_t from, size_t to, size_t *pos);

Tokens line_tokenize(Line *line, Tokenizer *tk);

void line_dump(FILE *f, Line *line);
void line_clean(Line *line);


#endif 