#ifndef DJEDITOR_H
#define DJEDITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#include "./consts.h"
#include "./types.h"
#include "./utils.h"
#include "./macros.h"


EDITOR editor = {0};

// START THE EDITOR
EDITOR editor_begin(EDITOR *editor);
// EVENT HANDLER
void editor_handle_event(EDITOR *editor, int ch);
// LOAD FILE TO THE EDITOR BUFFER
void editor_load_file(EDITOR *editor, char *filename);
// ROWS FUNCTIONS
static void editor_resize_row(ROW **row);
static void editor_resize_curr_row(EDITOR *editor);
static void editor_append_row_to_another(EDITOR *editor, ROW **dest, ROW **src);   
static void editor_add_new_row(EDITOR *editor);
static void row_remove(ROW **row);
// ADD THE CHARACTER TO THE EDITOR BUFFER
static void editor_handle_normal_char(EDITOR *editor, int ch);
static void editor_buffer_add_char(EDITOR *editor, int ch);
// GETTING INPUT FUNCTIONS
static void editor_ask_user_for_input_on_bottom_window(EDITOR *editor);
// HANDLING THE ARROW KEYS FUNCTIONS
static void editor_go_left(EDITOR *editor);
static void editor_go_right(EDITOR *editor);
static void editor_go_down(EDITOR *editor);
static void editor_go_up(EDITOR *editor);
// REMOVE A CHAR FROM THE EDITOR BUFFER
static void editor_remove_char(EDITOR *editor);
// REMOVE THE EDITOR CURRENT CONFIG
static void editor_remove_config(EDITOR *editor);
// RENDERING FUNCTIONS
static void editor_print_no_line(EDITOR *editor);
static void editor_print_line_number(EDITOR *editor, size_t row);
void editor_render(EDITOR *editor);
// IMPLEMENTATION OF THE UNDO OPERATION
static ROW *copy_row(ROW *row);
static ROW *get_current_rows(ROW *all_rows);
static BUFFER get_current_buffer(BUFFER buffer, size_t current_row_position);
static EDITOR_CONFIG get_current_config(EDITOR_CONFIG curr_config);
static EDITOR_CONFIG editor_get_current_config(EDITOR *editor);
static void editor_push_current_config_to_stack(EDITOR *editor);
static EDITOR_CONFIG editor_pop_config(EDITOR *editor);
void editor_save_primary_snapshot(EDITOR *editor);
static void editor_undo(EDITOR *editor);
// QUIT THE EDITOR
void editor_quit();

#define DJEDITOR_IMPLEMENTATION

// ROWS FUNCTIONS
static bool editor_check_curr_row_full(EDITOR *editor) {
    return (editor->config.buff.current_row->size == editor->config.buff.current_row->cap);
}

static void editor_resize_row(ROW **row) {
    // DOUBLE THE CAPACITY OF THE CURRENT ROW 
    (*row)->cap *= 2;

    // REALLOCATE MEMORY FOR IT
    (*row)->content = realloc((*row)->content, (*row)->cap);
}

