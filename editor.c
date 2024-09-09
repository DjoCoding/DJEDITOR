#include "headers/editor.h"

Editor editor_init(void) {
    Editor e = {0};
    e.b = buffer_init();
    editor_push_line(&e);
    return e;
}

size_t editor_get_current_line(Editor *e) {
    return e->render_row + e->cursor_row;
}

size_t editor_get_current_char(Editor *e) {
    return e->render_col + e->cursor_col;
}

void editor_push_line(Editor *e) {
    buffer_init_line(&e->b, e->cursor_row);
}

void editor_move_left(Editor *e, size_t w) {
    if (e->render_col + e->cursor_col > 0) { --e->cursor_col; return; }
    
    if (e->render_row + e->cursor_row == 0) { return; }
    
    --e->cursor_row;


    if (e->b.lines[e->cursor_row].count > w) {
        e->render_col = e->b.lines[e->cursor_row].count - w;
    } else {
        e->render_col = 0;
    }

    e->cursor_col = e->b.lines[e->cursor_row].count - e->render_col;
}

void editor_move_right(Editor *e) {
    Line *line = &e->b.lines[e->cursor_row];

    if (e->cursor_col + e->render_col >= line->count) {
        if (e->render_row + e->cursor_row + 1 >= e->b.count) { 
            e->cursor_row = e->b.count - 1 - e->render_row;
            return; 
        }
        
        ++e->cursor_row;

        e->render_col = 0;
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
    size_t x = editor_get_current_char(e);

    // save the initial values of the x 
    size_t init_x = x;

    buffer_insert_text_after_cursor(&e->b, text, text_size, e->render_row + e->cursor_row, &x);

    e->cursor_col += x - init_x;
}

void editor_insert_line_after_cursor(Editor *e) {
    size_t x = editor_get_current_char(e);
    size_t y = editor_get_current_line(e);

    // save the initial values of the x and y 
    size_t init_x = x;
    size_t init_y = y;
    
    buffer_insert_line_after_cursor(&e->b, &y, &x);

    // update the cursor
    e->cursor_row += y - init_y;

    e->cursor_col = 0;
    e->render_col = 0;
}

void editor_remove_text_before_cursor(Editor *e, size_t text_size) {
    size_t x = editor_get_current_char(e);
    size_t y = editor_get_current_line(e);

    // save the initial values of the x and y 
    size_t init_x = x;
    size_t init_y = y;
    
    buffer_remove_text_before_cursor(&e->b, text_size, &y, &x);

    // update the cursor
    e->cursor_row += y - init_y;
    e->cursor_col += x - init_x;
}

void editor_dump(FILE *f, Editor *e) {
    buffer_dump(f, &e->b);
}

void editor_clean(Editor *e) {
    buffer_clean(&e->b);
}