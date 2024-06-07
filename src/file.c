#include "./lexer.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int iskeyword(STRING_VIEW *sv, size_t first, size_t size) {
    for (int i = 0; i < ARR_SIZE(keywords); i++) {
        if (size == strlen(keywords[i]) && strncmp(sv->content + first, keywords[i], size) == 0) {
            return 1;
        }
    }
    return 0;
}

size_t inside_comment = 0;

TOKEN get_next_token(STRING_VIEW *sv) {
    size_t first = sv->current;
    size_t size = 1;
    TOKEN_TYPE type = IDENTIFIER_TYPE;

    int ch = sv_peek(sv);
    int quote_type;

    if (inside_comment) {
        while (!sv_end(sv)) {
            ch = sv_peek(sv);
            size++;
            sv_consume(sv);

            if (ch == '*') {
                if (sv_peek(sv) == '/') {
                    size++;
                    sv_consume(sv);
                    inside_comment = 0;
                    break;
                }
            }
        }
        type = COMMENT_TYPE;
    } else if (iscomment(ch)) {
        sv_consume(sv);
        ch = sv_peek(sv);
        if (ch == '/') {
            type = COMMENT_TYPE;
            while (!sv_end(sv)) {
                if (sv_peek(sv) == '\n') break;
                size++;
                sv_consume(sv);
            }
        } else if (ch == '*') {
            inside_comment = 1;
            size++;
            sv_consume(sv);
            type = COMMENT_TYPE;
        } else {
            type = OPERATOR_TYPE;
        }
    } else if (istag(ch)) {
        size = 0;
        while (!sv_end(sv)) {
            if (sv_peek(sv) == '\n') break;
            size++;
            sv_consume(sv);
        }
        type = PREPROCESSOR_TYPE;
    } else if (issep(ch)) {
        type = SEPARATOR_TYPE;
        sv_consume(sv);
    } else if (isquote(ch)) {
        quote_type = ch;
        type = STRING_TYPE;
        sv_consume(sv);
    } else if (iscolon(ch)) {
        type = SEPARATOR_TYPE;
        sv_consume(sv);
    } else if (isoperator(ch)) {
        type = OPERATOR_TYPE;
        sv_consume(sv);
    }

    if (type == STRING_TYPE) {
        while (!sv_end(sv)) {
            int ch = sv_peek(sv);
            sv_consume(sv);
            size++;
            if (ch == quote_type) break;
        }
    }

    if (type == IDENTIFIER_TYPE) {
        size = 0;
        while (!sv_end(sv)) {
            if (isvalid(sv_peek(sv))) {
                size++;
                sv_consume(sv);
            } else {
                break;
            }
        }
        if (iskeyword(sv, first, size)) {
            type = KEYWORD_TYPE;
        }
    }

    return token_init(type, first, size);
}

size_t get_row(char *content, size_t index) {
    size_t count = 0;
    for(size_t i = 0; i < index; i++) {
        if (content[i] == '\n')
            count++;
    }
    return count;
}

size_t get_col(char *content, size_t index) {
    size_t count = 0;
    for(size_t i = 0; i < index; i++) {
        if (content[i] == '\n')
            count = 0;
    }
    return count;
}

TOKEN_ARR lex(STRING_VIEW *sv) {
    TOKEN_ARR arr = tokens_init();
    while (!sv_end(sv)) {
        int ch = sv_peek(sv);
        if (ch == '\t' || ch == ' ' || ch == '\n') {
            sv_consume(sv);
        } else {
            TOKEN token = get_next_token(sv);
            tokens_push(&arr, token);
            print_token(sv, token);
            // printf("%zu:%zu\n", get_row(sv->content, token.first), get_col(sv->content, sv->current));
        }
    }
    return arr;
}

void print_tokens(STRING_VIEW *sv, TOKEN_ARR arr) {
    for (size_t i = 0; i < arr.count; i++) {
        print_token(sv, arr.tokens[i]);
    }
}

int main() {
    size_t size = 0;
    char *content = get_file_content("../main.c", &size);
    if (content == NULL) {
        fprintf(stderr, "Failed to read the file.\n");
        return 1;
    }
    STRING_VIEW sv = sv_init(content, size);
    TOKEN_ARR arr = lex(&sv);
    print_tokens(&sv, arr);
    free(content);
    free(arr.tokens);
    return 0;
}
