#include "headers/editor.h"

Editor editor_init(void) {
    Editor e = {0};
    
    e.b = buffer_init();
    editor_push_line(&e);


    int w = 0, h = 0;
    getmaxyx(stdscr, h, w);

    // main screen configuration, full width, full height - 1 (for the command screen), starts from the top left corner
    Config main_config = config_init(w, h - 1, 0, 0);
    e.screens[MAIN_SCREEN] = screen_init(main_config);

    // command screen configuration 
    Config cmd_config = config_init(w, 1, h - 1, 0);
    e.screens[COMMAND_SCREEN] = screen_init(cmd_config);

    e.cmd = line_init();

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


void editor_load_file(Editor *e, char *filepath) {
    char *content = fcontent(filepath);
    size_t size = strlen(content);
    
    char c = 0;
    char *begin = content;
    char *current = begin;

    while ((c = *current) != 0) {
        ++current;
        if (c == '\n') {
            editor_insert_text_after_cursor(e, begin, current - begin - 1);
            editor_insert_line_after_cursor(e);
            begin = current;
        }
    }

    editor_insert_text_after_cursor(e, begin, current - begin);
    
    e->cursor_col = 0;
    e->cursor_row = 0;
}

void editor_store_in_file(Editor *e, char *filepath) {
    FILE *f = fopen(filepath, "w");
    if (!f) {
        DJ_ERROR("could not open the file %s: %s\n", filepath, strerror(errno));
    }

    for (size_t i = 0; i < e->b.count; ++i) {
        Line *line = &e->b.lines[i];
        fwrite(line->content, sizeof(char), line->count, f);
        if (i != e->b.count - 1) { fputc('\n', f); }
    }

    fclose(f);
}

void editor_render(Editor *e) {
    int w = e->screens[MAIN_SCREEN].config.w;
    int h = e->screens[MAIN_SCREEN].config.h;

    if (e->cursor_col + 1 >= w) {
        e->cursor_col -= 1;
        e->render_col += 1;    
    }

    if (e->cursor_col < 3 && e->render_col > 0) {
        e->cursor_col += 1;
        e->render_col -= 1;
    }

    if (e->cursor_row + 1 >= h) {
        e->cursor_row -= 1;
        e->render_row += 1;
    }

    if (e->cursor_row < 3 && e->render_row > 0) {
        e->cursor_row += 1;
        e->render_row -= 1;
    }

    WINDOW *wind = e->screens[MAIN_SCREEN].window;

    wclear(wind);
    buffer_render(&e->b, e->render_row, e->render_col, &e->screens[MAIN_SCREEN]);
    wmove(wind, e->cursor_row, e->cursor_col);
    wrefresh(wind);
    
    wind = e->screens[COMMAND_SCREEN].window;

    wclear(wind);
    line_render(&e->cmd, e->screens[COMMAND_SCREEN].config.row, 0, &e->screens[COMMAND_SCREEN]);
    wmove(wind, e->screens[COMMAND_SCREEN].config.row, e->cmd_cursor);
    wrefresh(wind);

    if(e->mode == COMMAND) {
        move(e->screens[COMMAND_SCREEN].config.row, e->cmd_cursor);
    } else {
        move(e->cursor_row, e->cursor_col);        
    }
}

void editor_insert_command_text(Editor *e, char *text, size_t text_size) {
    Line *cmd = &e->cmd;
    line_insert_text_after_cursor(cmd, text, text_size, &e->cmd_cursor);
}

void editor_remove_command_text(Editor *e, size_t text_size) {
    Line *cmd = &e->cmd;
    line_remove_text_before_cursor(cmd, text_size, &e->cmd_cursor);
}

void editor_remove_command(Editor *e) {
    line_reset(&e->cmd);
    e->cmd_cursor = 0;
}

void editor_exec_command(Editor *e) {
    String_View s = sv(e->cmd.content, e->cmd.count);

    // consuming ':'
    s = sv_chop_left(s);

    String_View cmd = sv_get_until(&s, ' ');
    if (cmd.count == 0) { return; }

    if (sv_cmp(cmd, SV("q"))) {
        e->state = STOPED;
        return editor_remove_command(e);
    }

    if (sv_cmp(cmd, SV("w"))) {
        s = sv_trim_left(s);
        if (s.count == 0) {
            char *msg = "no file path provided";
            editor_remove_command(e);
            return editor_insert_command_text(e, msg, strlen(msg));
        }

        cmd = s;
        char *filepath = DJ_ALLOC(cmd.count + 1);
        filepath = cmd.content;
        filepath[cmd.count] = 0;

        editor_store_in_file(e, filepath);
        return editor_remove_command(e);
    }
}

void editor_clean(Editor *e) {
    buffer_clean(&e->b);
    line_clean(&e->cmd);

    for (int i = 0; i < SCREENS_COUNT; ++i) {
        delwin(e->screens[i].window);
    }
}  