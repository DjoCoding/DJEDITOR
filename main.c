#define ENV_IMPL
#define UTILS_IMPL
#define SV_IMPL

#include "headers/editor.h"

#define ESC 27

Editor e = {0};

// ncurses initialization and cleaning 
void ncurses_init(void) {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
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
            case 'l': return editor_move_down(e);
            case 'm': return editor_move_right(e);
            case 'o': return editor_move_up(e);
            case 'k': 
                int w = e->screens[MAIN_SCREEN].config.w;
                return editor_move_left(e, w);
            default: 
                return;
        }
    }

    if (e->mode == COMMAND) {
        if (c == '\n') { 
            editor_exec_command(e);
            e->mode = NORMAL;
            return;
        }
        
        if (c == ESC) { editor_remove_command(e); e->mode = NORMAL; return; }
        
        return editor_insert_command_text(e, (char *) &c, sizeof(char));
    }

    switch(c) {
        case KEY_LEFT:  
            int w = e->screens[MAIN_SCREEN].config.w;
            return editor_move_left(e, w);
        case KEY_RIGHT:
            return editor_move_right(e);
        case KEY_UP:
            return editor_move_up(e);
        case KEY_DOWN:
            return editor_move_down(e);
        case KEY_BACKSPACE:
            return editor_remove_text_before_cursor(e, sizeof(char));
        case '\n':
            editor_insert_line_after_cursor(e);
            return;
        case ESC:
            e->mode = NORMAL; return;
        default:
            editor_insert_text_after_cursor(e, (char *) &c, sizeof(char));
    }
}

int main(void) {
    ncurses_init();

    e = editor_init();
    e.state = RUNNING;

    editor_load_file(&e, "./test");

    // initial render 
    editor_render(&e);

    while (e.state != STOPED) {
        editor_update(&e);
        editor_render(&e);
    }
    
    editor_clean(&e);

    ncurses_quit();
}
