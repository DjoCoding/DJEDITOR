#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "./types.h"
#include "./lexer.h"
#include "./utils.h"
#include "./sv.h"

#include <string.h>


TOKEN get_next_token(EDITOR *editor, STRING_VIEW *sv) {
    size_t first = sv->current;
    size_t size = 1;
    TOKEN_TYPE type = IDENTIFIER_TYPE;

    int ch = sv_peek(sv);
    int quote_type;

    // CHECK IF WE ARE IN THE COMMENT MODE
    if (editor->highlight.inside_comment) {
        while (!sv_end(sv)) {
            ch = sv_peek(sv);
            size++;
            sv_consume(sv);

            if (ch == '*') {
                if (sv_peek(sv) == '/') {
                    size++;
                    sv_consume(sv);
                    editor->highlight.inside_comment = 0;
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
                if (sv_peek(sv) == NEW_LINE_CHAR) {
                    sv_consume(sv);
                    break;
                }
                
                size++;
                sv_consume(sv);
            }
        } else if (ch == '*') {
            editor->highlight.inside_comment = 1;
            size++;
            sv_consume(sv);
            type = COMMENT_TYPE;
        } else {
            type = OPERATOR_TYPE;
        }
    } else if (istag(ch)) {
        size = 0;
        while (!sv_end(sv)) {
            if (sv_peek(sv) == NEW_LINE_CHAR) {
                sv_consume(sv);
                break;
            }
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
        sv_consume(sv);
        while (!sv_end(sv)) {
            if (isvalid(sv_peek(sv))) {
                size++;
                sv_consume(sv);
            } else break;
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

void editor_lex_row(EDITOR *editor, size_t begin, size_t size) {
    STRING_VIEW sv = sv_init(editor->buffer.content + begin, size);

    while (!sv_end(&sv)) {
        int ch = sv_peek(&sv);
        if (ch == '\t' || ch == ' ' || ch == '\n') {
            sv_consume(&sv);
        } else {
            TOKEN token = get_next_token(editor, &sv);
            tokens_push(&editor->highlight.arr, token);
        }
    }
}

int editor_highlight_end(EDITOR *editor) {
    return (editor->highlight.current == editor->highlight.arr.count);
}

TOKEN editor_highlight_get_current_token(EDITOR *editor) {
    return editor->highlight.arr.tokens[editor->highlight.current];
}

int editor_start_highlight(EDITOR *editor, size_t index) {
    return (!editor_highlight_end(editor) && (index == editor_highlight_get_current_token(editor).first));
}


#endif