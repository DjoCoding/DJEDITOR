#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "./types.h"
#include "./lexer.h"
#include "./utils.h"
#include "./sv.h"

#include <string.h>

EDITOR_HIGHLIGHTER highlight_begin() {
    return (EDITOR_HIGHLIGHTER) {
        .arr = tokens_init(),
        .current = 0,
        .inside_comment = 0,
    };
}

int iskeyword(STRING_VIEW *sv, size_t first, size_t size) {
    for (size_t i = 0; i < ARR_SIZE(keywords); i++) {
        if (size == strlen(keywords[i]) && strncmp(sv->content + first, keywords[i], size) == 0) {
            return 1;
        }
    }
    return 0;
}

TOKEN get_next_token(EDITOR *editor, STRING_VIEW *sv) {
    size_t first = sv->current;
    size_t size = 1;
    TOKEN_TYPE type = IDENTIFIER_TYPE;

    int ch = sv_peek(sv);
    int quote_type;

    // CHECK IF WE ARE IN THE COMMENT MODE
    if (editor->highlighter.inside_comment) {
        while (!sv_end(sv)) {
            ch = sv_peek(sv);
            size++;
            sv_consume(sv);

            if (ch == '*') {
                if (sv_peek(sv) == '/') {
                    size++;
                    sv_consume(sv);
                    editor->highlighter.inside_comment = 0;
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
                size++;
                sv_consume(sv);
            }
        } else if (ch == '*') {
            editor->highlighter.inside_comment = 1;
            size++;
            sv_consume(sv);
            type = COMMENT_TYPE;
        } else {
            type = OPERATOR_TYPE;
        }
    } else if (istag(ch)) {
        size = 0;
        while (!sv_end(sv)) {
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

void print_sv(STRING_VIEW *sv, size_t first, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%c", sv->content[first + i]);
    }
}

void print_token(STRING_VIEW *sv, TOKEN token) {
    print_sv(sv, token.first, token.size);
    printf("\t%zu:%zu -> ", token.first, token.size);
    stringify(token.type);
    printf("\n");
}

void editor_lex_row(EDITOR *editor, ROW *row, size_t first, size_t size) {
    
    STRING_VIEW sv = sv_init(row->content + first, size);
    size_t current = sv.current;

    while (!sv_end(&sv)) {
        int ch = sv_peek(&sv);
        if (ch == '\t' || ch == ' ' || ch == '\n') {
            sv_consume(&sv);
        } else {
            TOKEN token = get_next_token(editor, &sv);
            tokens_push(&editor->highlighter.arr, token);
            print_token(&sv, token);
            // printf("%zu:%zu\n", get_row(sv->content, token.first), get_col(sv->content, sv->current));
        }
        if (current == sv.current) break;
        current = sv.current;
    }
}


#endif