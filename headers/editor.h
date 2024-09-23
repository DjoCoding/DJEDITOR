#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "color.h"
#include "command.h"
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
    LINE_NUMBER_SCREEN,
    SCREENS_COUNT,
} Editor_Screen;

typedef struct {
    Buffer b;
    Color_Buffer cb;
    uVec2 cursor; 
    iVec2 camera;
    size_t cmd_cursor;
    State state;
    Mode mode;
    Screen screens[SCREENS_COUNT];
    Command_History hist;
    Line cmd_line;
} Editor;

Editor editor_init(void);

void editor_insert_text_after_cursor(Editor *e, char *text, size_t text_size);
void editor_insert_line_after_cursor(Editor *e);
void editor_remove_text_before_cursor(Editor *e, size_t text_size);
void editor_push_line(Editor *e);
void editor_move_left_text(Editor *e);
void editor_move_right_text(Editor *e);
void editor_move_up_text(Editor *e);
void editor_move_down_text(Editor *e);


void editor_insert_command_text(Editor *e, char *cmd, size_t cmd_size);
void editor_remove_command_text(Editor *e, size_t cmd_size);
void editor_remove_command(Editor *e);
void editor_exec_command(Editor *e);
void editor_push_command_to_history(Editor *e);
void editor_move_up_command(Editor *e);
void editor_move_down_command(Editor *e);
void editor_move_left_command(Editor *e);
void editor_move_right_command(Editor *e);
void editor_exec_last_command(Editor *e);

void editor_dump(FILE *f, Editor *e);

void editor_load_file(Editor *e, char *filepath);
void editor_store_in_file(Editor *e, char *filepath);

int editor_find_text(Editor *e, char *text, size_t text_size, uVec2 *pos);
void editor_replace_text(Editor *e, char *text, size_t text_size, size_t _size, uVec2 pos);

void editor_shape_color_buffer(Editor *e);

void editor_set_default_color(Editor *e);
void editor_render(Editor *e);


void editor_clean(Editor *e);


#endif