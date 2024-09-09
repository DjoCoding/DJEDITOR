#ifndef LINE_H
#define LINE_H

#include "env.h"

typedef struct {
    char *content;
    size_t count;
    size_t size;
} Line;

#define LINE_INIT_SIZE 1024

Line line_init(void);
void line_reset(Line *line);
void line_resize(Line *line);
void line_insert_text_after_cursor(Line *line, char *text, size_t text_size, size_t *cursor);
void line_dump(FILE *f, Line *line);
void line_clean(Line *line);


#endif 