#ifndef DJEDITOR_H
#define DJEDITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
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
static void editor_resize_row(EDITOR *editor, ROW **row);
static void editor_resize_curr_row(EDITOR *editor);
static void editor_append_row_to_another(EDITOR *editor, ROW **dest, ROW **src);   
static void editor_add_new_row(EDITOR *editor);
static void row_remove(ROW **row);
// ADD THE CHARACTER TO THE EDITOR BUFFER
static void editor_handle_new_line_char(EDITOR *editor);
static void editor_handle_char(EDITOR *editor, int ch);
static void editor_shift_row_right(EDITOR *editor, ROW *row, size_t index);
static void editor_shift_row_left(EDITOR *editor, ROW *row, size_t index);
static void editor_buffer_add_char(EDITOR *editor, int ch);
// GETTING INPUT FUNCTIONS
static char *editor_get_input(EDITOR *editor, char *question, char *answer);
static void editor_get_file_name(EDITOR *editor);
// HANDLING THE ARROW KEYS FUNCTIONS
static void editor_go_left(EDITOR *editor);
static void editor_go_right(EDITOR *editor);
static void editor_go_down(EDITOR *editor);
static void editor_go_up(EDITOR *editor);
// REMOVE A CHAR FROM THE EDITOR BUFFER
static void editor_remove_char(EDITOR *editor);
// CHECK
static bool editor_has_no_snapshots(EDITOR *editor);
// RENDERING FUNCTIONS
static void editor_print_line_place_holder(EDITOR *editor);
static void editor_print_line_number(EDITOR *editor, size_t row);
void editor_render(EDITOR *editor);
// IMPLEMENTATION OF THE UNDO OPERATION
static ROW *copy_row(ROW *row);
static ROW *get_current_rows(ROW *all_rows);
static BUFFER get_current_buffer(BUFFER buffer, size_t current_row_position);
static EDITOR_CONFIG get_current_config(EDITOR_CONFIG curr_config); // THIS WILL COPY THE WHOLE CONFIG PASSED
static EDITOR_CONFIG editor_get_current_config(EDITOR *editor);
static void editor_push_current_config_to_snapshots_stack(EDITOR *editor);
static EDITOR_CONFIG editor_pop_config(EDITOR *editor);
void editor_save_primary_snapshot(EDITOR *editor);
static void editor_undo(EDITOR *editor);                 // MAIN FUNCTION
// IMPLEMENTATION OF THE SEARCH OPERATION
static size_t editor_search_in_row(EDITOR *editor, ROW *row, char *input, int input_size);
static void editor_search(EDITOR *editor, size_t row_number, ROW *start_searching_row, char *input);
static void editor_search_input_after_cursor(EDITOR *editor);
static void editor_handle_search(EDITOR *editor);         // MAIN FUNCTION
// IMPLEMENTATION OF THE VISUAL MODE
static POSITION editor_get_current_cursor_position(EDITOR *editor);
static void editor_make_points_in_order(EDITOR *editor);
static void editor_set_start_visual_pos(EDITOR *editor);
static void editor_set_end_visual_pos(EDITOR *editor);
static void editor_delete_between_two_pos(EDITOR *editor);
static void editor_visual_delete(EDITOR *editor);
// QUIT THE EDITOR
static void editor_remove_config(EDITOR *editor);
static void editor_remove_snapshots(EDITOR *editor);
void editor_quit();

#define DJEDITOR_IMPLEMENTATION

// ROWS FUNCTIONS
static bool editor_check_curr_row_full(EDITOR *editor) {
    return (editor->config.buff.current_row->size == editor->config.buff.current_row->cap);
}

static void editor_resize_row(EDITOR *editor, ROW **row) {
    (void)editor;
    // DOUBLE THE CAPACITY OF THE CURRENT ROW 
    (*row)->cap *= 2;

    // REALLOCATE MEMORY FOR IT
    (*row)->content = (char *)realloc((*row)->content, (*row)->cap);
}

static void editor_resize_curr_row(EDITOR *editor) {
    editor_resize_row(editor, &(editor->config.buff.current_row));
}

