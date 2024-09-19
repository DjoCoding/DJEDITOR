#define ENV_IMPL
#define UTILS_IMPL
#define SV_IMPL
#define UVEC2_IMPL
#define IVEC2_IMPL

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
            case 'l': return editor_move_down_text(e);
            case 'm': return editor_move_right_text(e);
            case 'o': return editor_move_up_text(e);
            case 'k': return editor_move_left_text(e);
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

int main2(void) {
    Line line = line_init();
    size_t cursor = 0;
    char *text = NULL;

    {
        text = "djaoued";
        line_insert_text_after_cursor(&line, text, strlen(text), &cursor);
    }

    {
        text = "f";
        line_replace_text(&line, text, strlen(text), strlen("djaoued"), 0);
    }

    line_dump(stdout, &line);
    fprintf(stdout, "\n");

    line_clean(&line);
    return 0;
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
