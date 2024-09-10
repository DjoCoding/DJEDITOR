#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

size_t fsize(FILE *f);
char *fcontent(char *filepath);

#endif

#ifdef UTILS_IMPL

size_t fsize(FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

char *fcontent(char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        DJ_ERROR("could not open the file %s: %s\n", filepath, strerror(errno));
    }

    size_t filesize = fsize(f);
    char *content = DJ_ALLOC((filesize + 1) * sizeof(char));
    size_t size = fread(content, sizeof(char), filesize, f);

    if (size != filesize) { 
        DJ_ERROR("could not read the whole file %s", filepath);
    }

    content[filesize] = 0;
    fclose(f);

    return content;
}

#endif