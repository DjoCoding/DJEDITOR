#define ENV_IMPL
#define UTILS_IMPL
#define SV_IMPL
#define UVEC2_IMPL
#define IVEC2_IMPL

#include "headers/editor.h"

Editor e = {0};

#define ARR_LEN(arr)    sizeof(arr)/sizeof((arr)[0])

int ncurses_color_table[] = {
    COLOR_BLACK, 
    COLOR_RED, 
    COLOR_GREEN, 
    COLOR_YELLOW, 
    COLOR_BLUE, 
    COLOR_MAGENTA, 
    COLOR_CYAN, 
    COLOR_WHITE
};


// ncurses initialization and cleaning 
void ncurses_init(void) {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
}

void ncurses_init_colors(void) {
    start_color();
    use_default_colors();

    size_t i = 0;
    while(i < ARR_LEN(ncurses_color_table)) {
        printf("%ld\n", i + BLACK_ON_DEFAULT);
        init_pair(i + BLACK_ON_DEFAULT, ncurses_color_table[i], -1);
        ++i;
    }

    for(size_t i = 1; i < ARR_LEN(ncurses_color_table); ++i) {
        init_pair(i + RED_ON_BLACK - 1, ncurses_color_table[i], COLOR_BLACK);
    }
}

void ncurses_quit(void) {
    endwin();
}

int main(void) {
    ncurses_init();

    e = editor_init();

    #if 1   
        ncurses_init_colors();
    #endif
    
    editor_set_default_color(&e);

    e.state = RUNNING;
    
    while (e.state != STOPED) {
        editor_update(&e);
        editor_render_highlighted(&e);
    }
    
    editor_clean(&e);

    ncurses_quit();
}
