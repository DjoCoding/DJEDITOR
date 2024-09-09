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
        case KEY_BACKSPACE:
            return editor_remove_text_before_cursor(e, sizeof(char));
        case '\n':
            editor_insert_line_after_cursor(e);
            return;
        default:
            editor_insert_text_after_cursor(e, (char *) &c, sizeof(char));
    }
}

void editor_render(Editor *e) {
    clear();

    for (size_t i = 0; i < e->b.count; ++i) {
        Line *line = &e->b.lines[i];
        line_render(line, i);
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

    editor_load_file(&e, "./file.c");
    
    ncurses_init();

    // initial render 
    editor_render(&e);
    
    while (e.state != STOPED) {
        editor_update(&e);
        editor_render(&e);
    }

    editor_store_in_file(&e, "./a.out");

    ncurses_quit();
}