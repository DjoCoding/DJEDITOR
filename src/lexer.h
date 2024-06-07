#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include "./sv.h"
#include "./utils.h"
#include "./types.h"

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
    arr.size = 10;
    arr.count = 0;
    arr.tokens = (TOKEN *) malloc(sizeof(TOKEN) * arr.size);
    return arr;
}

int tokens_full(TOKEN_ARR *arr) {
    if (arr->count == arr->size)
        return 1;
    return 0;
} 

void tokens_resize(TOKEN_ARR *arr) {
    arr->size *= 2;
    arr->tokens = (TOKEN *) realloc(arr->tokens, arr->size * sizeof(TOKEN));
}

void tokens_push(TOKEN_ARR *arr, TOKEN token) {
    if (tokens_full(arr))
        tokens_resize(arr);
    
    arr->tokens[arr->count++] = token;
}

#endif