static void editor_append_row_to_another(EDITOR *editor, ROW **dest, ROW **src) {   
    // JUST IN ORDER TO FOLLOW THE PARAMETER CONVENTIONS 
    (void) editor;

    while ((*dest)->size + (*src)->size > (*dest)->cap) {
        editor_resize_row(editor, dest);
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
    ROW *new_row = row_init();
    
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

static void editor_copy_after_cursor_to_next_row(EDITOR *editor) {
    size_t col = editor->config.cursor.pos.col;

    ROW *curr_row = editor->config.buff.current_row;
    ROW *next_row = curr_row->next;

    while (col + next_row->size < curr_row->size) {
        next_row->content[next_row->size] = curr_row->content[col + next_row->size]; 
        next_row->size++;
    }

    curr_row->size = col;

    // CHANGE THE CURSOR POSITION WITH THE CURRENT ROW
    editor->config.cursor.pos.row++;
    editor->config.cursor.pos.col = 0;
    editor->config.buff.current_row = next_row;
    return;
}

// THIS FUNCTION WILL HANDLE THE CASE WHERE THE USER PRESS ENTER ON THE A LINE (MORE CHARS AFTER THE CURSOR POSITION)
static void editor_handle_new_line_char(EDITOR *editor) {
    editor_push_current_config_to_snapshots_stack(editor);
    editor_add_new_row(editor);
    editor_copy_after_cursor_to_next_row(editor);
    return; 
}


// THE SHIFT OPERATION ON A ROW
static void editor_shift_row_right(EDITOR *editor, ROW *row, size_t index) {
    (void)editor;
    size_t i = row->size;
    while (i > index) {
        row->content[i] = row->content[i - 1];
        i--;
    }
}

static void editor_shift_row_left(EDITOR *editor, ROW *row, size_t index) {
    size_t i = index;
    while (i < row->size) {
        row->content[i] = row->content[i + 1];
        i++;
    };
    (void)editor;
}

// ADDING A CHAR IN THE EDITOR BUFFER
static void editor_buffer_add_char(EDITOR *editor, int ch) {
    ROW *curr_row = editor->config.buff.current_row;

    // IF THE CHAR IS A NEW LINE CHAR ('\n') THEN WE ADD A NEW ROW IN THE BUFFER
    if(ch == NEW_LINE_CHAR) {
        editor_handle_new_line_char(editor);
        return;
    }

    // GET THE SNAPSHOT OF THE CURRENT CONFIG
    if (ch == '\t' || ch == ' ') editor_push_current_config_to_snapshots_stack(editor);
    
    if (editor_has_no_snapshots(editor)) {
        editor_push_current_config_to_snapshots_stack(editor);
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
    editor_shift_row_right(editor, curr_row, editor->config.cursor.pos.col);


    // PUT THE CHARACTER IN ITS PLACE
    curr_row->content[editor->config.cursor.pos.col] = ch;


    // INCREMENT THE CURRENT ROW SIZE
    curr_row->size++;


    // MOVE THE CURSOR TO THE RIGHT
    editor_go_right(editor);
    
    // ADD MACROS LATER
    // if (ch == '(' || ch == '{') {
    //     int ch_rev;
    //     switch (ch) {
    //         case '(':
    //             ch_rev = ')';
    //             break;
    //         case '{':
    //             ch_rev = '}';
    //             break;
    //     }

    //     editor_buffer_add_char(editor, ch_rev);
    //     editor_go_left(editor);
    //     return;
    // }
} 

// READ A STRING IN THE BOTTOM WINDOW 
// (NEED TO HANDLE THE BACK SPACE KEY AND IGNORE THE TABS AND THE NEW LINE CHARS)
static char *editor_get_input(EDITOR *editor, char *question, char *answer) {
    // CLEAR THE BOTTOM WINDOW
    wclear(editor->windows[INPUT_WINDOW].wind);
    

    // MOVE THE CURSOR TO THE WINDOW
    wmove(editor->windows[INPUT_WINDOW].wind, 0, 1);

    // ASK FOR INPUT
    mvwprintw(editor->windows[INPUT_WINDOW].wind, 0, 0, "%s", question);
    
    // APPLY CHANGES
    wrefresh(editor->windows[INPUT_WINDOW].wind);
    
    size_t input_size = 0;
    int ch;
    
    while (input_size < MAX_INPUT_SIZE) {
        ch = getch();
        if (ch == NEW_LINE_CHAR) break;
        
        // HANDLE THE BACK SPACE INSIDE THE BOTTOM WINDOW
        if (ch == KEY_BACKSPACE) {
            if (input_size > 0) {
                input_size--;
                size_t row, col;
                getyx(editor->windows[INPUT_WINDOW].wind, row, col);
                wmove(editor->windows[INPUT_WINDOW].wind, row, col - 1);
                wdelch(editor->windows[INPUT_WINDOW].wind);
            } else {
                
            }
        } else {
            answer[input_size++] = ch;
            // PRINT THE CHAR ENTERED TO THE SCREEN
            waddch(editor->windows[INPUT_WINDOW].wind, ch);
        }
        wrefresh(editor->windows[INPUT_WINDOW].wind);
    }

    // MOVE THE CURSOR BACK TO ITS ORIGINAL PLACE
    wmove(editor->windows[MAIN_WINDOW].wind, editor->config.cursor.pos.row, editor->config.cursor.pos.col);
    
    // SET THE NULL TERMINATOR 
    answer[input_size] = NULL_TERMINATOR;

    return answer;
}



static void editor_get_file_name(EDITOR *editor) {
    char input[MAX_INPUT_SIZE + 1] = {0};

    do {
        editor_get_input(editor, "type file name: ", input);
    } while (input[0] == NULL_TERMINATOR);
    
    editor->config.FILE_NAME = (char *)malloc(sizeof(char) * (MAX_INPUT_SIZE + 1));
    memcpy(editor->config.FILE_NAME, input, MAX_INPUT_SIZE + 1);
}

// SAVE THE CURRENT BUFFER OF THE EDITOR IN A FILE
void editor_save_in_file(EDITOR *editor) {
    FILE *fp = fopen(editor->config.FILE_NAME, "w");
    ROW *current = editor->config.buff.rows;
    
    while (current != NULL) {
        fwrite(current->content, sizeof(char) * current->size, 1, fp);
        fprintf(fp, "%c", NEW_LINE_CHAR);
        current = current->next;
    }
    
    fclose(fp);
}

// ADD IT TO THE BUFFER AT THE CURRENT POSITION OF THE CURSOR
// UPDATE THE CURSOR 
// IF THE CHAR IS THE NEW LINE CHAR THEN UPDATE THE BUFFER LINES
// UPDATE THE MODE IF WE'RE AT THE NORMAL MODE

static void editor_handle_char(EDITOR *editor, int ch) {
    if (editor->config.mode == NORMAL) {
        if (ch == 'i') {
            // SWITCH THE MODE TO INSERT
            editor->config.mode = INSERT;
        } else if (ch == ':') {
            // HANDLE THE COMMANDS GIVEN
        } else if (ch == 'v') {
            editor->config.mode = VISUAL;
            // SET THE START POINT
            editor->visual.start = editor_get_current_cursor_position(editor);
        }
    } else if (editor->config.mode == VISUAL) {
        if (ch == 'd') {
            editor->visual.end = editor_get_current_cursor_position(editor);
            editor_visual_delete(editor);
            editor->config.mode = NORMAL;
        } else if (ch == 'i') {
            editor->config.mode = INSERT;
        } else if (ch == 'n') {
            editor->config.mode = NORMAL;
        }
    } else if (editor->config.mode == INSERT) {
        if (ch == CTRL('s')) {
            // ASK THE USER FOR THE NAME OF THE FILE IF IT'S NOT PROVIDJEDITOR AT THE COMMAND LINE
            if (editor->config.FILE_NAME == NULL) {
                editor_get_file_name(editor);
            }
            // OPEN A NEW FILE AND COPY THE BUFFER THERE
            editor_save_in_file(editor);
        }
        else if (ch == CTRL('f')) {
            // SEARCH FOR A STRING IN WHOLE FILE (FROM TOP TO BOTTOM)
            editor_handle_search(editor);
        }  else if (ch == CTRL('d')) {
            // SEARCH FOR A STRING IN NEXT ROWS
            editor_search_input_after_cursor(editor);
        } else {            
                // PUT THE NEW CHAR AT PLACE TO RENDER AFTER
                editor_buffer_add_char(editor, ch);
            }
    }
}

// ADDITIONAL FUNCTIONS
static char *editor_stringify_state(EDITOR *editor) {
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

static char *editor_stringify_mode(EDITOR *editor) {
    switch (editor->config.mode) {
        case NORMAL:
            return "NORMAL";
        case INSERT:
            return "INSERT";
        case VISUAL:    
            return "VISUAL";
        default:
            return "NORMAL";
    }
}

void editor_print_meta_data_on_bottom_window(EDITOR *editor) {
    
    // CLEARING THE WINDOW 
    wclear(editor->windows[STATUS_WINDOW].wind);
    
    // GETTING THE WINDOW SIZE
    size_t window_height, window_width;

    window_height = editor->windows[STATUS_WINDOW].win_height;
    window_width = editor->windows[STATUS_WINDOW].win_width;

    // GETTING THE CURSOR POSITION
    size_t row = editor->config.cursor.pos.row, col = editor->config.cursor.pos.col;

    // SETTING BOLD FONT
    // wattron(editor->windows[STATUS_WINDOW].wind, A_BOLD);

    // size_t row = window_height * 0.95;

    // PRINTING...
    mvwprintw(editor->windows[STATUS_WINDOW].wind, 1, 0, "%s", editor_stringify_mode(editor));
    // mvwprintw(editor->windows[MAIN_WINDOW].wind, row, 1, "%s", editor_stringify_mode(editor));


    // GO TO THE CENTER
    mvwprintw(editor->windows[STATUS_WINDOW].wind, 1, (window_width - 15) / 2, "%.3zu : %.3zu", row + 1, col + 1);
    // mvwprintw(editor->windows[MAIN_WINDOW].wind, row, (window_width - 15) / 2, "%.3zu : %.3zu", editor->config.cursor.pos.row + 1, editor->config.cursor.pos.col + 1);


    // MAKE THE EDITOR STATE BLINKING
    wattron(editor->windows[STATUS_WINDOW].wind, A_BLINK);
    // wattron(editor->windows[MAIN_WINDOW].wind, A_BLINK);

    // GO TO END OF LINE
    mvwprintw(editor->windows[STATUS_WINDOW].wind, 1, (window_width - 10), "%s", editor_stringify_state(editor));
    // mvwprintw(editor->windows[MAIN_WINDOW].wind, row, (window_width - 10), "%s", editor_stringify_state(editor));

    // DISABLE THE BLIKING ATTRIBUTE
    wattroff(editor->windows[STATUS_WINDOW].wind, A_BLINK);
    // wattroff(editor->windows[MAIN_WINDOW].wind, A_BLINK);


    // SETTING OFF THE BOLD FONT
    // wattroff(editor->windows[STATUS_WINDOW].wind, A_BOLD);

    // PRINT THE CURRENT TIME ON THE SCREEN
    // time_t tm;
    // time(&tm);
    // mvwprintw(editor->windows[STATUS_WINDOW].wind, 0, (window_width + 15) / 2, "%s", ctime(tm));

    // APPLY CHANGES ON THE WINDOW
    wrefresh(editor->windows[STATUS_WINDOW].wind);
    // wrefresh(waddch(editor->windows[MAIN_WINDOW].wind);

    (void)window_width;
    (void)window_height;
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

    ROW *curr_row = editor->config.buff.current_row;
    if (curr_row->size == 0) {
        if (curr_row->prev == NULL) return;
        
        editor_go_left(editor);
        
        // LINK THE CURRENT ROW (AFTER GOING LEFT) WITH THE NEXT ONE
        ROW *next = curr_row->next;
        editor->config.buff.current_row->next = next;

        row_remove(&(curr_row));
        return;
    }

    // UPDATE THE CURSOR POSITION
    editor_go_left(editor);

    // SHIFT ALL THE BUFFER ONCE TO THE LEFT
    editor_shift_row_left(editor, curr_row, editor->config.cursor.pos.col);


    // SEE ABOVE THAT I MOVED THE CURSOR TO THE LEFT 
    // THAT MEANS IF THE CURSOR POSITION IS THE SAME AS THE SIZE OF THE CURRENT ROW
    // THAT MEANS THE PREVIOUS POSITION OF THE CURSOR WAS AT POSITION 0
    
        // LATER I GOTTA MOVE ALL THE NEXT LINE (OF THE CURRENT ROW) TO THE PREV ROW AT THE END
    if (editor->config.cursor.pos.col != curr_row->size) curr_row->size--;
    else 
        // IF WE ENCOUNTERED THE SITUATION ABOVE WE APPEND THE CURRENT ROW WITH IT'S PREVIOUS
        editor_append_row_to_another(editor, &curr_row, &(curr_row->next));
}


// THE EDITOR EVENT HANDLER
void editor_handle_event(EDITOR *editor, int ch) {
    switch (ch) {
        case CTRL('c'):
            editor->config.state = EXIT;
            break;
        case CTRL('n'):
            editor->config.mode = NORMAL;
            break;
        case CTRL('v'):
            editor->config.mode = VISUAL;
            editor_set_start_visual_pos(editor);
            break;
        case CTRL('i'):
            editor->config.mode = INSERT;
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
            editor_handle_char(editor, ch);
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
    // REMOVE THE CURRENT CONFIG OF THE EDITOR
    editor_remove_config(&editor);

    // REMOVE ALL THE AVAILABEL SNAPSHOTS
    editor_remove_snapshots(&editor);

    // DELETE ALL THE WINDOWS
    for (int i = 0; i < WINDOW_COUNT; i++)  
        delwin(editor.windows[i].wind);
        
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
}

static void editor_remove_snapshots(EDITOR *editor) {
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
        printf("%c", content[i]);
        editor_handle_event(editor, content[i]);
    }

    free(content);
}

static void editor_print_line_place_holder(EDITOR *editor) {
    (void)editor;

    // SETTING THE COLOR
    wattron(editor->windows[MAIN_WINDOW].wind, LINE_NUMBER_THEME);

    // PRINT THE NO ROW CHAR
    waddch(editor->windows[MAIN_WINDOW].wind, '~');

    // RESET THE COLOR BACK
	wattroff(editor->windows[MAIN_WINDOW].wind, LINE_NUMBER_THEME);
}

static void editor_print_line_number(EDITOR *editor, size_t row) {
    (void)editor;

    // SETTING THE COLOR
    wattron(editor->windows[MAIN_WINDOW].wind, LINE_NUMBER_THEME);

    size_t printed_row = (editor->config.cursor.pos.row + 1 == row) ? row : (size_t) abs((int) (editor->config.cursor.pos.row + 1 - row));

    // PRINT THE ROW
    int num_digits = count_num_digit(editor->config.buff.size);
    for (int i = 0; i < num_digits - count_num_digit(printed_row); i++) 
        waddch(editor->windows[MAIN_WINDOW].wind, ' ');

    wprintw(editor->windows[MAIN_WINDOW].wind, "%zu", printed_row);

    // RESET THE COLOR BACK
	wattroff(editor->windows[MAIN_WINDOW].wind, LINE_NUMBER_THEME);
}

static ROW *editor_get_row_by_row_number(EDITOR *editor, size_t row_number) {
    size_t i = 0;
    ROW *current = editor->config.buff.rows;

    while ((current) && (i < row_number)) {
        current = current->next;
        i++;
    }

    return current;
}

static void editor_set_row_render_start(EDITOR *editor, WINDOW_TYPE window_type) {
    if (editor->windows[window_type].renderer.row_start > editor->config.cursor.pos.row)
        editor->windows[window_type].renderer.row_start =  editor->config.cursor.pos.row;

    if (editor->windows[window_type].renderer.row_start + editor->windows[window_type].win_height - 1 < editor->config.cursor.pos.row) 
        editor->windows[window_type].renderer.row_start =  editor->config.cursor.pos.row - editor->windows[window_type].win_height + 1;
}

static void editor_set_col_render_start(EDITOR *editor, WINDOW_TYPE window_type) {
    if (editor->windows[window_type].renderer.col_start > editor->config.cursor.pos.col) 
        editor->windows[window_type].renderer.col_start =  0;

    if (editor->windows[window_type].renderer.col_start + editor->windows[window_type].win_width < editor->config.cursor.pos.col) 
        editor->windows[window_type].renderer.col_start =  editor->config.cursor.pos.col - editor->windows[window_type].win_width;
}

static bool editor_if_position_selected(EDITOR *editor, size_t row, size_t col) {

    if (editor->config.mode != VISUAL) return false;


    // GET THE POSITIONS
    POSITION visual_position = editor->visual.start;
    POSITION curs_position = editor->config.cursor.pos;

    // MAKE THE SWAP TO COMPARE EASILY (visual_position < curs_position)
    if (visual_position.row > curs_position.row) {
        swap(&visual_position.row, &curs_position.row);
    }

    if (visual_position.row == curs_position.row) 
        if (visual_position.col > curs_position.col) {
            swap(&visual_position.col, &curs_position.col);
        }
    

    if (row < visual_position.row) return false;
    if (row > curs_position.row) return false;

    if (row == visual_position.row && col < visual_position.col) return false;
    if (row == curs_position.row && col >= curs_position.col) return false;

    return true;   
}

static size_t editor_render_visual_mode(EDITOR *editor, ROW *start_row, size_t left_space) {
    size_t row = 0, col = 0;

    ROW *current = start_row;

    size_t row_start = editor->windows[MAIN_WINDOW].renderer.row_start;
    size_t col_start = editor->windows[MAIN_WINDOW].renderer.col_start;

    while (current  && (row < editor->windows[MAIN_WINDOW].win_height)) {
        size_t index = 0;
        col = 0;

        editor_print_line_number(editor, row_start + row + 1);

        // MOVE THE CURSOR THERE
        col += left_space;


        while(col_start + index < current->size) {
            // HIGHLIGHT THE SELECTED TEXT
            if (editor_if_position_selected(editor, row_start + row, col_start + col - left_space)) 
                wattron(editor->windows[MAIN_WINDOW].wind, A_STANDOUT);
        
            wmove(editor->windows[MAIN_WINDOW].wind, row, col++);
            waddch(editor->windows[MAIN_WINDOW].wind, current->content[col_start + index]);
            wattroff(editor->windows[MAIN_WINDOW].wind, A_STANDOUT);
            
            index++; 
        }

        waddch(editor->windows[MAIN_WINDOW].wind, NEW_LINE_CHAR);
        row++;
        current = current->next;
    }

    return row;
}

// THE SAME FUNCTION AS THE VISUAL ONE BUT WITHOUT HIGHLIGHTIN THE TEXT
// RETURNS THE ROW WHERE IT FINISHED PRINTING
static size_t editor_render_insert_mode(EDITOR *editor, ROW *start_row, size_t left_space) {
    size_t row = 0, col = 0;

    ROW *current = start_row;

    size_t row_start = editor->windows[MAIN_WINDOW].renderer.row_start;
    size_t col_start = editor->windows[MAIN_WINDOW].renderer.col_start;

    while (current  && (row < editor->windows[MAIN_WINDOW].win_height)) {
        size_t index = 0;
        col = 0;

        editor_print_line_number(editor, row_start + row + 1);

        // MOVE THE CURSOR THERE
        col += left_space;

        while(col_start + index < current->size) {
            wmove(editor->windows[MAIN_WINDOW].wind, row, col++);
            waddch(editor->windows[MAIN_WINDOW].wind, current->content[col_start + index]);
            wrefresh(editor->windows[MAIN_WINDOW].wind);
            index++; 
        }

        waddch(editor->windows[MAIN_WINDOW].wind, NEW_LINE_CHAR);
        row++;
        current = current->next;
    }

    return row;
}

void editor_render(EDITOR *editor) {

    size_t height, width;

    height = editor->windows[MAIN_WINDOW].win_height;
    width = editor->windows[MAIN_WINDOW].win_width;
    
    wclear(editor->windows[MAIN_WINDOW].wind);
    move(0, 0);

    // GET THE ROW AND COL WHERE WE GOING TO START PRINTING THE BUFFER
    editor_set_row_render_start(editor, MAIN_WINDOW);
    editor_set_col_render_start(editor, MAIN_WINDOW);

    ROW *current = editor_get_row_by_row_number(editor, editor->windows[MAIN_WINDOW].renderer.row_start);

    size_t row = 0, col = 0;

    // COUNT THE SPACE TO MOVE THE CURSOR THERE AFTER PRINTTING THE LINE NUMBER
    int left_space = count_num_digit(editor->config.buff.size) + 2;

    if(editor->config.mode == VISUAL) 
        row = editor_render_visual_mode(editor, current, left_space);
    else if (editor->config.mode == NORMAL || editor->config.mode == INSERT) 
        row = editor_render_insert_mode(editor, current, left_space);

    mvwprintw(editor->windows[MAIN_WINDOW].wind, 10, 5, "%zu", editor->windows[MAIN_WINDOW].renderer.row_start);
    wrefresh(editor->windows[MAIN_WINDOW].wind);

    while (row < height) {
        col = 0;
        wmove(editor->windows[MAIN_WINDOW].wind, row, col);
        editor_print_line_place_holder(editor);
        waddch(editor->windows[MAIN_WINDOW].wind, NEW_LINE_CHAR);
        wrefresh(editor->windows[MAIN_WINDOW].wind);
        wmove(editor->windows[MAIN_WINDOW].wind, row, col++);
        row++;
    }

    move(editor->config.cursor.pos.row - editor->windows[MAIN_WINDOW].renderer.row_start, editor->config.cursor.pos.col + left_space - editor->windows[MAIN_WINDOW].renderer.col_start);
    editor_print_meta_data_on_bottom_window(editor);

    (void)width;
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

    for (size_t i = 0; i < row->size; i++) {
        result->content[i] = row->content[i];
    }
    // result->content = memcpy(result->content, row->content, sizeof(char) * row->size);

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
        config.FILE_NAME = (char *)memcpy(config.FILE_NAME, curr_config.FILE_NAME, MAX_INPUT_SIZE);
    }

    config.next = NULL;

    return config;
}

static EDITOR_CONFIG editor_get_current_config(EDITOR *editor) {
    return get_current_config(editor->config);
}

static void editor_push_current_config_to_snapshots_stack(EDITOR *editor) {
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
    if(editor_has_no_snapshots(editor)) return;

    // FREE ALL THE CONFIG 
    editor_remove_config(editor);


    editor->config = editor_pop_config(editor);
}

void editor_save_primary_snapshot(EDITOR *editor) {
    editor_push_current_config_to_snapshots_stack(editor);
}

static bool editor_has_no_snapshots(EDITOR *editor) {
    return (editor->snapshots == NULL);
}   


// IMPLEMENTATION OF THE SEARCH FUNCTIONALITY

// RETURN THE COLUMN + 1 IF THE STRING NOT FOUND ELSE RETURN 0
static size_t editor_search_in_row(EDITOR *editor, ROW *row, char *input, int input_size) {
    (void)editor;

    
    
    if (row->size < (size_t)input_size) return 0;

    size_t i = 0;
    bool isfound = false;

    while (i < row->size - (size_t)input_size + 1) {
        isfound = (strncmp((char *)(row->content + i), input, input_size) == 0);
        if (isfound) return (i + 1);
        i++;
    }
    return 0;
}

static void editor_search(EDITOR *editor, size_t row_number, ROW *start_searching_row, char *input) {
    ROW *current = start_searching_row;
    
    // MAKING IT GETTING THE ROW_NUMBER IS FOR MAKING THE CURSOR IN PLACE AFTER FINDING THE STRING
    size_t row = row_number;
    size_t col = 0;
    size_t input_size = strlen(input);

    while (current != NULL) {
        col = editor_search_in_row(editor, current, input, input_size);
        if (col != 0) break;
        row++;
        current = current->next;
    }

    if (col != 0) {
        // UPDATE THE CURSOR LOCATION
        editor->config.cursor.pos.row = row;
        editor->config.cursor.pos.col = col + input_size - 1;
        wmove(editor->windows[MAIN_WINDOW].wind, editor->config.cursor.pos.row, editor->config.cursor.pos.col);
        // UPDATE THE CURRENT ROW POINTER
        editor->config.buff.current_row = current;
    }
}

static void editor_search_input_after_cursor(EDITOR *editor) {
    char input[MAX_INPUT_SIZE + 1] = {0};
    do {
        editor_get_input(editor, "string: ", input);
    } while (input[0] == NULL_TERMINATOR);
    editor_search(editor, editor->config.cursor.pos.row + 1, editor->config.buff.current_row->next, input);
}


static void editor_handle_search(EDITOR *editor) {
    char input[MAX_INPUT_SIZE + 1] = {0};
    do {
        editor_get_input(editor, "string: ", input);
    } while (input[0] == NULL_TERMINATOR);
    editor_search(editor, 0, editor->config.buff.rows, input);
}


// VISUAL MODE

// RETURN THE CURRENT POSITION OF THE CURSOR
static POSITION editor_get_current_cursor_position(EDITOR *editor) {
    return editor->config.cursor.pos;
}

static void editor_set_start_visual_pos(EDITOR *editor) {
    editor->visual.start = editor_get_current_cursor_position(editor);
}

static void editor_set_end_visual_pos(EDITOR *editor) {
    editor->visual.end = editor_get_current_cursor_position(editor);
    editor->visual.end.col--;
}

// I ALWAYS WANT THE START POINT TO BE BEFORE THE END POINT
static void editor_make_points_in_order(EDITOR *editor) {
    if (editor->visual.start.row < editor->visual.end.row) return;
    if (editor->visual.start.row == editor->visual.end.row) 
        if (editor->visual.start.col < editor->visual.end.col) return;

    // THEN I HAVE TO SWAP THE ROWS AND THE COLS
    swap(&editor->visual.start.row, &editor->visual.end.row);
    swap(&editor->visual.start.col, &editor->visual.end.col);
}

// DELETE ALL THE CONTENT FROM THE START POINT TO THE END POINT
static void editor_delete_between_two_pos(EDITOR *editor) {
    // FIRST TAKE A SNAPSHOT
    editor_push_current_config_to_snapshots_stack(editor);

    // THE START AND END POINT ARE IN THE VISUAL OF THE EDITOR
    editor_make_points_in_order(editor);

    // THIS FUNCTION WILL HANDLE BOTH THE CASE WHERE THE START_ROW == END_ROW AND NOT
    size_t start_row = editor->visual.start.row;
    size_t end_row = editor->visual.end.row;
    size_t start_col = editor->visual.start.col;
    size_t end_col = editor->visual.end.col;

    // GET THE ROWS
    ROW *s_row = editor_get_row_by_row_number(editor, start_row);
    ROW *e_row;

    if (start_row == end_row) 
        e_row = s_row;
    else 
        e_row = editor_get_row_by_row_number(editor, end_row);

    if (start_row != end_row) {
        // FIRST DELETE ALL THE ROWS BETWEEN THE TWO ROWS
        ROW *current = s_row->next;

        // REMOVE THE ROWS BETWEEN THE TWO ROWS
        while(current != e_row) {
            ROW *next = current->next;
            row_remove(&current);
            current = next;
        }

        // LINK THE START ROW WITH THE END ROW
        s_row->next = current;
    }

    // CALCULATE THE SIZES OF THE START AND THE END ROWS AFTER REMOVING THE CHARS
    size_t e_row_size = e_row->size - end_col;
    size_t s_row_size = start_col;

    // SHIFT THE DATA
    strncpy(s_row->content + start_col, e_row->content + end_col, e_row_size);

    if (start_row != end_row) {
        s_row->next = e_row->next;
        row_remove(&e_row);
    }

    // SET THE NEW START ROW SIZE 
    s_row->size = e_row_size + s_row_size;


    // UPDATE THE CURSOR POSIITON
    editor->config.cursor.pos.col = start_col;
    editor->config.cursor.pos.row = start_row;

    // UPDATE THE CURRENT ROW
    editor->config.buff.current_row = s_row;
}

static void editor_visual_delete(EDITOR *editor) {
    editor_delete_between_two_pos(editor);
}


#endif