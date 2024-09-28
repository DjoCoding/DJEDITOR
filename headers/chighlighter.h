#ifndef C_HIGHLIGHTER_H
#define C_HIGHLIGHTER_H

#include "color.h"
#include "tokenizer.h"


#define C_SYMBOLS_COLOR   RED_ON_DEFAULT
#define C_KEYWORDS_COLOR  RED_ON_DEFAULT
#define C_STRING_COLOR    YELLOW_ON_DEFAULT
#define DEFAULT_COLOR     CYAN_ON_DEFAULT

extern char *symbols[];

Color ch_get_token_color(Token tkn);
void ch_setup_color_buffer(Color_Buffer *cb, Token_Container *container);


#endif