#ifndef SCREEN_H
#define SCREEN_H

#include <stdio.h>
#include <ncurses.h>

#include "env.h"

typedef struct {
    int w;
    int h;
    int row;    // the starting point for the window
    int col;    // the starting point for the window
} Config; 

#define CONF_UNPACK(conf)  conf.h, conf.w, conf.row, conf.col

typedef struct {
    WINDOW *window;
    Config config;
} Screen;

Config config_init(int w, int h, int row, int col);
Screen screen_init(Config config);

#endif 