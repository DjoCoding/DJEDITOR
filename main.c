#define ENV_IMPL
#include "editor.h"

#include <ncurses.h>

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

void line_render(Line *line, size_t row) {
    for(size_t i = 0; i < line->count; ++i) {
        mvaddch((int) row, (int) i, line->content[i]);
    }
}

void editor_update(Editor *e) {
    int c = getch();
    switch(c) {
        case 'q':
            e->state = STOPED;
            return;
        case KEY_LEFT:  
            return editor_move_left(e);
        case KEY_RIGHT:
            return editor_move_right(e);
        case KEY_UP:
            return editor_move_up(e);
        case KEY_DOWN:
            return editor_move_down(e);
        case '\n':
            editor_insert_line_after_cursor(e);
            return;
        default:
            editor_insert_text_after_cursor(e, (char *) &c, sizeof(char));
    }
}

void editor_render(Editor *e) {
    for (size_t i = 0; i < e->b.count; ++i) {
        Line *line = &e->b.lines[i];
        line_render(line, i);
    }

    move(e->cursor_row, e->cursor_col);
}

int main2(void) {
    e = editor_init();
    editor_push_line(&e);
    editor_insert_text_after_cursor(&e, "djaoued", strlen("djaoued"));
    e.cursor_col = 2;
    editor_insert_line_after_cursor(&e);
    e.cursor_row -= 1;
    e.cursor_col = 1;
    editor_insert_line_after_cursor(&e);
    editor_dump(stdout, &e);
    editor_clean(&e);
}

int main(void) {
    e = editor_init();
    e.state = RUNNING;
    editor_push_line(&e);

    ncurses_init();

    while (e.state != STOPED) {
        editor_update(&e);

        clear();
        editor_render(&e);
        refresh();
    }

    ncurses_quit();
}