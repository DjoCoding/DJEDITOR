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
void editor_render(EDITOR *editor);

void editor_load_file(EDITOR *editor, char *filename);

static void editor_go_left(EDITOR *editor);
static void editor_go_right(EDITOR *editor);
static void editor_go_down(EDITOR *editor);
static void editor_go_up(EDITOR *editor);

#define DED_IMPLEMENTATION


static void row_remove(ROW **row) {
    free((*row)->content);
    free(*row);
    *row = NULL;
}

static void editor_remove(EDITOR *editor) {
    // DELETE ALL THE ROWS WITH THEIR CONTENTS
    ROW *current = editor->buff.rows;
    while (current != NULL) {
        ROW *next = current->next;
        row_remove(&current);
        current = next;
    }

    editor->buff.rows = NULL;
    free(editor->FILE_NAME);
    delwin(editor->windows[BOTTOM_WINDOW]);
}

static bool editor_check_curr_row_full(EDITOR *editor) {
    return (editor->buff.current_row->size == editor->buff.current_row->cap);
}

static void editor_resize_row(ROW **row) {
    // DOUBLE THE CAPACITY OF THE CURRENT ROW 
    (*row)->cap *= 2;

    // REALLOCATE MEMORY FOR IT
    (*row)->content = realloc((*row)->content, (*row)->cap);
}

static void editor_resize_curr_row(EDITOR *editor) {
    editor_resize_row(&(editor->buff.current_row));
}

void editor_get_cursor_pos_from_buffer(EDITOR *editor) {
    editor->cursor.pos.col++;
}

// THIS WILL ADD A NEW ROW AFTER THE CURRENT ROW IN THE BUFFER
static void editor_add_new_row(EDITOR *editor) {
    ROW *new_row = row_init(editor);
    
    // IF THE EDITOR DOESN'T HAVE ANY LINES THEN UPDATE ALL THE POINTERS
    if (editor->buff.current_row == NULL) {
        editor->buff.rows = new_row;
        editor->buff.current_row = new_row;
        editor->buff.tail = new_row;
    } else {
        new_row->next = editor->buff.current_row->next;
        new_row->prev = editor->buff.current_row;
        
        editor->buff.current_row->next = new_row;
    }

    editor->buff.size++;
    new_row = NULL;
}

static void editor_buffer_add_char(EDITOR *editor, int ch) {
    ROW *curr_row = editor->buff.current_row;

    // IF THE CHAR IS A NEW LINE CHAR ('\n') THEN WE ADD A NEW ROW IN THE BUFFER
    if(ch == NEW_LINE_CHAR) {
        editor_add_new_row(editor);
        editor_go_down(editor);
        return;
    }
    
    // IF NO ROW IS FOUND THEN MAKE A NEW ONE
    if (curr_row == NULL) {
        editor_add_new_row(editor);
        curr_row = editor->buff.current_row;
    }

    // FIRST I HAVE TO CHECK IF THE CURRENT ROW IS FULL OF CHARS THEN RESIZE IT
    if (editor_check_curr_row_full(editor)) {
        editor_resize_curr_row(editor);
    }
    
    // IF THE CURSOR ISN'T AT THE END OF THE LINE THEN SHIFT ALL THE CONTENT TO THE RIGHT OF THE CURSROR AND APPLY THE CHANGES
    size_t i = curr_row->size;
    while (i > editor->cursor.pos.col) {
        curr_row->content[i] = curr_row->content[i - 1];
        i--;
    }
    
    // PUT THE CHARACTER IN ITS PLACE
    curr_row->content[editor->cursor.pos.col] = ch;


    // INCREMENT THE CURRENT ROW SIZE
    curr_row->size++;


    // MOVE THE CURSOR TO THE RIGHT
    editor_go_right(editor);
    
    if (ch == '(' || ch == '{') {
        int ch_rev;
        switch (ch) {
            case '(':
                ch_rev = ')';
                break;
            case '{':
                ch_rev = '}';
                break;
        }

        editor_buffer_add_char(editor, ch_rev);
        editor_go_left(editor);
        return;
    }
} 

// DO IT AGAIN
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


void editor_save_in_file(EDITOR *editor) {
    FILE *fp = fopen(editor->FILE_NAME, "w");
    ROW *current = editor->buff.rows;

    size_t index;
    
    while (current != NULL) {
        index = 0;
        while (index < current->size) {
            fprintf(fp, "%c", current->content[index]);
            index++;
        }
        fprintf(fp, "%c", NEW_LINE_CHAR);
        current = current->next;
    }
    
    fclose(fp);
}

// ADD IT TO THE BUFFER AT THE CURRENT POSITION OF THE CURSOR
// UPDATE THE CURSOR 
// IF THE CHAR IS THE NEW LINE CHAR THEN UPDATE THE BUFFER LINES
// UPDATE THE MODE IF WE'RE AT THE NORMAL MODE