static void editor_resize_curr_row(EDITOR *editor) {
    editor_resize_row(&(editor->config.buff.current_row));
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

// THIS WILL ADD A NEW ROW AFTER THE CURRENT ROW IN THE BUFFER
static void editor_add_new_row(EDITOR *editor) {
    ROW *new_row = row_init(editor);
    
    // IF THE EDITOR DOESN'T HAVE ANY LINES THEN UPDATE ALL THE POINTERS
    if (editor->config.buff.current_row == NULL) {
        editor->config.buff.rows = new_row;
        editor->config.buff.current_row = new_row;
        editor->config.buff.tail = new_row;
    } else {
        new_row->next = editor->config.buff.current_row->next;
        new_row->prev = editor->config.buff.current_row;
        
        editor->config.buff.current_row->next = new_row;
    }

    editor->config.buff.size++;
    new_row = NULL;
}


// ADDING A CHAR IN THE EDITOR BUFFER
static void editor_buffer_add_char(EDITOR *editor, int ch) {
    ROW *curr_row = editor->config.buff.current_row;

    // IF THE CHAR IS A NEW LINE CHAR ('\n') THEN WE ADD A NEW ROW IN THE BUFFER
    if(ch == NEW_LINE_CHAR) {
        editor_push_current_config_to_stack(editor);
        editor_add_new_row(editor);
        editor_go_down(editor);
        return;
    }
    
    // IF NO ROW IS FOUND THEN MAKE A NEW ONE
    if (curr_row == NULL) {
        editor_add_new_row(editor);
        curr_row = editor->config.buff.current_row;
    }

    // FIRST I HAVE TO CHECK IF THE CURRENT ROW IS FULL OF CHARS THEN RESIZE IT
    if (editor_check_curr_row_full(editor)) {
        editor_resize_curr_row(editor);
    }
    
    // IF THE CURSOR ISN'T AT THE END OF THE LINE THEN SHIFT ALL THE CONTENT TO THE RIGHT OF THE CURSROR AND APPLY THE CHANGES
    size_t i = curr_row->size;
    while (i > editor->config.cursor.pos.col) {
        curr_row->content[i] = curr_row->content[i - 1];
        i--;
    }
    
    // PUT THE CHARACTER IN ITS PLACE
    curr_row->content[editor->config.cursor.pos.col] = ch;


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

// READ A STRING IN THE BOTTOM WINDOW 
// (NEED TO HANDLE THE BACK SPACE KEY AND IGNORE THE TABS AND THE NEW LINE CHARS)
static void editor_ask_user_for_input_on_bottom_window(EDITOR *editor) {
    wclear(editor->windows[BOTTOM_WINDOW]);
    mvwprintw(editor->windows[BOTTOM_WINDOW], 0, 1, "type the file name: ");
    
    // APPLY CHANGES
    wrefresh(editor->windows[BOTTOM_WINDOW]);
    
    editor->config.FILE_NAME = (char *)malloc(sizeof(char) * (MAX_INPUT_SIZE + 1));

    size_t input_size = 0;
    int ch;
    
    while (input_size < MAX_INPUT_SIZE) {
        ch = getch();
        if (ch == NEW_LINE_CHAR) break;

        editor->config.FILE_NAME[input_size++] = ch;

        // PRINT THE CHAR ENTERED TO THE SCREEN
        waddch(editor->windows[BOTTOM_WINDOW], ch);
        wrefresh(editor->windows[BOTTOM_WINDOW]);
    }

    editor->config.FILE_NAME[input_size] = NULL_TERMINATOR;
}

// SAVE THE CURRENT BUFFER OF THE EDITOR IN A FILE
void editor_save_in_file(EDITOR *editor) {
    FILE *fp = fopen(editor->config.FILE_NAME, "w");
    ROW *current = editor->config.buff.rows;

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
    if (editor->config.mode == NORMAL) {
        if (ch == 'i') {
            // SWITCH THE MODE TO INSERT
            editor->config.mode = INSERT;
        } else if (ch == ':') {
            // HANDLE THE COMMANDS GIVEN
        }
    } else if (editor->config.mode == INSERT) {
        if (ch == CTRL('s')) {
            // ASK THE USER FOR THE NAME OF THE FILE IF IT'S NOT PROVIDJEDITOR AT THE COMMAND LINE
        
            if (editor->config.FILE_NAME == NULL) {
                editor_ask_user_for_input_on_bottom_window(editor);
            }

            // OPEN A NEW FILE AND COPY THE BUFFER THERE
            editor_save_in_file(editor);

        } else if (ch == CTRL('d')) {
            // SWITCH THE MODE TO NORMAL
            editor->config.mode = NORMAL;

        }  else {            
                // PUT THE NEW CHAR AT PLACE TO RENDER AFTER
                editor_buffer_add_char(editor, ch);
            }
    }
}

// ADDITIONAL FUNCTIONS
static char *editor_stringfy_state(EDITOR *editor) {
    switch (editor->config.state) {
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
    switch (editor->config.mode) {
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
    size_t row = editor->config.cursor.pos.row, col = editor->config.cursor.pos.col;

    // SETTING BOLD FONT
    wattron(editor->windows[BOTTOM_WINDOW], A_BOLD);

    // PRINTING...
    mvwprintw(editor->windows[BOTTOM_WINDOW], 0, 1, "%s", editor_stringfy_mode(editor));

    // GO TO THE CENTER
    mvwprintw(editor->windows[BOTTOM_WINDOW], 0, (window_width - 15) / 2, "%ld : %ld", row, col);

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


// HANDLING ARROW KEYS 
static void editor_go_left(EDITOR *editor) {
    if (editor->config.cursor.pos.col > 0) {
        editor->config.cursor.pos.col--;    
    } else if (editor->config.buff.current_row->prev) {
        editor->config.cursor.pos.row--;

        editor->config.buff.current_row = editor->config.buff.current_row->prev;
        
        editor->config.cursor.pos.col = editor->config.buff.current_row->size;
    }
    move(editor->config.cursor.pos.row, editor->config.cursor.pos.col);
}

static void editor_go_right(EDITOR *editor) {
    if (editor->config.cursor.pos.col < editor->config.buff.current_row->size) {
        editor->config.cursor.pos.col++;    
    } else if (editor->config.buff.current_row->next) {
        editor->config.cursor.pos.row++;

        editor->config.buff.current_row = editor->config.buff.current_row->next;
        
        editor->config.cursor.pos.col = 0;
    }
    move(editor->config.cursor.pos.row, editor->config.cursor.pos.col);
}

static void editor_go_down(EDITOR *editor) {
    // THERE EXISTS MORE ROWS AFTER THE CURRENT ROW
    if (editor->config.buff.current_row->next) {
        editor->config.cursor.pos.row++;
        editor->config.buff.current_row = editor->config.buff.current_row->next;

        if (editor->config.cursor.pos.col > editor->config.buff.current_row->size) 
            editor->config.cursor.pos.col = editor->config.buff.current_row->size;
    } else  
        editor->config.cursor.pos.col = editor->config.buff.current_row->size;        


    move(editor->config.cursor.pos.row, editor->config.cursor.pos.col);
}

static void editor_go_up(EDITOR *editor) {
    if (editor->config.buff.current_row->prev) {
        editor->config.cursor.pos.row--;
        editor->config.buff.current_row = editor->config.buff.current_row->prev;
        
        if (editor->config.cursor.pos.col > editor->config.buff.current_row->size) 
            editor->config.cursor.pos.col = editor->config.buff.current_row->size;
    } else 
        editor->config.cursor.pos.col = 0;
        
    move(editor->config.cursor.pos.row, editor->config.cursor.pos.col);
}

static void editor_remove_char(EDITOR *editor) {
    if (editor->config.cursor.pos.row <= 0 && editor->config.cursor.pos.col <= 0) return;

    if (editor->config.buff.current_row->size == 0) {
        if (editor->config.buff.current_row->prev == NULL) return;
        
        editor_go_left(editor);
        
        row_remove(&(editor->config.buff.current_row->next));
        return;
    }

    // UPDATE THE CURSOR POSITION
    editor_go_left(editor);

    // SHIFT ALL THE BUFFER ONCE TO THE LEFT
    size_t i = editor->config.cursor.pos.col;
    while (i < editor->config.buff.current_row->size) {
        editor->config.buff.current_row->content[i] = editor->config.buff.current_row->content[i + 1];
        i++;
    };


    // SEE ABOVE THAT I MOVED THE CURSOR TO THE LEFT 
    // THAT MEANS IF THE CURSOR POSITION IS THE SAME AS THE SIZE OF THE CURRENT ROW
    // THAT MEANS THE PREVIOUS POSITION OF THE CURSOR WAS AT POSITION 0
    
        // LATER I GOTTA MOVE ALL THE NEXT LINE (OF THE CURRENT ROW) TO THE PREV ROW AT THE END
    if (editor->config.cursor.pos.col != editor->config.buff.current_row->size) editor->config.buff.current_row->size--;
    else 
        // IF WE ENCOUNTERED THE SITUATION ABOVE WE APPEND THE CURRENT ROW WITH IT'S PREVIOUS
        editor_append_row_to_another(editor, &(editor->config.buff.current_row), &(editor->config.buff.current_row->next));
}


// THE EDITOR EVENT HANDLER
void editor_handle_event(EDITOR *editor, int ch) {
    switch (ch) {
        case CTRL('c'):
            editor->config.state = EXIT;
            break;
        case CTRL('z'):
            editor_undo(editor);
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


// INITIALIZES THE EDITOR AND ALLOCATING MEMORY FOR IT
EDITOR editor_begin(EDITOR *editor) {
    ncurses_init();
    *editor = editor_init();
    return *editor;
}


// QUITTING THE EDITOR AND DE-ALLOCATING ALL THE MEMORY USED
void editor_quit() {
    editor_remove_config(&editor);
    delwin(editor.windows[BOTTOM_WINDOW]);
    endwin();
}


static void row_remove(ROW **row) {
    free((*row)->content);
    free(*row);
    *row = NULL;
}

static void remove_config(EDITOR_CONFIG *config) {
    // DELETE ALL THE ROWS WITH THEIR CONTENTS
    ROW *current = config->buff.rows;
    while (current != NULL) {
        ROW *next = current->next;
        row_remove(&current);
        current = next;
    }

    config->buff.current_row = NULL;
    config->buff.tail = NULL;

    free(config->FILE_NAME);
    
    config = NULL;
}

static void editor_remove_config(EDITOR *editor) {
    // REMOVE THE EDITOR CURRENT CONFIG
    remove_config(&(editor->config));

    // DELETE ALL THE SNAPSHOTS
    EDITOR_CONFIG *config = editor->snapshots;
    while(config != NULL) {
        EDITOR_CONFIG *config_next = config->next;
        remove_config(config);
        config = config_next;
    }
}


// LOAD A GIVEN FILE BY THE USER INTO THE BUFFER OF THE EDITOR
void editor_load_file(EDITOR *editor, char *filename) {
    size_t content_size = 0;

    char *content = get_file_content(filename, &content_size);

    for (size_t i = 0; i < content_size; i++) {
        editor_handle_event(editor, content[i]);
    }

    editor_render(editor);
    free(content);
}

static void editor_print_no_line(EDITOR *editor) {
    (void)editor;

    // SETTING THE COLOR
    wattron(stdscr, LINE_NUMBER_THEME);

    // PRINT THE NO ROW CHAR
    waddch(stdscr, '~');

    // RESET THE COLOR BACK
	wattroff(stdscr, LINE_NUMBER_THEME);
}

static void editor_print_line_number(EDITOR *editor, size_t row) {
    (void)editor;

    // SETTING THE COLOR
    wattron(stdscr, LINE_NUMBER_THEME);

    size_t printed_row = (editor->config.cursor.pos.row + 1 == row) ? row : (size_t) abs((int) (editor->config.cursor.pos.row + 1- row));

    // PRINT THE ROW
    int num_digits = count_num_digit(editor->config.buff.size);
    for (int i = 0; i < num_digits - count_num_digit(printed_row); i++) 
        waddch(stdscr, ' ');

    wprintw(stdscr, "%zu", printed_row);

    // RESET THE COLOR BACK
	wattroff(stdscr, LINE_NUMBER_THEME);
}

void editor_render(EDITOR *editor) {
    size_t height, width;
    getmaxyx(stdscr, height, width);
    (void)width;

    wclear(stdscr);
    move(0, 0);

    ROW *current = editor->config.buff.rows;
    size_t row = 0, col = 0;

    int left_space = 0;

    // SET THE LEFT SPACE AFTER PRINTING THE NO LINE CHAR
    if (editor->config.buff.size == 0) 
        left_space = 2;

    while (current) {
        size_t index = 0;
        col = 0;

        editor_print_line_number(editor, row + 1);

        // COUNT THE SPACE TO MOVE THE CURSOR THERE AFTER PRINTTING THE LINE NUMBER
        left_space = count_num_digit(editor->config.buff.size) + 2;

        // MOVE THE CURSOR THERE
        col += count_num_digit(editor->config.buff.size) + 2;

        while(index < current->size) {
            move(row, col++);
            waddch(stdscr, current->content[index++]); 
        }

        waddch(stdscr, NEW_LINE_CHAR);
        row++;
        current = current->next;
    }

    while (row < height) {
        col = 0;
        move(row, col);
        editor_print_no_line(editor);
        waddch(stdscr, NEW_LINE_CHAR);
        move(row, col++);
        row++;
    }

    move(editor->config.cursor.pos.row, editor->config.cursor.pos.col + left_space);
    wrefresh(stdscr);
    editor_print_meta_data_on_bottom_window(editor);
}


// IMPLEMENTATION OF THE UNDO OPERATION
static ROW *copy_row(ROW *row) {
    if (row == NULL) return NULL;

    (void)editor;
    ROW *result = (ROW *)malloc(sizeof(ROW));
    
    *result = (ROW) {
        .size = row->size,
        .cap = row->cap,
    };

    result->content = (char *)malloc(sizeof(char) * row->cap);
    result->content = memcpy(result->content, row->content, sizeof(char) * row->size);

    return result;
}

static ROW *get_current_rows(ROW *all_rows) {
    ROW *rows = NULL;
    ROW *current = all_rows;

    ROW *prev_row = NULL;
    ROW *curr_row = NULL;

    while (current) {
        // COPY THE CURRENT ROW
        curr_row = copy_row(current);

        // LINK THE PREVIOUS ROW WITH THE CURRENT ONE
        if (prev_row) prev_row->next = curr_row;
        else 
            // IF NO PREV ROW IS THERE THAN MAKE THE HEAD OF THE ROWS
            rows = curr_row;

        // LINK THE CURRENT ROW WITH THE PREV ONE
        curr_row->prev = prev_row;

        // ADVANCE
        prev_row = curr_row;

        current = current->next;
    }

    if (curr_row) curr_row->next = NULL;

    return rows;
}

static BUFFER get_current_buffer(BUFFER buffer, size_t current_row_position) {
    BUFFER buff;

    buff.size = buffer.size;
    buff.rows = get_current_rows(buffer.rows);
    
    // GET THE CURRENT ROW BASED ON THE CURSOR POSITION
    ROW *current = buff.rows;
    size_t row = 0;
    while (row != current_row_position) {
        if (current == NULL) {
            printf("ERROR IN THE EDITOR GET CURRENT BUFFER (CURRENT ROW SUPPOSED NOT NULL)\n");
            exit(1);
        }
        row++;
        current = current->next;
    } 
    
    // SET THE CURRENT ROW
    buff.current_row = current;

    // SEARCH FOR THE TAIL OF THE ROWS

    // IF CURRENT IS NULL => CASE WHERE THE EDITOR IS NOW INITIALIZED
    if (current == NULL) buff.tail = current;
    else {
        while (current->next) {
            current = current->next;
        }
    }   

    // SET THE TAIL ROW
    buff.tail = current;

    return buff;
}

static EDITOR_CONFIG get_current_config(EDITOR_CONFIG curr_config) {
    EDITOR_CONFIG config;

    // COPY THE CURSOR AND MODE AND STATE DATA
    config = (EDITOR_CONFIG) {
        .cursor = curr_config.cursor,
        .mode = curr_config.mode,
        .state = curr_config.state,
    };

    // COPY THE CURRENT BUFFER
    config.buff = get_current_buffer(curr_config.buff, curr_config.cursor.pos.row);

    // COPY THE CURRENT FILE NAME
    if (curr_config.FILE_NAME == NULL) {
        config.FILE_NAME = NULL;
    } else {
        config.FILE_NAME = (char *)malloc(sizeof(char) * (MAX_INPUT_SIZE + 1));
        config.FILE_NAME = memcpy(config.FILE_NAME, curr_config.FILE_NAME, MAX_INPUT_SIZE);
    }

    config.next = NULL;

    return config;
}

static EDITOR_CONFIG editor_get_current_config(EDITOR *editor) {
    return get_current_config(editor->config);
}

static void editor_push_current_config_to_stack(EDITOR *editor) {
    EDITOR_CONFIG curr_config = editor_get_current_config(editor);
    EDITOR_CONFIG *config = (EDITOR_CONFIG *)malloc(sizeof(EDITOR_CONFIG));
    *config = curr_config;

    // PUSH THE CURRENT CONFIG TO THE STACK
    config->next = editor->snapshots;
    editor->snapshots = config;
}

static EDITOR_CONFIG editor_pop_config(EDITOR *editor) {
    // GET THE TOP OF THE STACK CONFIGURATION
    EDITOR_CONFIG prev_config = *(editor->snapshots);

    EDITOR_CONFIG *next = editor->snapshots->next;
    
    // FREE THE POINTER TO THE NEXT CONFIG
    free(editor->snapshots);
    editor->snapshots = next;

    return prev_config;
}

static void editor_undo(EDITOR *editor) {
    if(editor->snapshots == NULL) return;

    // FREE ALL THE CONFIG 
    editor_remove_config(editor);


    editor->config = editor_pop_config(editor);
}

void editor_save_primary_snapshot(EDITOR *editor) {
    editor_push_current_config_to_stack(editor);
}



#endif