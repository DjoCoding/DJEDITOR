#define ENV_IMPL
#define UTILS_IMPL
#define SV_IMPL
#define UVEC2_IMPL
#define IVEC2_IMPL

#include "headers/editor.h"

#define ESC 27

Editor e = {0};

#define ARR_LEN(arr)    sizeof(arr)/sizeof((arr)[0])

int ncurses_color_table[] = {
    COLOR_BLACK, 
    COLOR_RED, 
    COLOR_GREEN, 
    COLOR_YELLOW, 
    COLOR_BLUE, 
    COLOR_MAGENTA, 
    COLOR_CYAN, 
    COLOR_WHITE
};


// ncurses initialization and cleaning 
void ncurses_init(void) {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
}

void ncurses_init_colors(void) {
    start_color();
    use_default_colors();

    size_t i = 0;
    while(i < ARR_LEN(ncurses_color_table)) {
        printf("%ld\n", i + BLACK_ON_DEFAULT);
        init_pair(i + BLACK_ON_DEFAULT, ncurses_color_table[i], -1);
        ++i;
    }

    for(size_t i = 1; i < ARR_LEN(ncurses_color_table); ++i) {
        init_pair(i + RED_ON_BLACK - 1, ncurses_color_table[i], COLOR_BLACK);
    }
}

void ncurses_quit(void) {
    endwin();
}

void editor_update(Editor *e) {
    int c = getch();

    if (e->mode == NORMAL) {
        switch(c) {
            case ':': 
                e->mode = COMMAND;
                editor_remove_command(e);
                return editor_insert_command_text(e, (char *) &c, sizeof(char));
            case 'i': e->mode = INSERT; return;
            case 'l': return editor_move_down_text(e);
            case 'm': return editor_move_right_text(e);
            case 'o': return editor_move_up_text(e);
            case 'k': return editor_move_left_text(e);
            case '\n': return editor_exec_last_command(e);
            default:  return;
        }
    }

    if (e->mode == COMMAND) {
        switch(c) {
            case '\n':
                editor_push_command_to_history(e);
                editor_exec_command(e);
                e->mode = NORMAL;
                return;
            case ESC:
                editor_remove_command(e);
                e->mode = NORMAL;
                return;    
            case KEY_LEFT:  
                return editor_move_left_command(e);
            case KEY_RIGHT:
                return editor_move_right_command(e);
            case KEY_UP:
                return editor_move_up_command(e);
            case KEY_DOWN:
                return editor_move_down_command(e);
            case KEY_BACKSPACE:
                return editor_remove_command_text(e, sizeof(char));
            default:
                return editor_insert_command_text(e, (char *) &c, sizeof(char));
        }        
    }

    switch(c) {
        case KEY_LEFT:  
            return editor_move_left_text(e);
        case KEY_RIGHT:
            return editor_move_right_text(e);
        case KEY_UP:
            return editor_move_up_text(e);
        case KEY_DOWN:
            return editor_move_down_text(e);
        case KEY_BACKSPACE:
            return editor_remove_text_before_cursor(e, sizeof(char));
        case '\n':
            return editor_insert_line_after_cursor(e);
        case ESC:
            e->mode = NORMAL; return;
        default:
            return editor_insert_text_after_cursor(e, (char *) &c, sizeof(char));
    }
}

int main(void) {
    ncurses_init();

    e = editor_init();

    #if 1   
        ncurses_init_colors();
    #endif
    
    editor_set_default_color(&e);

    e.state = RUNNING;
    
    while (e.state != STOPED) {
        editor_update(&e);
        editor_render_highlighted(&e);
    }
    
    editor_clean(&e);

    ncurses_quit();
}
