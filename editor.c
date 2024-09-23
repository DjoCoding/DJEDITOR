#include "headers/editor.h"

Editor editor_init(void) {
    Editor e = {0};
    
    e.b = buffer_init();
    editor_push_line(&e);


    int w = 0, h = 0;
    getmaxyx(stdscr, h, w);


    Config main_config = config_init(w, h - 1, 0, 0);

    e.screens[MAIN_SCREEN] = screen_init(main_config);

    Config cmd_config = config_init(w, 1, h - 1, 0);
    e.screens[COMMAND_SCREEN] = screen_init(cmd_config);

    e.cmd_line = line_init();
    e.hist = cmds_init();

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

void editor_move_left_text(Editor *e) {
    size_t w = e->screens[MAIN_SCREEN].config.w;

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

void editor_move_right_text(Editor *e) {
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

void editor_move_up_text(Editor *e) {
    if (e->mode == COMMAND) { return; }

    if (e->cursor.y == 0) { e->cursor.x = 0; return; }
    --e->cursor.y;
    
    Line *line = &e->b.lines[e->cursor.y];
    
    if (e->cursor.x > line->count) {
        e->cursor.x = line->count;
    }
}

void editor_move_down_text(Editor *e) {
    if (e->mode == COMMAND) { return; }

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

void editor_set_default_color(Editor *e) {
    for(size_t i = 0; i < SCREENS_COUNT; ++i) {
        wattron(e->screens[i].window, COLOR_PAIR(DEFAULT_COLOR_PAIR));
    }
}

void editor_render(Editor *e) {
    editor_adjust_camera_and_cursor(e);

    WINDOW *wind = e->screens[MAIN_SCREEN].window;

    wclear(wind);
    buffer_render(&e->b, e->camera.y, e->camera.x, &e->screens[MAIN_SCREEN]);
    wrefresh(wind);
    
    wind = e->screens[COMMAND_SCREEN].window;

    wclear(wind);
    line_render(&e->cmd_line, e->screens[COMMAND_SCREEN].config.row, 0, &e->screens[COMMAND_SCREEN]);
    wrefresh(wind);

    if(e->mode == COMMAND) {
        move(e->screens[COMMAND_SCREEN].config.row, e->cmd_cursor);
    } else {
        move(e->cursor.y, e->cursor.x);        
    }
}

int editor_find_text(Editor *e, char *text, size_t text_size, uVec2 *pos) {
    uVec2 from = uvec2(editor_get_current_char(e), editor_get_current_line(e));
    return buffer_find_text(&e->b, text, text_size, from, pos);
}

void editor_replace_text(Editor *e, char *text, size_t text_size, size_t _size, uVec2 pos) {
    buffer_replace_text(&e->b, text, text_size, _size, pos);
}


void editor_insert_command_text(Editor *e, char *text, size_t text_size) {
    Line *cmd = &e->cmd_line;
    line_insert_text_after_cursor(cmd, text, text_size, &e->cmd_cursor);
}

void editor_remove_command_text(Editor *e, size_t text_size) {
    return line_remove_text_before_cursor(&e->cmd_line, text_size, &e->cmd_cursor);
}

void editor_remove_command(Editor *e) {
    e->cmd_line.count = 0;
    e->cmd_cursor = 0;
}

void editor_move_left_command(Editor *e) {
    if (e->cmd_cursor == 0) { return; }
    --e->cmd_cursor;
}

void editor_move_right_command(Editor *e) {
    if (e->cmd_cursor < e->cmd_line.count) {
        ++e->cmd_cursor;
    }
}

void editor_move_up_command(Editor *e) {
    if (e->hist.count == 0) { return editor_remove_command(e); }
    if (e->hist.current > 0) --e->hist.current;
    Command cmd = cmds_get_cmd(&e->hist);
    return cmd_to_buffer_line(cmd, &e->cmd_line);
}

void editor_move_down_command(Editor *e) {
    if (e->hist.current < e->hist.count) { ++e->hist.current; }

    if (e->hist.current < e->hist.count) {
        Command cmd = cmds_get_cmd(&e->hist);
        return cmd_to_buffer_line(cmd, &e->cmd_line);
    }
    
    editor_remove_command(e);
}

void editor_push_command_to_history(Editor *e) {
    Line cmd_line = e->cmd_line;
    cmds_insert_cmd(&e->hist, cmd_line.content, cmd_line.count);
}

void editor_exec_last_command(Editor *e) {
    if (e->hist.count == 0) { return; }
    e->hist.current = e->hist.count - 1;
    cmd_to_buffer_line(cmds_get_cmd(&e->hist), &e->cmd_line);
    editor_exec_command(e);
}


void editor_exec_command(Editor *e) {
    String_View s = sv(e->cmd_line.content, e->cmd_line.count);

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

    if (sv_cmp(arg, SV("r"))) {
        String_View org = sv_get_arg(&s);
        uVec2 pos = {0};

        int isfound = editor_find_text(e, org.content, org.count, &pos);
        if (!isfound) { return; }
        
        String_View new = sv_get_arg(&s);
        uVec2 begin = uvec2_subx(pos, org.count);
        editor_replace_text(e, new.content, new.count, org.count, begin);
        
        editor_move_camera_and_cursor_to_char(e, uvec2_addx(begin, new.count));
        return editor_remove_command(e);
    }

    char *msg = "no command";
    editor_remove_command(e);
    return editor_insert_command_text(e, msg, strlen(msg));
}

void editor_shape_color_buffer(Editor *e) {
    size_t size = e->b.count;
    e->cb = color_buffer_init(size);
    for(size_t i = 0; i < size; ++i) {
        color_buffer_init_row(&e->cb, i, e->b.lines[i].count);
    }
}

void editor_clean_color_buffer_and_reshape(Editor *e) {
    color_buffer_clean(&e->cb);
    editor_shape_color_buffer(e);
}

void editor_clean(Editor *e) {
    buffer_clean(&e->b);
    line_clean(&e->cmd_line);
    cmds_clean(&e->hist);
    color_buffer_clean(&e->cb);

    for (int i = 0; i < SCREENS_COUNT; ++i) {
        delwin(e->screens[i].window);
    }
}  

void editor_dump(FILE *f, Editor *e) {
    buffer_dump(f, &e->b);
}
