//
// Created by francesco on 26/11/21.
//

#ifndef CHORA_TEMPLATE_H
#define CHORA_TEMPLATE_H

#include "../types.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#define define_vector(type)\
\
typedef struct vector_ ## type vector_ ## type;\
\
struct vector_ ## type {\
    usize length;\
    usize capacity;\
    type *entries;\
\
    void (* free)(vector_ ## type *);\
    void (* swap)(vector_ ## type *, vector_ ## type *);\
\
    void (*reserve)(vector_ ## type *v, usize size);\
    bool (* is_empty)(const vector_ ## type *);\
    void (* push)(vector_ ## type *, const type *);\
    type (* pop)(vector_ ## type *);\
    type *(* top)(const vector_ ## type *);\
    type *(* get)(const vector_ ## type *, usize pos);\
\
};\
\
\
static void vector_ ## type ## _free(vector_ ## type *v) {\
\
    free(v->entries);\
    v->entries = NULL;\
    v->length = 0;\
    v->capacity = 0;\
}\
\
\
static void vector_ ## type ## _swap(vector_ ## type *v1, vector_ ## type *v2) {\
\
    vector_ ## type tmp = *v1;\
    *v1 = *v2;\
    *v2 = tmp;\
}\
\
\
static bool vector_ ## type ## _is_empty(const vector_ ## type *v) {\
    return (v->length == 0 ? true : false);\
}\
\
\
static void vector_ ## type ## _reserve(vector_ ## type *v, usize size) {\
\
    if (v->capacity < size) {\
        v->capacity = size;\
        v->entries = reallocarray(v->entries, v->capacity, sizeof(type));\
    }\
}\
\
\
static void vector_ ## type ## _push(vector_ ## type *v, const type *item){\
\
    if (v->length >= v->capacity) {\
        v->capacity = (v->capacity > 0 ? v->capacity * 2 : 1 );\
        v->entries = reallocarray(v->entries, v->capacity, sizeof(type));\
    }\
    v->entries[v->length] = *item;\
    ++v->length;\
}\
\
\
static type vector_ ## type ## _pop(vector_ ## type *v) {\
\
    if (v->length == 0) {\
        exit(1);\
    } else {\
        type t = v->entries[v->length - 1];\
        --v->length;\
\
        if (v->length < v->capacity / 4) {\
            v->capacity /= 4;\
            v->entries = reallocarray(v->entries, v->capacity, sizeof(type));\
        }\
\
        return t;\
    }\
}\
\
\
static type* vector_ ## type ## _top(const vector_ ## type *v) {\
    if (v->length == 0) {\
        return NULL;\
    } else {                  \
        return &v->entries[v->length - 1];\
    }\
}\
\
\
static type* vector_ ## type ## _get(const vector_ ## type *v, usize pos) {\
\
    if (v->length <= pos) {\
        return NULL;\
    } else {\
        return &v->entries[pos];\
    }\
}\
\
\
static vector_ ## type vector_ ## type ## _new() {\
\
    vector_##type t = { 0, 0, NULL};\
\
    t.free = &( vector_ ## type ## _free );\
    t.swap = &( vector_ ## type ## _swap );\
    t.reserve = &( vector_ ## type ## _reserve );\
    t.is_empty = &( vector_ ## type ## _is_empty );\
    t.push = &( vector_ ## type ## _push );\
    t.pop = &( vector_ ## type ## _pop );\
    t.top = &( vector_ ## type ## _top );\
    t.get = &( vector_ ## type ## _get );\
\
    return t;\
};\

#define new_vector(type) vector_ ## type ## _new()

#define Vector(type) vector_ ## type


#endif //CHORA_TEMPLATE_H
