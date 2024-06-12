#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <ncurses.h>

#include "./init.h"
#include "./consts.h"
#include "./sv.h"

#define ARR_SIZE(arr) sizeof(arr)/sizeof((arr)[0])
#define CTRL(x) (x) & 31




char *keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
    "else", "enum", "extern", "float", "for", "goto", "if", "inline",
    "int", "long", "register", "restrict", "return", "short", "signed", "sizeof",
    "static", "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool",
    "_Complex", "_Generic", "_Imaginary", "_Noreturn", "_Static_assert",
    "_Thread_local", "size_t", "NULL", "FILE", "uint8_t", "uint16_t"
};

char *modes[] = {
    "NORMAL",
    "INSERT",
    "VISUAL",
};

char *states[] = {
    "RUNNING", 
    "PENDING", 
    "EXIT",
};

char *token_types[] = {
    "IDENTIFIER_TYPE",
    "KEYWORD_TYPE",
    "PREPROCESSOR_TYPE",
    "SEPARATOR_TYPE",
    "STRING_TYPE",
    "OPERATOR_TYPE",
    "COMMENT_TYPE",
    "COLON_TYPE",
    "END_TYPE",
};

char operators[] = {
    '+',
    '-',
    '*',
    '/',
    '%',
    '>',
    '<',
    '=',
    '!',
    '?',
    '&',
    '|',
    '^',
};

char seps[] = {
    '(',
    ')',
    '[',
    ']',
    '{',
    '}',
    '.',
};

void ncurses_init() {
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    raw();
    start_color();
}

void ncurses_quit() {
    endwin();
}

POSITION get_window_size(WINDOW *wind) {
    size_t height, width;
    getmaxyx(stdscr, height, width);
    return (POSITION) {
        .row = height,
        .col = width,
    };
}

size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

char *get_file_content(char *filename, size_t *content_size) {
    if (filename == NULL) 
        return NULL;

    FILE *fp = fopen(filename, "r");

    if (fp == NULL) return NULL;

    fseek(fp, 0, SEEK_END);
    *content_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *content = (char *)malloc(sizeof(char) * ((*content_size) + 1));

    fread(content, sizeof(char), *content_size, fp);

    content[*content_size] = NULL_TERMINATOR;

    fclose(fp);

    return content;
}

int count_num_digit(size_t number) {
    int count = 0;
    while (number != 0) {
        count++;
        number /= 10;
    }
    return count;
}

void swap(size_t *a, size_t *b) {
    size_t temp = *a;
    *a = *b;
    *b = temp;
}

int isletter(int ch) {
    if ((ch <= 'Z' && ch >= 'A') || (ch <= 'z' && ch >= 'a'))
        return 1;
    return 0;
}

int isnumber(int ch) {
    if (ch <= '9' && ch >= '0') 
        return 1;
    return 0;
}

int isvalid(int ch) {
    if (isletter(ch) || isnumber(ch) || ch == '_') 
        return 1;
    return 0;
}

int istag(int ch) {
    if (ch == '#')
        return 1;
    return 0;
}

int isoperator(int ch) {
    for (size_t i = 0; i < ARR_SIZE(operators); i++)
        if (ch == operators[i]) 
            return 1;

    return 0;
}

int issep(int ch) {
    for (size_t i = 0; i < ARR_SIZE(seps); i++)
        if (ch == seps[i])
            return 1;
    return 0;
}

int iscomment(int ch) {
    if (ch == '/')
        return 1;
    return 0;
}

int isquote(int ch) {
    if (ch == '"' || ch == '\'')
        return 1;
    return 0;
}

int iscolon(int ch) {
    if (ch == ':' || ch == ';' || ch == ',') 
        return 1;
    return 0;
}

int iskeyword(STRING_VIEW *sv, size_t first, size_t size) {
    for (size_t i = 0; i < ARR_SIZE(keywords); i++) {
        if (size == strlen(keywords[i]) && strncmp(sv->content + first, keywords[i], size) == 0) {
            return 1;
        }
    }
    return 0;
}


void stringify(TOKEN_TYPE type) {
    printf("%s", token_types[type - 2]);
}

char *stringify_mode(MODE mode) {
    return modes[mode];
}



// char *editor_stringify_state(EDITOR *editor) {
//     return states[editor->config.state];
// } 

// char *editor_stringify_mode(EDITOR *editor) {
//     return modes[editor->config.mode];
// }



#endif