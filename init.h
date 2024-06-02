#ifndef INIT_H
#define INIT_H

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "./types.h"
#include "./consts.h"
#include "./macros.h"

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

static void editor_init_theme(EDITOR *editor) {
    (void)editor;
    
    start_color();

    // INITIALZING THE MAIN COLOR THEME
    INIT_MAIN_THEME();

    // INITIALIZING THE LINE NUMBER COLOR THEME
    INIT_LINE_NUMBER_THEME();
}

static void editor_set_main_theme(EDITOR *editor) {
    (void)editor;
    // FILL THE BACKGROUND OF THE WINDOW WITH THE MAIN THEME
    wbkgd(stdscr, MAIN_THEME);
}

static void editor_reset_theme(EDITOR *editor) {
    (void)editor;
    
    // GET BACK TO DEFAULT THEME
    wbkgd(stdscr, use_default_colors());
}


EDITOR editor_init() {
    EDITOR editor = 
    (EDITOR) {
        .config = (EDITOR_CONFIG) {
            .buff = buff_init(),
            .cursor = cursor_init(),
            .mode = NORMAL,
            .state = RUNNING,
            .FILE_NAME = NULL,
        },
        .snapshots = NULL,
    };
    editor_init_bottom_window(&editor);
    editor_init_theme(&editor);
    editor_set_main_theme(&editor);
    return editor;
}


#endif 