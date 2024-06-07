#ifndef TYPES_H
#define TYPES_H


#include <stdio.h>
#include <ncurses.h>
#include "./consts.h"


typedef enum {
    IDENTIFIER_TYPE = 2,
    KEYWORD_TYPE,
    PREPROCESSOR_TYPE,
    SEPARATOR_TYPE,
    STRING_TYPE,
    OPERATOR_TYPE,
    COMMENT_TYPE,
    COLON_TYPE,
    END_TYPE,
} TOKEN_TYPE;

typedef struct {
    TOKEN_TYPE type;
    size_t first;
    size_t size;
} TOKEN;


typedef struct {
    TOKEN *tokens;
    size_t count;
    size_t size;
} TOKEN_ARR;


typedef enum {
    MAIN_WINDOW = 0,
    STATUS_WINDOW,
    INPUT_WINDOW,
    WINDOW_COUNT,
} WINDOW_TYPE;

typedef struct {
    size_t row;
    size_t col;
} POSITION;

typedef struct ROW {
    char *content; 
    size_t size;
    size_t cap;
    struct ROW *next, *prev;
} ROW;

typedef struct {
    ROW *rows;
    ROW *tail;                 // POINTS TO THE LAST ROW IN THE BUFFER!
    size_t size;
    ROW *current_row;          // FOR OPTIMIZATION PURPOSES
} BUFFER;

typedef struct {
    POSITION pos;
} CURSOR;

typedef enum {
    NORMAL, 
    INSERT, 
    VISUAL,
} MODE;

typedef enum {
    RUNNING, 
    PENDING, 
    EXIT,
} STATE;

typedef struct {
    POSITION start;
    POSITION end;
} EDITOR_VISUAL;

typedef struct {
    size_t row_start;
    size_t col_start;
} EDITOR_RENDERER;

typedef struct {
    WINDOW *wind;
    size_t win_height;
    size_t win_width;
    POSITION win_pos;
    EDITOR_RENDERER renderer;
} EDITOR_WINDOW;

typedef struct _EDITOR_CONFIG {
    BUFFER buff;
    CURSOR cursor;
    MODE mode;
    STATE state;
    char *FILE_NAME;
    struct _EDITOR_CONFIG *next;
} EDITOR_CONFIG;

typedef struct {
    TOKEN_ARR arr;
    size_t current;
    int inside_comment;
} EDITOR_HIGHLIGHTER;

typedef struct _EDITOR {
    EDITOR_CONFIG config;
    EDITOR_CONFIG *snapshots;                     // IMPLEMETING THE RE-DO AND UN-DO OPERATIONS
    EDITOR_WINDOW windows[WINDOW_COUNT];
    EDITOR_VISUAL visual;
    EDITOR_HIGHLIGHTER highlighter;
} EDITOR;

#endif