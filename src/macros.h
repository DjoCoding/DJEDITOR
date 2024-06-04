#ifndef MACROS_H
#define MACROS_H

#define CTRL(x) ((x) & 31)

// COLORS DEFINITION
#define BLACK                    0
#define BLUE                     1
#define GREEN                    2
#define CYAN                     3
#define RED                      4
#define MAGNETA                  5
#define YELLOW                   6 
#define WHITE                    7
#define BRIGHT_BLACK             8
#define BRIGHT_RED               9
#define BRIGHT_GREEN            10
#define BRIGHT_YELLOW           11
#define BRIGHT_BLUE             12
#define BRIGHT_MAGNETA          13
#define BRIGHT_CYAN             14  
#define BRIGHT_WHITE            15 


#define _MAIN_THEME 1
#define _LINE_NUMBER_COLOR 2

#define INIT_MAIN_THEME() init_pair(_MAIN_THEME, BRIGHT_CYAN, COLOR_BLACK)
#define INIT_LINE_NUMBER_THEME() init_pair(_LINE_NUMBER_COLOR, COLOR_YELLOW, COLOR_BLACK)

// THEMES DEFINITION
#define MAIN_THEME COLOR_PAIR(_MAIN_THEME)
#define LINE_NUMBER_THEME COLOR_PAIR(_LINE_NUMBER_COLOR)

#endif