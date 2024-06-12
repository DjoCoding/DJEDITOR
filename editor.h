#ifndef EDITOR_H
#define EDITOR_H

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "./init.h"
#include "./types.h"
#include "./utils.h"
#include "./consts.h"
#include "./highlight.h"
#include "./config.h"

int editor_rows_full(EDITOR *editor) {
    return (editor->arr.size == editor->arr.count);
}

int editor_rows_empty(EDITOR *editor) {
    return (editor->arr.size == 0);
}

void editor_resize_rows(EDITOR *editor) {
    if (editor_rows_empty(editor)) 
        editor->arr.size = 100;
    else 
        editor->arr.size *= 2;

    editor->arr.rows = (ROW *)realloc(editor->arr.rows, sizeof(ROW) * editor->arr.size);
}

void editor_rows_add(EDITOR *editor, ROW row) {
    if (editor_rows_empty(editor)) 
        editor_resize_rows(editor);
    
    editor->arr.rows[editor->arr.count++] = row;
}

int editor_buffer_full(EDITOR *editor) {
    return (editor->buffer.size == editor->buffer.count);
}

int editor_buffer_empty(EDITOR *editor) {
    return (editor->buffer.count == 0);
}

void editor_resize_buffer(EDITOR *editor) {
    if (editor_buffer_empty(editor)) 
        editor->buffer.size = BUFFER_ROW_INIITIAL_SIZE;
    else 
        editor->buffer.size *= 2;

    editor->buffer.content = (char *)realloc(editor->buffer.content, sizeof(char) * editor->buffer.size);
}

void editor_visual_delete(EDITOR *editor) {
    if (editor->visual.begin < editor->visual.end) 
        swap(&editor->visual.begin, &editor->visual.end);
    
    memcpy(editor->buffer.content + editor->visual.begin, editor->buffer.content + editor->visual.end, editor->buffer.count - editor->visual.end);
    editor->buffer.count -= editor->visual.end;
}

void editor_buffer_add(EDITOR *editor, int ch) {
    if (editor->mode == NORMAL) {
        if (ch == 'i')
            editor->mode = INSERT;
    } else if (editor->mode == VISUAL) {
        if (ch == 'd') {
            editor->visual.end = editor->buffer.pos;
            editor_visual_delete(editor);
        }
    } else {
        if (editor_buffer_full(editor)) {
            editor_resize_buffer(editor);
        }

        if (editor->pos < editor->buffer.count) 
            memcpy(editor->buffer.content + editor->pos + 1, editor->buffer.content + editor->pos, editor->buffer.count - editor->pos);
        
        editor->buffer.content[editor->pos] = ch;
        editor->buffer.count++;
        editor->pos++;
    }
}

void editor_buffer_delete(EDITOR *editor) {
    if (editor_buffer_empty(editor)) return;
    if (editor->pos == 0) return;

    char *target = editor->buffer.content + editor->pos - 1;
    memcpy(target, target + 1, editor->buffer.count - editor->pos);
    editor->pos--;
    editor->buffer.count--;
}

POSITION editor_get_curs_position(EDITOR *editor) {
    size_t row = 0;
    size_t col = 0;

    for (size_t i = 0; i < editor->pos; i++) {
        col++;

        if (editor->buffer.content[i] == '\t') {
            col += TABSIZE - 1;
        }

        if (editor->buffer.content[i] == '\n') {
            row++;
            col = 0;
        }
    }
    
    return (POSITION) { .row = row, .col = col };
}

void editor_move_cursor(EDITOR *editor) {
    EDITOR_WINDOW window = editor->windows[MAIN_WINDOW];
    POSITION pos = editor_get_curs_position(editor);
    move(window.config.row + pos.row - editor->render.row, window.config.col + pos.col - editor->render.col);
}


void editor_set_rows(EDITOR *editor) {    
    editor->arr.count = 0;

    size_t begin = 0;
    size_t size = 0;

    for (size_t i = 0; i < editor->buffer.count; i++) {
        size++;
        if (editor->buffer.content[i] == '\n') {
            editor_rows_add(editor, row_init(begin, size));
            begin += size;
            size = 0;
        }
    }

    editor_rows_add(editor, row_init(begin, size));
}

void editor_move_left(EDITOR *editor) {
    if (editor->pos == 0) return;
    editor->pos--;
}


void editor_move_right(EDITOR *editor) {
    if (editor->buffer.count <= editor->pos) 
        return;
    editor->pos++;
}


void editor_move_up(EDITOR *editor) {
    editor_set_rows(editor);
    POSITION pos = editor_get_curs_position(editor);

    if (pos.row <= 0) {
        editor->pos = 0;
        return;
    }

    ROW row = editor->arr.rows[pos.row - 1];

    if(pos.col >= row.size) 
        editor->pos = row.begin + row.size - 1;
    else 
        editor->pos = row.begin + pos.col;
}

