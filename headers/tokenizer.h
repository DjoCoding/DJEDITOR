#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include "env.h"
#include "../tools/vec2.h"
#include "../tools/sv.h"

typedef struct {
    char *text;
    size_t size;
    size_t current;
} Tokenizer;    

typedef struct {
    size_t pos;
    String_View sv;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t size;
} Tokens;

typedef struct {
    Tokens *tks;
    size_t size;
} Token_Container;  

#define TOKENS_INITIAL_SIZE  10

Tokenizer tk_setup(char *text, size_t size);

Token token_init(size_t pos, String_View sv);

Tokens tokens_init(size_t size);
void tokens_append(Tokens *tks, Token tkn);
void tokens_clean(Tokens *tks);
void tokens_reset(Tokens *tks);

Token_Container tkn_container_init(size_t size);

Tokens tk_tokenize_text(Tokenizer *tk, char *text, size_t text_size);

#endif