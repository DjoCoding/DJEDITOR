#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

#include "./types.h"
#include "./editor.h"

void editor_clean(EDITOR *editor) {
    free(editor->buffer.content);
    free(editor->arr.rows);
    free(editor->highlight.arr.tokens);

    for (int i = 0; i < WINDOWS_COUNT; i++) 
        delwin(editor->windows[i].wind);
}

int main2(void) {
    EDITOR editor = editor_init();
    editor_load_file(&editor);

    editor_set_rows(&editor);
    POSITION pos = editor_get_curs_position(&editor);

    editor_lex_row(&editor, 0, editor.buffer.count);

    for (size_t i = 0; i < editor.highlight.arr.count; i++) {
        STRING_VIEW sv = sv_init(editor.buffer.content + editor.highlight.arr.tokens[i].first, editor.highlight.arr.tokens[i].size);
        print_sv(&sv, 0, editor.highlight.arr.tokens[i].size);
        printf(" -> ");
        stringify(editor.highlight.arr.tokens[i].type);
        printf(" -> ");
        printf("%zu:%zu", editor.highlight.arr.tokens[i].first, editor.highlight.arr.tokens[i].size);
        printf("\n");
    }
}

int main(void) {
    ncurses_init();

    EDITOR editor = editor_init();

    editor_init_theme(&editor);

    editor_load_file(&editor);

    while (editor.state != EXITING) {
        int ch = getch();
        editor_event_handler(&editor, ch);
        editor_render(&editor);
    }

    editor_clean(&editor);
    ncurses_quit();
}