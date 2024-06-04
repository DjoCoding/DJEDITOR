#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <ncurses.h>

#include "./types.h"
#include "./init.h"

void ncurses_init() {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
}

void ncurses_quit() {
    endwin();
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


#endif