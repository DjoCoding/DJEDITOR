#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include "./utils.h"
#include "./types.h"
#include "./sv.h"

// INITIALIZERS
TOKEN token_init(TOKEN_TYPE type, size_t first, size_t size) {
    return (TOKEN) {
        .type = type,
        .first = first,
        .size = size,
    };
}

TOKEN_ARR tokens_init() {
    TOKEN_ARR arr;
    arr.size = TOKENS_INITIAL_SIZE;
    arr.count = 0;
    arr.tokens = (TOKEN *)malloc(sizeof(TOKEN) * arr.size);
    return arr;
}

int tokens_full(TOKEN_ARR *arr) {
    return (arr->count == arr->size);
} 

int tokens_empty(TOKEN_ARR *arr) {
    return (arr->size == 0);
}

void tokens_resize(TOKEN_ARR *arr) {
    if (tokens_empty(arr)) 
        arr->size = TOKENS_INITIAL_SIZE;
    else 
        arr->size *= 2;
    arr->tokens = (TOKEN *) realloc(arr->tokens, arr->size * sizeof(TOKEN));
}

void tokens_push(TOKEN_ARR *arr, TOKEN token) {
    if (tokens_full(arr))
        tokens_resize(arr);
    
    arr->tokens[arr->count++] = token;
}

#endif