#include "headers/command.h"

Command cmd_init(char *text, size_t _size) {
    Command cmd = {0};
    cmd.size = _size;
    cmd.text = DJ_ALLOC(sizeof(char) * _size);
    memcpy(cmd.text, text, _size);
    return cmd;
}

Command_History cmds_init() {
    Command_History hist = {0};
    hist.size = COMMAND_HISTORY_INITIAL_SIZE;
    hist.cmds = DJ_ALLOC(sizeof(Command) * hist.size);
    return hist;
}

void cmds_resize(Command_History *hist) {
    hist->size *= 2;
    hist->cmds = DJ_REALLOC(hist->cmds, sizeof(Command), hist->size);
}

void cmds_insert_cmd(Command_History *hist, char *cmd_text, size_t _size) {
    Command cmd = cmd_init(cmd_text, _size);
    if (hist->count >= hist->size) { cmds_resize(hist); }
    hist->cmds[hist->count++] = cmd;
    hist->current = hist->count - 1;
}

void cmd_to_buffer_line(Command cmd, Line *line) {
    line->count = cmd.size;
    memcpy(line->content, cmd.text, sizeof(char) * cmd.size);
}

Command cmds_get_cmd(Command_History *hist) {
    return hist->cmds[hist->current];
}   

