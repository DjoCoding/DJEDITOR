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
} MODE;

typedef enum {
    RUNNING, 
    PENDING, 
    EXIT,
} STATE;

typedef struct _EDITOR_CONFIG {
    BUFFER buff;
    CURSOR cursor;
    MODE mode;
    STATE state;
    char *FILE_NAME;
    struct _EDITOR_CONFIG *next;
} EDITOR_CONFIG;

typedef struct _EDITOR {
    EDITOR_CONFIG config;
    EDITOR_CONFIG *snapshots;                     // IMPLEMETING THE RE-DO AND UN-DO OPERATIONS
    WINDOW *windows[WINDOW_COUNT];
} EDITOR;

#endif