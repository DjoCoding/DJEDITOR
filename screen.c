#include "headers/screen.h"

Config config_init(int w, int h, int row, int col) {
    return (Config) { w, h, row, col };
}

Screen screen_init(Config config) {
    Screen screen = {0};

    screen.config = config;

    screen.window = newwin(CONF_UNPACK(config));
    if (!screen.window) {
        DJ_ERROR("could not initialize ncurses window\n");
    }

    return screen;
}