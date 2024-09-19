#ifndef VEC2_H
#define VEC2_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    size_t x, y;
} uVec2;

typedef struct {
    int64_t x, y;
} iVec2;

iVec2 ivec2(int64_t x, int64_t y);
iVec2 ivec2x(int64_t x);
iVec2 ivec2y(int64_t y);
iVec2 ivec2v(int64_t v);
iVec2 ivec2_addx(iVec2 a, int64_t v);
iVec2 ivec2_addy(iVec2 a, int64_t v);
iVec2 ivec2_add(iVec2 a, iVec2 b);


uVec2 uvec2(size_t x, size_t y);
uVec2 uvec2x(size_t x);
uVec2 uvec2y(size_t y);
uVec2 uvec2v(size_t v);
uVec2 uvec2_addx(uVec2 a, size_t v);
uVec2 uvec2_addy(uVec2 a, size_t v);
uVec2 uvec2_add(uVec2 a, uVec2 b);

#ifdef IVEC2_IMPL

iVec2 ivec2(int64_t x, int64_t y) {
    return (iVec2) { x, y }; 
}

iVec2 ivec2x(int64_t x) {
    return ivec2(x, 0);
}

iVec2 ivec2y(int64_t y) {
    return ivec2(y, 0);
}

iVec2 ivec2v(int64_t v) {
    return ivec2(v, v);
}

iVec2 ivec2_addx(iVec2 a, int64_t v) {
    return ivec2(a.x + v, a.y);
}

iVec2 ivec2_addy(iVec2 a, int64_t v) {
    return ivec2(a.x, a.y + v);
}

iVec2 ivec2_add(iVec2 a, iVec2 b) {
    return ivec2_addy(ivec2_addx(a, b.x), b.y);
}

#endif

#ifdef UVEC2_IMPL

uVec2 uvec2(size_t x, size_t y) {
    return (uVec2) { x, y }; 
}

uVec2 uvec2x(size_t x) {
    return uvec2(x, 0);
}

uVec2 uvec2y(size_t y) {
    return uvec2(y, 0);
}

uVec2 uvec2v(size_t v) {
    return uvec2(v, v);
}

uVec2 uvec2_addx(uVec2 a, size_t v) {
    return uvec2(a.x + v, a.y);
}

uVec2 uvec2_addy(uVec2 a, size_t v) {
    return uvec2(a.x, a.y + v);
}

uVec2 uvec2_add(uVec2 a, uVec2 b) {
    return uvec2_addy(uvec2_addx(a, b.x), b.y);
}

#endif


#endif