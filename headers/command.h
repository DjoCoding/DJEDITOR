#ifndef COMMAND_H
#define COMMAND_H

#include "line.h"

typedef struct {
    char *text;
    size_t size;
} Command;

typedef struct{
    Command *cmds;
    size_t count;
    size_t size;
    size_t current;
} Command_History;

#define COMMAND_HISTORY_INITIAL_SIZE 10

Command cmd_init(char *text, size_t _size);
void cmd_to_buffer_line(Command cmd, Line *line);
Command_History cmds_init();
void cmds_insert_cmd(Command_History *hist, char *cmd, size_t _size);
Command cmds_get_cmd(Command_History *hist);

#endif