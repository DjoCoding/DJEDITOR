#ifndef ENV_H
#define ENV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define DJ_ERROR(...)        { fprintf(stderr, "ERROR: "); fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); }
#define DJ_ASSERT(cond, ...) { assert((cond) && __VA_ARGS__); }
#define DJ_ALLOC             alloc
#define DJ_REALLOC           ralloc

void *alloc(size_t size);
void *ralloc(void *prev, size_t isize, size_t nsize);

#ifdef ENV_IMPL

// memory allocation wrapper function
void *alloc(size_t size) {
    void *p = malloc(size);
    if (!p) { DJ_ERROR("could not allocate memory"); }
    return p;
}

// isize for item size
// nsize for new size
void *ralloc(void *prev, size_t isize, size_t nsize) {
    void *p = realloc(prev, nsize * isize);
    if (!p) { DJ_ERROR("could not allocate memory"); }
    return p;
}

#endif



#endif