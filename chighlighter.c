#include "headers/chighlighter.h"

char *C_symbols[] = {
    "+", "-", "*", "/", "%", "=", "==", "!=", "<", ">",
    "<=", ">=", "&&", "||", "&", "|", "^", "~", "!", "<<",
    ">>","[", "]", "{", "}", "(", ")", "?", ":", ";", ",",
    ".", "#", "\\", "\"", "\'"
};

char *C_keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", 
    "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", 
    "restrict", "return", "short", "signed", "sizeof", "static", "struct", "switch", 
    "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", 
    "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
};

Color ch_get_token_color(Token tkn) {
    for(size_t i = 0; i < ARR_LEN(C_symbols); ++i) {
        if (sv_cmp(tkn.sv, SV(C_symbols[i]))) { return C_SYMBOLS_COLOR; }
    }

    for(size_t i = 0; i < ARR_LEN(C_keywords); ++i) {
        if (sv_cmp(tkn.sv, SV(C_keywords[i]))) { return C_KEYWORDS_COLOR; }
    }

    if (tkn.sv.content[0] == '"') { return C_STRING_COLOR; }

    return DEFAULT_COLOR;
}

void ch_setup_color_buffer(Color_Buffer *cb, Token_Container *container) {
    for(size_t i = 0; i < container->size; ++i) {
        Tokens tks = container->tks[i];
        for(size_t j = 0; j < tks.count; ++j) {
            Token tkn = tks.items[j];
            Color color = ch_get_token_color(tkn);
            for (size_t k = 0; k < tkn.sv.count; ++k) {
                cb->colors[i][tkn.pos + k] = color;
            }
        }
    }
}
