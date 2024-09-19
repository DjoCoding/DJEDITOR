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
    return e->camera.y + e->cursor.y;
}

size_t editor_get_current_char(Editor *e) {
    return e->camera.x + e->cursor.x;
}

void editor_push_line(Editor *e) {
    buffer_init_line(&e->b, e->cursor.y);
}

void editor_move_left(Editor *e, size_t w) {
    if (e->camera.x + e->cursor.x > 0) { --e->cursor.x; return; }
    
    if (e->camera.y + e->cursor.y == 0) { return; }
    
    --e->cursor.y;


    if (e->b.lines[e->cursor.y].count > w) {
        e->camera.x = e->b.lines[e->cursor.y].count - w;
    } else {
        e->camera.x = 0;
    }

    e->cursor.x = e->b.lines[e->cursor.y].count - e->camera.x;
}

void editor_move_right(Editor *e) {
    Line *line = &e->b.lines[e->cursor.y];

    if (e->cursor.x + e->camera.x >= line->count) {
        if (e->camera.y + e->cursor.y + 1 >= e->b.count) { 
            e->cursor.y = e->b.count - 1 - e->camera.y;
            return; 
        }
        
        ++e->cursor.y;

        e->camera.x = 0;
        e->cursor.x = 0;

        return;
    }

    ++e->cursor.x;
}

void editor_move_up(Editor *e) {
    if (e->cursor.y == 0) { e->cursor.x = 0; return; }
    --e->cursor.y;
    
    Line *line = &e->b.lines[e->cursor.y];
    
    if (e->cursor.x > line->count) {
        e->cursor.x = line->count;
    }
}

void editor_move_down(Editor *e) {
    if (e->cursor.y + 1 == e->b.count) { e->cursor.x = e->b.lines[e->cursor.y].count; return; }
    ++e->cursor.y;
    
    Line *line = &e->b.lines[e->cursor.y];
    if (e->cursor.x > line->count) {
        e->cursor.x = line->count;
    }
}

void editor_insert_text_after_cursor(Editor *e, char *text, size_t text_size) {
    size_t x = editor_get_current_char(e);

    // save the initial values of the x 
    size_t init_x = x;

    buffer_insert_text_after_cursor(&e->b, text, text_size, e->camera.y + e->cursor.y, &x);

    e->cursor.x += x - init_x;
}

void editor_insert_line_after_cursor(Editor *e) {
    size_t x = editor_get_current_char(e);
    size_t y = editor_get_current_line(e);

    // save the initial values of the x and y 
    size_t init_x = x;
    size_t init_y = y;
    
    buffer_insert_line_after_cursor(&e->b, &y, &x);

    // update the cursor
    e->cursor.y += y - init_y;

    e->cursor.x = 0;
    e->camera.x = 0;
}

void editor_remove_text_before_cursor(Editor *e, size_t text_size) {
    size_t x = editor_get_current_char(e);
    size_t y = editor_get_current_line(e);

    // save the initial values of the x and y 
    size_t init_x = x;
    size_t init_y = y;
    
    buffer_remove_text_before_cursor(&e->b, text_size, &y, &x);

    // update the cursor
    e->cursor.y += y - init_y;
    e->cursor.x += x - init_x;
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
    
    e->cursor.x = 0;
    e->cursor.y = 0;
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


void editor_move_camera_and_cursor_to_char(Editor *e, uVec2 cpos) {
    int w = e->screens[MAIN_SCREEN].config.w;
    int h = e->screens[MAIN_SCREEN].config.h;

    e->camera = ivec2v(0);
    e->cursor = uvec2v(0);

    while (cpos.y > h) {
        e->camera.y += h;
        cpos.y -= h;
    }

    while(cpos.x > w) {
        e->camera.x += w;
        cpos.x -= w;
    }

    e->cursor = cpos;
}

void editor_adjust_camera_and_cursor(Editor *e) {
    int w = e->screens[MAIN_SCREEN].config.w;
    int h = e->screens[MAIN_SCREEN].config.h;

    if (e->cursor.x + 1 >= w) {
        size_t diffx = e->cursor.x + 1 - w;
        e->cursor.x -= diffx; 
        e->camera.x += diffx;
    }

    while (e->cursor.x < 3 && e->camera.x > 0) {
        e->cursor.x += 1;
        e->camera.x -= 1;
    }

    if (e->cursor.y + 1 >= h) {
        size_t diffy = e->cursor.y + 1 - h;
        e->cursor.y -= diffy;
        e->camera.y += diffy;
    }

    while (e->cursor.y < 3 && e->camera.y > 0) {
        e->cursor.y += 1;
        e->camera.y -= 1;
    }
}

void editor_render(Editor *e) {
    editor_adjust_camera_and_cursor(e);

    WINDOW *wind = e->screens[MAIN_SCREEN].window;

    wclear(wind);
    buffer_render(&e->b, e->camera.y, e->camera.x, &e->screens[MAIN_SCREEN]);
    wmove(wind, e->cursor.y, e->cursor.x);
    wrefresh(wind);
    
    wind = e->screens[COMMAND_SCREEN].window;

    wclear(wind);
    line_render(&e->cmd, e->screens[COMMAND_SCREEN].config.row, 0, &e->screens[COMMAND_SCREEN]);
    wmove(wind, e->screens[COMMAND_SCREEN].config.row, e->cmd_cursor);
    wrefresh(wind);

    if(e->mode == COMMAND) {
        move(e->screens[COMMAND_SCREEN].config.row, e->cmd_cursor);
    } else {
        move(e->cursor.y, e->cursor.x);        
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

int editor_find_text(Editor *e, char *text, size_t text_size, uVec2 *pos) {
    uVec2 from = uvec2(editor_get_current_char(e), editor_get_current_line(e));
    return buffer_find_text(&e->b, text, text_size, from, pos);
}


void editor_exec_command(Editor *e) {
    String_View s = sv(e->cmd.content, e->cmd.count);

    // consuming ':'
    s = sv_trim_right(sv_chop_left(s));

    if (!s.count) return;
    
    String_View arg = sv_get_arg(&s);
    if (sv_cmp(arg, SV("q"))) { e->state = STOPED; return editor_remove_command(e); }
    
    if (sv_cmp(arg, SV("w"))) {
        String_View filepath_sv = sv_get_arg(&s);
        if (sv_isnull(filepath_sv)) {
            char *msg = "no file path provided";
            editor_remove_command(e);
            return editor_insert_command_text(e, msg, strlen(msg));
        }
        char *filepath = DJ_ALLOC(filepath_sv.count + 1);
        sv_to_cstr(filepath_sv, filepath);
        editor_store_in_file(e, filepath);
        return editor_remove_command(e);
    }
    
    if (sv_cmp(arg, SV("f"))) {
        String_View text = sv_get_arg(&s);
        uVec2 pos = {0};
        int isfound = editor_find_text(e, text.content, text.count, &pos);
        if (!isfound) {
            char *msg = "not found";
            editor_remove_command(e);
            return editor_insert_command_text(e, msg, strlen(msg));
        }
        editor_move_camera_and_cursor_to_char(e, pos);
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