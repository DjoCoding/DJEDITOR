#ifndef DED_H
#define DED_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./consts.h"
#include "./types.h"
#include "./utils.h"
#include "./macros.h"


EDITOR editor = {0};

EDITOR editor_begin(EDITOR *editor);
void editor_quit();

void editor_handle_event(EDITOR *editor, int ch);
void editor_print_meta_data_on_bottom_window(EDITOR *editor);

void editor_load_file(EDITOR *editor, char *filename);

#define DED_IMPLEMENTATION


static void editor_remove(EDITOR *editor) {
    free(editor->buff.content);
    free(editor->lines.buff_lines);
}

static void editor_resize_buffer(EDITOR *editor) {
    editor->buff.cap += BUFFER_SIZE_ADDED;
    editor->buff.content = realloc(editor->buff.content, editor->buff.cap);
    if (editor->buff.content == NULL) {
        fprintf(stderr, "ERROR: CAN NOT RESIZE THE BUFFER FOR MORE INPUT\n");
        exit(1);
    }
}

static bool editor_buffer_full(EDITOR *editor) {
    return (editor->buff.size == editor->buff.cap); 
}

static void editor_update_cursor_position(EDITOR *editor) {
    editor->cursor.pos = get_ncurses_cursor_position();
}

static void editor_buffer_add_char(EDITOR *editor, int ch) {
    for (size_t i = editor->buff.size; i > editor->cursor.index; i--) {
        editor->buff.content[i + 1] = editor->buff.content[i];
    }
    editor->buff.content[editor->cursor.index++] = ch;
    editor->buff.size++;
} 

static bool editor_lines_full(EDITOR *editor) {
    return (editor->lines.size == editor->lines.cap);
}

static void editor_resize_lines(EDITOR *editor) {
    editor->lines.cap += LINES_SIZE_ADDED;
    editor->lines.buff_lines = realloc(editor->lines.buff_lines, editor->lines.cap);
    if (editor->lines.buff_lines == NULL) {
        fprintf(stderr, "ERROR: CAN NOT RESIZE THE BUFFER LINES FOR MORE INPUT\n");
        exit(1);
    }
}

static void editor_add_new_line(EDITOR *editor) {
    if (editor_lines_full(editor)) editor_resize_lines(editor);

    // ADD THE NEW LINE POSIITON AFTER RESIZING THE EDITOR LINES ARRAY 
    editor->lines.buff_lines[editor->lines.size++] = buff_line_init(editor->cursor.pos, editor->cursor.index); 
}

static void editor_ask_user_for_input_on_bottom_window(EDITOR *editor) {
    wclear(editor->windows[BOTTOM_WINDOW]);
    mvwprintw(editor->windows[BOTTOM_WINDOW], 0, 1, "type the file name: ");
    
    // APPLY CHANGES
    wrefresh(editor->windows[BOTTOM_WINDOW]);
    
    editor->FILE_NAME = (char *)malloc(sizeof(char) * (MAX_INPUT_SIZE + 1));

    size_t input_size = 0;
    int ch;
    
    while (input_size < MAX_INPUT_SIZE) {
        ch = getch();
        if (ch == NEW_LINE_CHAR) break;

        editor->FILE_NAME[input_size++] = ch;

        // PRINT THE CHAR ENTERED TO THE SCREEN
        waddch(editor->windows[BOTTOM_WINDOW], ch);
        wrefresh(editor->windows[BOTTOM_WINDOW]);
    }

    editor->FILE_NAME[input_size] = NULL_TERMINATOR;
}

// ADD IT TO THE BUFFER AT THE CURRENT POSITION OF THE CURSOR
// UPDATE THE CURSOR 
// IF THE CHAR IS THE NEW LINE CHAR THEN UPDATE THE BUFFER LINES
// UPDATE THE MODE IF WE'RE AT THE NORMAL MODE

static void editor_handle_normal_char(EDITOR *editor, int ch) {
    if (editor->mode == NORMAL) {
        if (ch == 'i') {
            // SWITCH THE MODE TO INSERT
        } else if (ch == ':') {
            // HANDLE THE COMMANDS GIVEN
        }
    }
    if (ch == CTRL('s')) {
        // ASK THE USER FOR THE NAME OF THE FILE IF IT'S NOT PROVIDED AT THE COMMAND LINE
        
        if (editor->FILE_NAME == NULL) {
            editor_ask_user_for_input_on_bottom_window(editor);
        }

        // OPEN A NEW FILE AND COPY THE BUFFER THERE
        FILE *fp = fopen(editor->FILE_NAME, "w");
        fseek(fp, 0, SEEK_SET);
        fwrite(editor->buff.content, editor->buff.size, 1, fp);
        fclose(fp);

    } else if (ch == CTRL('d')) {
        // SWITCH THE MODE TO NORMAL
        editor->mode = NORMAL;
    }  else {
            // RESIZE THE EDITOR BUFFER IS IT'S FULL
            if (editor_buffer_full(editor)) editor_resize_buffer(editor);
            
            // PUT THE NEW CHAR AT PLACE
            editor_buffer_add_char(editor, ch);

            // IF THE CHAR IS A NEW LINE CHAR ('\n') THEN ADD IT TO THE EDITOR ARRAY FOR LATER USE
            if (ch == NEW_LINE_CHAR) editor_add_new_line(editor);
            
            // PRINT IT TO THE SCREEN
            addch(ch);

            // FINALLY UPDATE THE EDITOR CURSOR
            editor_update_cursor_position(editor);
        }
}

