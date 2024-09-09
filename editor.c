#include "editor.h"

Editor editor_init(void) {
    Editor e = {0};
    e.b = buffer_init();
    return e;
}

void editor_push_line(Editor *e) {
    buffer_init_line(&e->b, e->cursor_row);
}

void editor_move_left(Editor *e) {
    if (e->cursor_col > 0) { --e->cursor_col; return; }
    
    if (e->cursor_row == 0) { return; }
    
    --e->cursor_row;
    e->cursor_col = e->b.lines[e->cursor_row].count;
}

void editor_move_right(Editor *e) {
    Line *line = &e->b.lines[e->cursor_row];

    if (e->cursor_col == line->count) {
        if (e->cursor_row + 1 == e->b.count) { return; }
        ++e->cursor_row;
        e->cursor_col = 0;
        return;
    }

    ++e->cursor_col;
}

void editor_move_up(Editor *e) {
    if (e->cursor_row == 0) { e->cursor_col = 0; return; }
    --e->cursor_row;
    
    Line *line = &e->b.lines[e->cursor_row];
    
    if (e->cursor_col > line->count) {
        e->cursor_col = line->count;
    }
}

void editor_move_down(Editor *e) {
    if (e->cursor_row + 1 == e->b.count) { e->cursor_col = e->b.lines[e->cursor_row].count; return; }
    ++e->cursor_row;
    
    Line *line = &e->b.lines[e->cursor_row];
    if (e->cursor_col > line->count) {
        e->cursor_col = line->count;
    }
}

void editor_insert_text_after_cursor(Editor *e, char *text, size_t text_size) {
    buffer_insert_text_after_cursor(&e->b, text, text_size, e->cursor_row, &e->cursor_col);
}

void editor_insert_line_after_cursor(Editor *e) {
    buffer_insert_line_after_cursor(&e->b, &e->cursor_row, &e->cursor_col);
}

void editor_dump(FILE *f, Editor *e) {
    buffer_dump(f, &e->b);
}

void editor_clean(Editor *e) {
    buffer_clean(&e->b);
}