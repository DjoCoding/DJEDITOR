#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <ncurses.h>

typedef enum {
    IDENTIFIER_TYPE = 2,
    KEYWORD_TYPE,
    PREPROCESSOR_TYPE,
    SEPARATOR_TYPE,
    STRING_TYPE,
    OPERATOR_TYPE,
    COMMENT_TYPE,
    COLON_TYPE,
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


typedef struct {
    size_t row;
    size_t col;
} POSITION;

typedef struct {
    size_t begin;
    size_t size;
} ROW;

typedef struct {
    ROW *rows;
    size_t size;
    size_t count;
} ROW_ARR;

typedef struct {
    char *content; 
    size_t size;
    size_t count;
    size_t pos;
} BUFFER;


typedef enum {
    NORMAL = 0,
    INSERT, 
    VISUAL,
} MODE;

typedef enum {
    RUNNING, 
    PENDING, 
    EXITING,
} STATE;

typedef struct {
    size_t row;
    size_t col;
} RENDRER;

typedef struct {
    int inside_comment;
    TOKEN_ARR arr;
    size_t current;
} HIGHLIGHTER;

typedef enum {
    MAIN_WINDOW = 0,
    LINE_NUMBERS_WINDOWS,
    WINDOWS_COUNT, 
} WINDOW_TYPE;  

typedef struct {
    size_t height;
    size_t width;
    size_t row;
    size_t col;
} WINDOW_CONFIG;

typedef struct {
    WINDOW *wind;   
    WINDOW_CONFIG config;
} EDITOR_WINDOW;

typedef struct {
    size_t begin;
    size_t end;
} EDITOR_VISUAL;

typedef struct {
    BUFFER buffer;
    ROW_ARR arr;
    RENDRER render;
    HIGHLIGHTER highlight;
    STATE state;
    MODE mode;
    size_t pos; 
    EDITOR_WINDOW windows[WINDOWS_COUNT];      
    EDITOR_VISUAL visual;
    char *filename;
} EDITOR;

#endif