static void editor_handle_normal_char(EDITOR *editor, int ch) {
    if (editor->mode == NORMAL) {
        if (ch == 'i') {
            // SWITCH THE MODE TO INSERT
            editor->mode = INSERT;
        } else if (ch == ':') {
            // HANDLE THE COMMANDS GIVEN
        }
    } else if (editor->mode == INSERT) {
        if (ch == CTRL('s')) {
            // ASK THE USER FOR THE NAME OF THE FILE IF IT'S NOT PROVIDED AT THE COMMAND LINE
        
            if (editor->FILE_NAME == NULL) {
                editor_ask_user_for_input_on_bottom_window(editor);
            }

            // OPEN A NEW FILE AND COPY THE BUFFER THERE
            editor_save_in_file(editor);

        } else if (ch == CTRL('d')) {
            // SWITCH THE MODE TO NORMAL
            editor->mode = NORMAL;

        }  else {            
                // PUT THE NEW CHAR AT PLACE TO RENDER AFTER
                editor_buffer_add_char(editor, ch);
            }
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
    mvwprintw(editor->windows[BOTTOM_WINDOW], 0, (window_width - 20) / 2, "%ld : %ld, %ld", row, col, editor->buff.current_row->size);

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

static void editor_go_left(EDITOR *editor) {
    if (editor->cursor.pos.col > 0) {
        editor->cursor.pos.col--;    
    } else if (editor->buff.current_row->prev) {
        editor->cursor.pos.row--;

        editor->buff.current_row = editor->buff.current_row->prev;
        
        editor->cursor.pos.col = editor->buff.current_row->size;
    }
    move(editor->cursor.pos.row, editor->cursor.pos.col);
}

static void editor_go_right(EDITOR *editor) {
    if (editor->cursor.pos.col < editor->buff.current_row->size) {
        editor->cursor.pos.col++;    
    } else if (editor->buff.current_row->next) {
        editor->cursor.pos.row++;

        editor->buff.current_row = editor->buff.current_row->next;
        
        editor->cursor.pos.col = 0;
    }
    move(editor->cursor.pos.row, editor->cursor.pos.col);
}

static void editor_go_down(EDITOR *editor) {
    // THERE EXISTS MORE ROWS AFTER THE CURRENT ROW
    if (editor->buff.current_row->next) {
        editor->cursor.pos.row++;
        editor->buff.current_row = editor->buff.current_row->next;

        if (editor->cursor.pos.col > editor->buff.current_row->size) 
            editor->cursor.pos.col = editor->buff.current_row->size;
    } else  
        editor->cursor.pos.col = editor->buff.current_row->size;        


    move(editor->cursor.pos.row, editor->cursor.pos.col);
}

static void editor_go_up(EDITOR *editor) {
    if (editor->buff.current_row->prev) {
        editor->cursor.pos.row--;
        editor->buff.current_row = editor->buff.current_row->prev;
        
        if (editor->cursor.pos.col > editor->buff.current_row->size) 
            editor->cursor.pos.col = editor->buff.current_row->size;
    } else 
        editor->cursor.pos.col = 0;
        
    move(editor->cursor.pos.row, editor->cursor.pos.col);
}

static void editor_append_row_to_another(EDITOR *editor, ROW **dest, ROW **src) {   
    // JUST IN ORDER TO FOLLOW THE PARAMETER CONVENTIONS 
    (void) editor;

    while ((*dest)->size + (*src)->size > (*dest)->cap) {
        editor_resize_row(dest);
    }

    memcpy((*dest)->content + (*dest)->size, (*src)->content, sizeof(char) * (*src)->size);

    (*dest)->size += (*src)->size;

    ROW *next = (*src)->next;
    row_remove(src);
    (*dest)->next = next;

    return;
}


static void editor_remove_char(EDITOR *editor) {
    if (editor->cursor.pos.row <= 0 && editor->cursor.pos.col <= 0) return;

    if (editor->buff.current_row->size == 0) {
        if (editor->buff.current_row->prev == NULL) return;
        
        editor_go_left(editor);
        
        row_remove(&(editor->buff.current_row->next));
        return;
    }

    // UPDATE THE CURSOR POSITION
    editor_go_left(editor);

    // SHIFT ALL THE BUFFER ONCE TO THE LEFT
    size_t i = editor->cursor.pos.col;
    while (i < editor->buff.current_row->size) {
        editor->buff.current_row->content[i] = editor->buff.current_row->content[i + 1];
        i++;
    };


    // SEE ABOVE THAT I MOVED THE CURSOR TO THE LEFT 
    // THAT MEANS IF THE CURSOR POSITION IS THE SAME AS THE SIZE OF THE CURRENT ROW
    // THAT MEANS THE PREVIOUS POSITION OF THE CURSOR WAS AT POSITION 0
    
        // LATER I GOTTA MOVE ALL THE NEXT LINE (OF THE CURRENT ROW) TO THE PREV ROW AT THE END
    if (editor->cursor.pos.col != editor->buff.current_row->size) editor->buff.current_row->size--;
    else 
        // IF WE ENCOUNTERED THE SITUATION ABOVE WE APPEND THE CURRENT ROW WITH IT'S PREVIOUS
        editor_append_row_to_another(editor, &(editor->buff.current_row), &(editor->buff.current_row->next));
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
        case KEY_UP:
            editor_go_up(editor);
            break;
        case KEY_DOWN:
            editor_go_down(editor);
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
    editor_remove(&editor);
    endwin();
}

void editor_load_file(EDITOR *editor, char *filename) {
    size_t content_size = 0;

    char *content = get_file_content(filename, &content_size);

    for (size_t i = 0; i < content_size; i++) {
        editor_handle_event(editor, content[i]);
    }

    editor_render(editor);
    free(content);
}


void editor_render(EDITOR *editor) {
    wclear(stdscr);
    move(0, 0);

    ROW *current = editor->buff.rows;
    size_t row = 0, col = 0;

    while (current) {
        size_t index = 0;
        col = 0;

        while(index < current->size) {
            move(row, col++);
            waddch(stdscr, current->content[index++]); 
        }

        row++;
        current = current->next;
    }

    move(editor->cursor.pos.row, editor->cursor.pos.col);
    wrefresh(stdscr);
    if (editor->buff.current_row != NULL) editor_print_meta_data_on_bottom_window(editor);
}

#endif