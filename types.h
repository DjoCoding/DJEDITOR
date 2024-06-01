#ifndef TYPES_H
#define TYPES_H


#include <stdio.h>
#include <ncurses.h>

#include "./consts.h"

typedef enum {
    BOTTOM_WINDOW = 0,
    WINDOW_COUNT,
} WINDOW_TYPE;

typedef struct {
    size_t row;
    size_t col;
} POSITION;

typedef struct {
    char *content;
    size_t cap;
    size_t size;
} BUFFER;

typedef struct {
    POSITION pos;
    size_t index;
} CURSOR;

typedef struct {
    POSITION cursor_pos;      // pointer to where the new line begins
    size_t cursor_index;
    size_t size;           // count the number of chars in the line
} BUFFER_LINE;

typedef struct {
    BUFFER_LINE *buff_lines;
    size_t cap;           // tells the capacity of the dynamic array (buff_lines)
    size_t size;       
} BUFFER_LINES;

typedef enum {
    NORMAL, 
    INSERT, 
} MODE;

typedef enum {
    RUNNING, 
    PENDING, 
    EXIT,
} STATE;

typedef struct {
    BUFFER buff;
    BUFFER_LINES lines;
    CURSOR cursor;
    MODE mode;
    STATE state;
    WINDOW *windows[WINDOW_COUNT];
    char *FILE_NAME;
} EDITOR;

#endif