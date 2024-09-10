#define ENV_IMPL
#define UTILS_IMPL
#include "headers/editor.h"

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

// // col for the start column of the rendering (for scrolling)
// // w for the window width
// void line_render(Line *line, size_t row, size_t col, int w, WINDOW *w) {
//     size_t i = col;

//     while(true) {
//         if ((i >= col + w) || (i >= line->count)) { return; }
//         wmove(w, (int)row, (int)(i - col));
//         waddch(w, line->content[i]);
//         ++i;
//     }
// }

void editor_update(Editor *e) {
    int c = getch();

    switch(c) {
        case 'q':
            e->state = STOPED;
            return;
        case KEY_LEFT:  
            int w = getmaxx(stdscr);
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

    editor_store_in_file(&e, "./f");
    
    editor_clean(&e);
    ncurses_quit();
}