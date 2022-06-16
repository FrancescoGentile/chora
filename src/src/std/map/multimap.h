//
// Created by francesco on 01/12/21.
//

#ifndef CHORA_MULTIMAP_H
#define CHORA_MULTIMAP_H

#include "../string.h"
#include "../types.h"
#include "../vector/template.h"
#include "map.h"

#include <stdbool.h>

#define define_multi_map(type)\
\
define_vector(type)\
\
typedef struct {\
    EntryKind kind;\
    StringSlice key;\
    Vector(type) values;\
} multi_entry_ ## type ;\
\
\
typedef struct multi_map_ ## type multi_map_ ## type ;\
\
\
struct multi_map_ ## type {\
    usize capacity;\
    usize length;\
    usize deleted;\
    multi_entry_ ## type *entries;\
\
    void (*free)(multi_map_ ## type *mm);\
\
    void (*insert)(multi_map_ ## type *mm, const StringSlice *key, const type *value);\
    bool (*contains_key)(const multi_map_ ## type *mm, const StringSlice *key);\
    Vector(type) * (*get)(const multi_map_ ## type *mm, const StringSlice *key);\
    bool (*remove)(multi_map_ ## type *mm, const StringSlice *key, Vector(type) *values);\
};\
\
\
static usize multi_map_ ## type ## _insert_index(const multi_entry_ ## type *entries, usize capacity, const StringSlice *key) {\
\
    usize i = key->hash % capacity;\
\
    while (true) {\
        if (entries[i].kind == PRESENT && !str_equals(key, &entries[i].key)) {\
            i = (i + 1) % capacity;\
        } else {\
            return i;\
        }\
    }\
}\
\
\
static void multi_map_ ## type ## _resize(multi_map_ ## type *mm, usize new_cap) {\
\
    multi_entry_ ## type *entries = calloc(new_cap, sizeof(multi_entry_ ## type));\
    for (usize i = 0; i < new_cap; ++i) {\
        entries[i].kind = ABSENT;\
    }\
\
    for (usize i = 0; i < mm->capacity; ++i) {\
        if (mm->entries[i].kind == PRESENT) {\
            usize j = multi_map_ ## type ## _insert_index(entries, new_cap, &mm->entries[i].key);\
            entries[j].kind = PRESENT;\
            entries[j].key = mm->entries[i].key;\
            entries[j].values = mm->entries[i].values;\
        }\
    }\
\
    free(mm->entries);\
    mm->entries = entries;\
    mm->capacity = new_cap;\
    mm->deleted = 0;\
}\
\
\
static usize multi_map_ ## type ## _search(const multi_map_ ## type *mm, const StringSlice *key) {\
\
    usize i = key->hash % mm->capacity;\
    usize j = 0;\
\
    while (j < mm->capacity) {\
        if (mm->entries[i].kind == ABSENT) {\
            return mm->capacity + 1;\
        } else if (mm->entries[i].kind == PRESENT && str_equals(key, &mm->entries[i].key)) {\
            return i;\
        } else {\
            i = (i + 1) % mm->capacity;\
        }\
\
        ++j;\
    }\
\
    return mm->capacity + 1;\
}\
\
\
static void multi_map_ ## type ## _insert(multi_map_ ## type *mm, const StringSlice *key, const type *value) {\
\
    if (mm->length + 1 >= MAX_LOAD_FACTOR * mm->capacity) {\
        usize new_cap = (mm->capacity > 0 ? mm->capacity * 2 : 1);\
        multi_map_ ## type ## _resize(mm, new_cap);\
    }\
\
    usize i = multi_map_ ## type ## _insert_index(mm->entries, mm->capacity, key);\
    switch (mm->entries[i].kind) {\
        case ABSENT: {\
            ++mm->length;\
            mm->entries[i].kind = PRESENT;\
            mm->entries[i].key = *key;\
            mm->entries[i].values = new_vector(type);\
            mm->entries[i].values.push(&mm->entries[i].values, value);\
            break;\
        }\
        case PRESENT: {\
            mm->entries[i].values.push(&mm->entries[i].values, value);\
            break;\
        }\
        case DELETED: {\
            ++mm->length;\
            --mm->deleted;\
            mm->entries[i].kind = PRESENT;\
            mm->entries[i].key = *key;\
            mm->entries[i].values = new_vector(type);\
            mm->entries[i].values.push(&mm->entries[i].values, value);\
            break;\
        }\
    }\
}\
\
\
static bool multi_map_ ## type ## _contains_key(const multi_map_ ## type *mm, const StringSlice *key) {\
\
    if (mm->length == 0) {\
        return false;\
    }\
\
    usize i = multi_map_ ## type ## _search(mm, key);\
    return i < mm->capacity;\
}\
\
\
static Vector(type)* multi_map_ ## type ## _get(const multi_map_ ## type *mm, const StringSlice *key) {\
\
    if (mm->length == 0) {\
        return NULL;\
    }\
\
    usize i = multi_map_ ## type ## _search(mm, key);\
    return (i < mm->capacity ? &mm->entries[i].values : NULL);\
}\
\
\
static bool multi_map_ ## type ## _remove(multi_map_ ## type *mm, const StringSlice *key, Vector(type) *values) {\
\
    bool deleted = false;\
    usize i = multi_map_ ## type ## _search(mm, key);\
    if (i < mm->capacity) {\
        deleted = true;\
        mm->entries[i].kind = DELETED;\
        ++mm->deleted;\
        --mm->length;\
\
        if (values != NULL) {\
            *values = mm->entries[i].values;\
        } else {\
            mm->entries[i].values.free(&mm->entries[i].values);\
        }\
    }\
\
    if (mm->length < mm->capacity * MIN_LOAD_FACTOR) {\
        multi_map_ ## type ## _resize(mm, (usize) ((f64) mm->capacity * 0.5f));\
    }\
    if (mm->deleted > mm->capacity * MAX_DELETED_FACTOR) {\
        multi_map_ ## type ## _resize(mm, mm->capacity);\
    }\
}\
\
\
static void multi_map_ ## type ## _free(multi_map_ ## type *mm) {\
\
    for (usize i = 0; i < mm->capacity; ++i) {\
        if (mm->entries[i].kind == PRESENT) {\
            mm->entries[i].values.free(&mm->entries[i].values);\
        }\
    }\
    free(mm->entries);\
}\
\
\
static multi_map_ ## type multi_map_ ## type ## _new() {\
\
    multi_map_ ## type mm = {0, 0, 0, NULL};\
\
    mm.free = &( multi_map_ ## type ## _free);\
    mm.insert = &( multi_map_ ## type ## _insert);\
    mm.contains_key = &( multi_map_ ## type ## _contains_key);\
    mm.get = &( multi_map_ ## type ## _get);\
    mm.remove = &( multi_map_ ## type ## _remove);\
\
    return mm;\
}\

#define MultiMap(type) multi_map_ ## type

#define MultiEntry(type) multi_entry_ ## type

#define new_multi_map(type) multi_map_ ## type ## _new()

#endif //CHORA_MULTIMAP_H