static char *editor_stringfy_state(EDITOR *editor) {
    switch (editor->state) {
        case RUNNING: 
            return "RUNNING";
        case PENDING: 
            return "PENDING";
        case EXIT:
            return "EXIT";
        default:
            return "RUNNING"; 
    }
} 

static char *editor_stringfy_mode(EDITOR *editor) {
    switch (editor->mode) {
        case NORMAL:
            return "NORMAL";
        case INSERT:
            return "INSERT";
        default:
            return "NORMAL";
    }
}

void editor_print_meta_data_on_bottom_window(EDITOR *editor) {
    // CLEARING THE WINDOW 
    wclear(editor->windows[BOTTOM_WINDOW]);
    
    // GETTING THE WINDOW SIZE
    size_t window_height, window_width;
    (void) window_height;
    
    getmaxyx(editor->windows[BOTTOM_WINDOW], window_height, window_width);

    // GETTING THE CURSOR POSITION
    size_t row = editor->cursor.pos.row, col = editor->cursor.pos.col;

    // SETTING BOLD FONT
    wattron(editor->windows[BOTTOM_WINDOW], A_BOLD);

    // PRINTING...
    mvwprintw(editor->windows[BOTTOM_WINDOW], 0, 1, "%s", editor_stringfy_mode(editor));

    // GO TO THE CENTER
    mvwprintw(editor->windows[BOTTOM_WINDOW], 0, (window_width - 20) / 2, "%ld : %ld", row, col);

    // MAKE THE EDITOR STATE BLINKING
    wattron(editor->windows[BOTTOM_WINDOW], A_BLINK);
    
    // GO TO END OF LINE
    mvwprintw(editor->windows[BOTTOM_WINDOW], 0, (window_width - 10), "%s", editor_stringfy_state(editor));

    // DISABLE THE BLIKING ATTRIBUTE
    wattroff(editor->windows[BOTTOM_WINDOW], A_BLINK);

    // SETTING OFF THE BOLD FONT
    wattroff(editor->windows[BOTTOM_WINDOW], A_BOLD);

    // APPLY CHANGES ON THE WINDOW
    wrefresh(editor->windows[BOTTOM_WINDOW]);
}

static void editor_remove_char(EDITOR *editor) {
    if (editor->cursor.index <= 0) return;

    // DECREMENT THE CURSOR INDEX
    editor->cursor.index--;
    
    // SHIFT ALL THE BUFFER ONCE TO THE LEFT
    for (size_t i = editor->cursor.index; i < editor->buff.size; i++) {
        editor->buff.content[i] = editor->buff.content[i + 1];
    }

    // DELETE THE CHAR IN NCURSES WINDOW
    move(editor->cursor.pos.row, editor->cursor.pos.col - 1);
    delch();
    
    // UPDATE THE CURSOR POSITION
    editor_update_cursor_position(editor);

    // DECREMENT THE BUFFER SIZE
    editor->buff.size--;
}


static void editor_go_left(EDITOR *editor) {
    if (editor->cursor.index > 0) {
        editor->cursor.index--;
        move(editor->cursor.pos.row, editor->cursor.pos.col - 1);
        editor_update_cursor_position(editor);
    }
}

static void editor_go_right(EDITOR *editor) {
    if (editor->cursor.index >= editor->buff.size) return;

    editor->cursor.index++;
    move(editor->cursor.pos.row, editor->cursor.pos.col + 1);
    editor_update_cursor_position(editor);
}

void editor_handle_event(EDITOR *editor, int ch) {
    switch (ch) {
        case CTRL('c'):
            editor->state = EXIT;
            break;
        case KEY_BACKSPACE:
            editor_remove_char(editor);
            break;
        case KEY_LEFT: 
            editor_go_left(editor);
            break;
        case KEY_RIGHT:
            editor_go_right(editor);
            break;
        default:
            editor_handle_normal_char(editor, ch);
            break;
    }
}

EDITOR editor_begin(EDITOR *editor) {
    ncurses_init();
    *editor = editor_init();
    return *editor;
}

void editor_quit() {
    free(editor.FILE_NAME);
    delwin(editor.windows[BOTTOM_WINDOW]);
    editor_remove(&editor);
    endwin();
}

void editor_load_file(EDITOR *editor, char *filename) {
    size_t content_size = 0;

    char *content = get_file_content(filename, &content_size);

    for (size_t i = 0; i < content_size; i++) {
        editor_handle_event(editor, content[i]);
    }

    free(content);
}

#endif