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

void editor_set_windows_config(EDITOR *editor) {
    // INITIALIZE THE RENDRER OF ALL THE WINDOWS
    for (int i = 0; i < WINDOW_COUNT; i++) {
        editor->windows[i].renderer.row_start = 0;
        editor->windows[i].renderer.col_start = 0;
    }

    size_t height, width;
    getmaxyx(stdscr, height, width);

    // INITIALIZE THE MAIN WINDOW CONFIG
    editor->windows[MAIN_WINDOW].win_height = height - 4;
    editor->windows[MAIN_WINDOW].win_width = width;
    editor->windows[MAIN_WINDOW].win_pos = (POSITION) { .row = 0, .col = 0 }; 

    // INITIALIZE THE STATUS WINDOWS CONFIG
    editor->windows[STATUS_WINDOW].win_height = 2;
    editor->windows[STATUS_WINDOW].win_width = width;
    editor->windows[STATUS_WINDOW].win_pos = (POSITION) { .row = height - 3, .col = 0 }; 


    // INITIALIZE THE INPUT WINDOW CONFIG
    editor->windows[INPUT_WINDOW].win_height = 2;
    editor->windows[INPUT_WINDOW].win_width = width;
    editor->windows[INPUT_WINDOW].win_pos = (POSITION) { .row = height - 2, .col = 0 }; 


    return;
}

void editor_init_windows(EDITOR *editor) {
    // SET EACH WINDOW CONFIGURATION
    editor_set_windows_config(editor);

    // INIT ALL THE WINDOWS WITH THEIR CONFIG 
    for (int i = 0; i < WINDOW_COUNT; i++) {
        EDITOR_WINDOW current = editor->windows[i];
        editor->windows[i].wind = window_init(current.win_height, current.win_width, current.win_pos.row, current.win_pos.col);
        // REFRESH THE WINDOW
        wrefresh(editor->windows[i].wind);
    }
}

static void editor_init_theme(EDITOR *editor) {
    (void)editor;
    
    start_color();

    // INITIALZING THE MAIN COLOR THEME
    INIT_MAIN_THEME();

    // INITIALIZING THE LINE NUMBER COLOR THEME
    INIT_LINE_NUMBER_THEME();

    init_pair(IDENTIFIER_TYPE, BRIGHT_WHITE, BLACK);
    init_pair(KEYWORD_TYPE, BRIGHT_RED, BLACK);
    init_pair(PREPROCESSOR_TYPE, BRIGHT_GREEN, BLACK);
    init_pair(SEPARATOR_TYPE, BRIGHT_YELLOW, BLACK);
    init_pair(STRING_TYPE, BRIGHT_CYAN, BLACK);
    init_pair(OPERATOR_TYPE, BRIGHT_CYAN, BLACK);
    init_pair(COMMENT_TYPE, BRIGHT_BLACK, BLACK);
    init_pair(COLON_TYPE, BRIGHT_WHITE, BLACK);
}

static void editor_set_main_theme(EDITOR *editor) {
    (void)editor;
    // FILL THE BACKGROUND OF THE WINDOW WITH THE MAIN THEME
    wbkgd(editor->windows[MAIN_WINDOW].wind, MAIN_THEME);
    // SET THE THEME
    wattron(editor->windows[MAIN_WINDOW].wind, MAIN_THEME);
}

static void editor_reset_theme(EDITOR *editor) {
    (void)editor;
    
    // GET BACK TO DEFAULT THEME
    wbkgd(editor->windows[MAIN_WINDOW].wind, use_default_colors());
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

    editor_init_windows(&editor);

    editor_init_windows(&editor);
    editor_init_theme(&editor);
    editor_set_main_theme(&editor);

    return editor;
}


#endif 
