#ifndef INIT_H
#define INIT_H

#include <stdio.h>
#include <stdlib.h>

#include "./types.h"
#include "./consts.h"

#define INIT_IMPLEMENTATION


ROW *row_init() {
    ROW *row = (ROW *)malloc(sizeof(ROW));
    *row = (ROW) {
        .content = (char *)malloc(sizeof(char) * BUFFER_ROW_INIITIAL_SIZE),
        .size = 0,
        .cap = BUFFER_ROW_INIITIAL_SIZE,
        .next = NULL,
        .prev = NULL,
    };
    return row;
}

BUFFER buff_init() {
    BUFFER buff = (BUFFER) {
        .rows = NULL,
        .tail = NULL,
        .current_row = NULL,
        .size = 0,
    };
    return buff;
}

POSITION pos_init(size_t row, size_t col) {
    return (POSITION) {
        .row = row,
        .col = col,
    };
} 

CURSOR cursor_init() {
    return (CURSOR) {
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
                .mode = NORMAL,
                .state = RUNNING,
                .FILE_NAME = NULL,
            };
    editor_init_bottom_window(&editor);
    return editor;
}


#endif 