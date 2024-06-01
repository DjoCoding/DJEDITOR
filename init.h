#ifndef INIT_H
#define INIT_H

#include <stdio.h>
#include <stdlib.h>

#include "./types.h"
#include "./consts.h"

#define INIT_IMPLEMENTATION


BUFFER buff_init() {
    BUFFER buff = (BUFFER) {
        .cap = BUFFER_INIITIAL_SIZE,
        .size = 0,
        .content = (char *)malloc(sizeof(char) * BUFFER_INIITIAL_SIZE),
    };

    return buff;
}

BUFFER_LINE buff_line_init(POSITION pos, size_t index) {
    return (BUFFER_LINE) {
        .size = 0,
        .cursor_pos = pos,
        .cursor_index = index,
    };
}

BUFFER_LINES buff_lines_init() {
    return (BUFFER_LINES) {
        .buff_lines = (BUFFER_LINE *)malloc(sizeof(BUFFER_LINE) * LINES_INITIAL_SIZE),
        .size = 0,
        .cap = LINES_INITIAL_SIZE,
    };
}

POSITION pos_init(size_t row, size_t col) {
    return (POSITION) {
        .row = row,
        .col = col,
    };
} 

CURSOR cursor_init() {
    return (CURSOR) {
        .index = 0,
        .pos = pos_init(0, 0),
    };
}


WINDOW *window_init(size_t height, size_t width, size_t row, size_t col) {
    return newwin(height, width, row, col);
}


void editor_init_bottom_window(EDITOR *editor) {
    size_t window_height,window_width;
    getmaxyx(stdscr, window_height, window_width);

    box(editor->windows[BOTTOM_WINDOW], 0, 0);

    size_t height = window_height * 5 / 100;
    size_t width  = window_width;

    size_t row_start = window_height - 2 * height;
    size_t col_start = 1;

    editor->windows[BOTTOM_WINDOW] = window_init(height, width, row_start, col_start);
    wrefresh(editor->windows[BOTTOM_WINDOW]);
}

EDITOR editor_init() {
    EDITOR editor = (EDITOR) {
                .buff = buff_init(),
                .cursor = cursor_init(),
                .lines = buff_lines_init(),
                .mode = NORMAL,
                .state = RUNNING,
                .FILE_NAME = NULL,
            };
    editor_init_bottom_window(&editor);
    return editor;
}


#endif 