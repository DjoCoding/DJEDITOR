#define ENV_IMPL
#include "headers/editor.h"

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

// col for the start column of the rendering (for scrolling)
// w for the window width
void line_render(Line *line, size_t row, size_t col, int w) {
    size_t i = col;

    while(true) {
        if ((i >= col + w) || (i >= line->count)) { return; }
        mvaddch((int) row, (int) (i - col), line->content[i]);
        ++i;
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
        case KEY_BACKSPACE:
            return editor_remove_text_before_cursor(e, sizeof(char));
        case '\n':
            editor_insert_line_after_cursor(e);
            return;
        default:
            editor_insert_text_after_cursor(e, (char *) &c, sizeof(char));
    }
}

void editor_render_line(Editor *e, size_t which_line, size_t row, int w) {
    Line *line = &e->b.lines[which_line];
    line_render(line, row, e->render_col, w);
}

void editor_render(Editor *e) {
    clear();

    int w = 0;
    int h = 0;

    getmaxyx(stdscr, h, w);

    if (e->cursor_col + 1 >= w) {
        e->cursor_col -= 1;
        e->render_col += 1;    
    }

    if (e->cursor_col < 3 && e->render_col > 0) {
        e->cursor_col += 1;
        e->render_col -= 1;
    }

    if (e->cursor_row + 1 >= h) {
        e->cursor_row -= 1;
        e->render_row += 1;
    }

    if (e->cursor_row < 3 && e->render_row > 0) {
        e->cursor_row += 1;
        e->render_row -= 1;
    }


    size_t i = e->render_row;
    bool render = true;

    while(true) {
        if ((i >= e->render_row + h) || (i >= e->b.count)) { break; }
        editor_render_line(e, i, i - e->render_row, w);
        ++i;
    }

    move(e->cursor_row, e->cursor_col);

    refresh();
}


size_t fsize(FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

char *fcontent(char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        DJ_ERROR("could not open the file %s: %s\n", filepath, strerror(errno));
    }

    size_t filesize = fsize(f);
    char *content = DJ_ALLOC((filesize + 1) * sizeof(char));
    size_t size = fread(content, sizeof(char), filesize, f);

    if (size != filesize) { 
        DJ_ERROR("could not read the whole file %s", filepath);
    }

    content[filesize] = 0;
    fclose(f);

    return content;
}

void editor_load_file(Editor *e, char *filepath) {
    char *content = fcontent(filepath);
    size_t size = strlen(content);
    
    char c = 0;
    char *begin = content;
    char *current = begin;

    while ((c = *current) != 0) {
        ++current;
        if (c == '\n') {
            editor_insert_text_after_cursor(e, begin, current - begin - 1);
            editor_insert_line_after_cursor(e);
            begin = current;
        }
    }

    editor_insert_text_after_cursor(e, begin, current - begin);
    
    e->cursor_col = 0;
    e->cursor_row = 0;
}

void editor_store_in_file(Editor *e, char *filepath) {
    FILE *f = fopen(filepath, "w");
    if (!f) {
        DJ_ERROR("could not open the file %s: %s\n", filepath, strerror(errno));
    }

    for (size_t i = 0; i < e->b.count; ++i) {
        Line *line = &e->b.lines[i];
        fwrite(line->content, sizeof(char), line->count, f);
        if (i != e->b.count - 1) { fputc('\n', f); }
    }

    fclose(f);
}

int main(void) {
    e = editor_init();
    e.state = RUNNING;

    editor_load_file(&e, "./test");
    
    ncurses_init();

    // initial render 
    editor_render(&e);
    
    while (e.state != STOPED) {
        editor_update(&e);
        editor_render(&e);
    }

    editor_store_in_file(&e, "./f");

    ncurses_quit();
}