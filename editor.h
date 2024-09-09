#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"

typedef enum {
    RUNNING, 
    STOPED,
} State;

typedef struct {
    Buffer b;
    size_t cursor_row;
    size_t cursor_col;
    State state;
} Editor;

Editor editor_init(void);
void editor_push_line(Editor *e);
void editor_move_left(Editor *e);
void editor_move_right(Editor *e);
void editor_move_up(Editor *e);
void editor_move_down(Editor *e);
void editor_insert_text_after_cursor(Editor *e, char *text, size_t text_size);
void editor_insert_line_after_cursor(Editor *e);
void editor_dump(FILE *f, Editor *e);
void editor_clean(Editor *e);


#endif