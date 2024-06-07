#ifndef SV_H
#define SV_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char *content;
    size_t size;
    size_t current;
} STRING_VIEW;

STRING_VIEW sv_init(char *content, size_t size) {
    return (STRING_VIEW) {
        .content = content,
        .size = size,
        .current = 0,
    };
}

char sv_peek(STRING_VIEW *sv) {
    return (sv->content[sv->current]);
}

char sv_consume(STRING_VIEW *sv) {
    return (sv->content[sv->current++]);
}

int sv_end(STRING_VIEW *sv) {
    if (sv->current == sv->size) 
        return 1;
    return 0;
}


#endif