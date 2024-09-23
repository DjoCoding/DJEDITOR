#include "headers/tokenizer.h"

Tokenizer tk_setup(char *text, size_t size) {
    return (Tokenizer) { text, size, 0 };
}

Token token_init(size_t pos, String_View sv) {
    return (Token) { pos, sv }; 
}

Tokens tokens_init(size_t size) {
    Tokens tks = {0};
    tks.size = size;
    tks.items = DJ_ALLOC(sizeof(Token) * tks.size);
    return tks;
}

void tokens_resize(Tokens *tks) {
    if (tks->size == 0) { tks->size = TOKENS_INITIAL_SIZE; }
    tks->size *= 2;
    tks->items = DJ_REALLOC(tks->items, sizeof(Token), tks->size);
}

void tokens_append(Tokens *tks, Token tkn) {
    if (tks->count >= tks->size) { tokens_resize(tks); }
    tks->items[tks->count++] = tkn;
}

void tokens_clean(Tokens *tks) {
    if (tks->items) free(tks->items);
}

void tokens_reset(Tokens *tks) {
    tks->count = 0;
}

char tk_peek(Tokenizer *tk) {
    return tk->text[tk->current];
}

void tk_advance(Tokenizer *tk) {
    ++tk->current;
}

bool tk_end(Tokenizer *tk) {
    return (tk->current >= tk->size);
}

Token tk_get_token(Tokenizer *tk) {
    size_t pos = tk->current;
    char *c = &tk->text[tk->current];

    if (isalpha(*c)) {
        while(!tk_end(tk)) {
            char current = tk_peek(tk);
            if (!isalnum(current)) { break; }
            tk_advance(tk);
        }

        return token_init(pos, sv(c, tk->current - pos));
    } 

    if (isdigit(*c)) {
        while(!tk_end(tk)) {
            char current = tk_peek(tk);
            if (!isdigit(current)) { break; }
            tk_advance(tk);
        }

        return token_init(pos, sv(c, tk->current - pos));
    } 

    if (*c == '"') {
        tk_advance(tk);
        while(!tk_end(tk)) { 
            char current = tk_peek(tk);
            tk_advance(tk);
            if (current == '"') { break; }
        }

        return token_init(pos, sv(c, tk->current - pos));
    }

    tk_advance(tk);
    return token_init(pos, sv(c, 1));
}

Tokens tk_tokenize_text(Tokenizer *tk, char *text, size_t text_size) {
    *tk = tk_setup(text, text_size);
    Tokens tks = tokens_init(TOKENS_INITIAL_SIZE);
    
    while(!tk_end(tk)) {
        if (isspace(tk_peek(tk))) { tk_advance(tk); continue; }
        Token tkn = tk_get_token(tk);
        tokens_append(&tks, tkn);
    }

    return tks;
}

Token_Container tkn_container_init(size_t size) {
    Token_Container container = {0};
    container.size = size;
    container.tks = DJ_ALLOC(sizeof(Tokens) * size);
    memset(container.tks, 0, sizeof(Tokens) * size);
    return container;
}