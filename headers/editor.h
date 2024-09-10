#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "utils.h"
#include "screen.h"
#include "../tools/sv.h"

typedef enum {
    STOPED = 0,
    RUNNING, 
} State;

typedef enum {
    NORMAL = 0,
    INSERT,
    COMMAND,
} Mode;

typedef enum {
    MAIN_SCREEN = 0,
    COMMAND_SCREEN,
    SCREENS_COUNT,
} Editor_Screen;

typedef struct {
    Buffer b;
    size_t cursor_row;
    size_t cursor_col;
    int render_row;
    int render_col;
    size_t cmd_cursor;
    State state;
    Mode mode;
    Screen screens[SCREENS_COUNT];
    Line cmd;
} Editor;

Editor editor_init(void);
void editor_push_line(Editor *e);
void editor_move_left(Editor *e, size_t w);
void editor_move_right(Editor *e);
void editor_move_up(Editor *e);
void editor_move_down(Editor *e);
void editor_insert_text_after_cursor(Editor *e, char *text, size_t text_size);
void editor_insert_line_after_cursor(Editor *e);
void editor_remove_text_before_cursor(Editor *e, size_t text_size);
void editor_insert_command_text(Editor *e, char *cmd, size_t cmd_size);
void editor_remove_command_text(Editor *e, size_t cmd_size);
void editor_remove_command(Editor *e);
void editor_dump(FILE *f, Editor *e);
void editor_load_file(Editor *e, char *filepath);
void editor_store_in_file(Editor *e, char *filepath);
void editor_exec_command(Editor *e);
void editor_render(Editor *e);
void editor_clean(Editor *e);


#endif