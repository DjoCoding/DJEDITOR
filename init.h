#ifndef INIT_H
#define INIT_H

#include <stdio.h>

#include "./types.h"
#include "./lexer.h"

ROW row_init(size_t begin, size_t size) {
    return (ROW) {
        .begin = begin,
        .size = size,
    };
}

ROW_ARR rows_init() {
    return (ROW_ARR) {
        .rows = NULL,
        .size = 0,
        .count = 0, 
    };
}

BUFFER buffer_init() {
    return (BUFFER) {
        .content = NULL,
        .size = 0,
        .count = 0,
    };
}

WINDOW_CONFIG config_init(int height, int width, int row, int col) {
    return (WINDOW_CONFIG) {
        .height = height, 
        .width = width,
        .row = row, 
        .col = col
    };
}

void editor_init_windows(EDITOR *editor) {
    POSITION pos = get_window_size(stdscr);

    editor->windows[MAIN_WINDOW].config = config_init(pos.row, pos.col - 4, 0, 4);
    editor->windows[LINE_NUMBERS_WINDOWS].config = config_init(pos.row, 3, 0, 0);

    for (size_t window = 0; window < WINDOWS_COUNT; window++) {
        WINDOW_CONFIG config = editor->windows[window].config;
        WINDOW *wind = newwin(config.height, config.width, config.row, config.col);
        editor->windows[window].wind = wind;
    }
}

EDITOR editor_init() {
    EDITOR editor = (EDITOR) {
        .buffer = buffer_init(),
        .arr = rows_init(),
        .pos = 0,
        .mode = NORMAL,
        .state = RUNNING,
        .render = {
            .row = 0,
            .col = 0,
        },
        .highlight =  {
            .arr = tokens_init(),
            .current = 0,
            .inside_comment = 0,
        },
        .visual = {
            .begin = 0,
            .end = 0,
        },
        .filename = "./file.c",
    };

    editor_init_windows(&editor);
    return editor;
}

void editor_init_theme(EDITOR *editor) {
    (void)editor;
    init_pair(IDENTIFIER_TYPE, BRIGHT_WHITE, BLACK);
    init_pair(KEYWORD_TYPE, BRIGHT_RED, BLACK);
    init_pair(PREPROCESSOR_TYPE, BRIGHT_GREEN, BLACK);
    init_pair(SEPARATOR_TYPE, BRIGHT_YELLOW, BLACK);
    init_pair(STRING_TYPE, BRIGHT_CYAN, BLACK);
    init_pair(OPERATOR_TYPE, BRIGHT_CYAN, BLACK);
    init_pair(COMMENT_TYPE, BRIGHT_BLACK, BLACK);
    init_pair(COLON_TYPE, BRIGHT_WHITE, BLACK);

    attron(COLOR_PAIR(IDENTIFIER_TYPE));
}



// WINDOW *window_init(size_t height, size_t width, size_t row, size_t col) {
//     return newwin(height, width, row, col);
// }

// void editor_set_windows_config(EDITOR *editor) {
//     // INITIALIZE THE RENDRER OF ALL THE WINDOWS
//     for (int i = 0; i < WINDOW_COUNT; i++) {
//         editor->windows[i].renderer.row_start = 0;
//         editor->windows[i].renderer.col_start = 0;
//     }

//     size_t height, width;
//     getmaxyx(stdscr, height, width);

//     // INITIALIZE THE MAIN WINDOW CONFIG
//     editor->windows[MAIN_WINDOW].win_height = height - 4;
//     editor->windows[MAIN_WINDOW].win_width = width;
//     editor->windows[MAIN_WINDOW].win_pos = (POSITION) { .row = 0, .col = 0 }; 

//     // INITIALIZE THE STATUS WINDOWS CONFIG
//     editor->windows[STATUS_WINDOW].win_height = 2;
//     editor->windows[STATUS_WINDOW].win_width = width;
//     editor->windows[STATUS_WINDOW].win_pos = (POSITION) { .row = height - 3, .col = 0 }; 


//     // INITIALIZE THE INPUT WINDOW CONFIG
//     editor->windows[INPUT_WINDOW].win_height = 2;
//     editor->windows[INPUT_WINDOW].win_width = width;
//     editor->windows[INPUT_WINDOW].win_pos = (POSITION) { .row = height - 2, .col = 0 }; 


//     return;
// }





#endif 
