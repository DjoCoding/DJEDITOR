#ifndef SV_H
#define SV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *content;
    size_t count;
} String_View;

#define SV(s)      (String_View) { s, strlen(s) } 

String_View sv(char *content, size_t size);
String_View sv_trim_left(String_View s);
String_View sv_trim_right(String_View s);
String_View sv_trim(String_View s);
String_View sv_chop_left(String_View s);
String_View sv_chop_right(String_View s);
String_View sv_get_until(String_View *s, char c);
int sv_cmp(String_View s, String_View t);

#endif

#ifdef SV_IMPL

String_View sv(char *content, size_t count) {
    return (String_View) { content, count };
}

String_View sv_trim_left(String_View s) {
    while(s.count) {
        if (!isspace(*s.content)) { break; }
        --s.count; ++s.content;
    }

    return s;
}

String_View sv_trim_right(String_View s) {
    while(s.count) {
        if (!isspace(*(s.content + s.count))) { break; }
        --s.count; ++s.content;
    }

    return s;
}

String_View sv_trim(String_View s) {
    return sv_trim_left(sv_trim_right(s));
}

String_View sv_chop_left(String_View s) {
    s.content++;
    s.count--;
    return s;
}

String_View sv_chop_right(String_View s) {
    s.count--;
    return s;
}

String_View sv_get_until(String_View *s, char c) {
    String_View result = sv(s->content, 0);

    while(s->count) {
        if (*s->content == c) { break; }
        *s = sv_chop_left(*s);
        ++result.count;
    }

    return result;
}

int sv_cmp(String_View s, String_View t) {
    if (s.count != t.count) { return 0; }
    return memcmp(s.content, t.content, s.count * sizeof(char)) == 0;
}

#endif