void editor_move_down(EDITOR *editor) {
    editor_set_rows(editor);
    POSITION pos = editor_get_curs_position(editor);
    
    if (pos.row >= editor->arr.count - 1) {
        editor->pos = editor->buffer.count; 
        return;
    }

    ROW row = editor->arr.rows[pos.row + 1];

    if(pos.col >= row.size) 
        editor->pos = row.begin + row.size;
    else 
        editor->pos = row.begin + pos.col;
}

void editor_save_in_file(EDITOR *editor) {
    FILE *fp = fopen(editor->filename, "w");
    fwrite(editor->buffer.content, sizeof(char), editor->buffer.count, fp);
    fclose(fp);
}

void editor_load_file(EDITOR *editor) {
    FILE *fp = fopen(editor->filename, "r");

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    while (size >= editor->buffer.size) 
        editor_resize_buffer(editor);

    fread(editor->buffer.content, sizeof(char), size, fp);
    
    editor->buffer.count = size;
    editor->pos = 0;

    fclose(fp);
}

void editor_event_handler(EDITOR *editor, int ch) {
    switch(ch) {
        case CTRL('c'):
            editor->state = EXITING;
            break;
        case CTRL('s'):
            editor_save_in_file(editor);
            editor->state = EXITING;
            break;
        case CTRL('n'):
            editor->mode = NORMAL;
            break;
        case CTRL('v'):
            editor->mode = VISUAL;
            editor->visual.begin = editor->buffer.pos;
            break;
        case KEY_LEFT:
            editor_move_left(editor);
            break;
        case KEY_RIGHT:
            editor_move_right(editor);
            break;
        case KEY_UP:
            editor_move_up(editor);
            break;
        case KEY_DOWN:
            editor_move_down(editor);
            break;
        case KEY_BACKSPACE:
            editor_buffer_delete(editor);
            break;
        default:
            editor_buffer_add(editor, ch);
            break;
    }
}

void editor_print_line_numbers(EDITOR *editor) {
    if (!line_numbers) return;

    EDITOR_WINDOW window = editor->windows[LINE_NUMBERS_WINDOWS];

    wclear(window.wind);
    wattron(window.wind, COLOR_PAIR(KEYWORD_TYPE));

    size_t row = editor->render.row;
    

    // -3 FOR THE STATUS WINDOW (HEIGHT = 3) // NOT IMPLEMENTED YET
    for (size_t i = 0; i < min(editor->arr.count - row, window.config.height - 3); i++) {
        mvwprintw(window.wind, i, 0, "%zu\n", row + i + 1);
    }

    wattroff(window.wind, COLOR_PAIR(KEYWORD_TYPE));
    wrefresh(window.wind);
}

void editor_render(EDITOR *editor) {
    EDITOR_WINDOW window = editor->windows[MAIN_WINDOW];

    editor_set_rows(editor);
    POSITION pos = editor_get_curs_position(editor);
    WINDOW_CONFIG config = window.config;

    config.height -= 3;
    config.width -= 3;

    // SET THE START ROW
    if (pos.row < editor->render.row) {
        editor->render.row = pos.row;
    }  else if (pos.row > config.height + editor->render.row - 1) {
        editor->render.row++;
    }

    // SET THE START COL
    if (pos.col < editor->render.col) {
        editor->render.col = pos.col;
    } else if(pos.col > config.width + editor->render.col - 1) {
        editor->render.col++;
    }

    wclear(window.wind);
    
    size_t row_render = editor->render.row;
    size_t col_render = editor->render.col;

    editor_print_line_numbers(editor);
    
    for (size_t i = 0; i < min(editor->arr.count - row_render, config.height); i++) {
        ROW current_row = editor->arr.rows[row_render + i];
        size_t begin_row = current_row.begin;
        size_t size =  current_row.size;
        size_t end_row = begin_row + size - 1;
        
        editor_lex_row(editor, begin_row + col_render, size);

        int highlight = 0;
        size_t stop_at = 0;
        size_t j = 0;

        while (j < size) {
            if (editor_start_highlight(editor, j)) {
                    highlight = 1;
                    stop_at = j + editor->highlight.arr.tokens[editor->highlight.current++].size;
                }

            if (highlight) {
                wattron(window.wind, COLOR_PAIR(editor->highlight.arr.tokens[editor->highlight.current - 1].type));
            }

            char ch = editor->buffer.content[begin_row + col_render + j];
            waddch(window.wind, ch);

            j++;

            if (j == stop_at) {
                wattroff(window.wind, COLOR_PAIR(editor->highlight.arr.tokens[editor->highlight.current - 1].type));
                highlight = 0;
            }
        }

        editor->highlight.arr.count = 0;
        editor->highlight.current = 0;
    }
    
    mvprintw(30, 10, "%3.zu : %3.zu", pos.row + 1, pos.col + 1);
    mvprintw(30, 30, "%s", stringify_mode(editor->mode));
    
    editor_move_cursor(editor);
    wrefresh(window.wind);
}

#endif