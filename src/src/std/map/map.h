//
// Created by francesco on 01/12/21.
//

#ifndef CHORA_MAP_H
#define CHORA_MAP_H

static const f64 MAX_LOAD_FACTOR = 0.75f;
static const f64 MIN_LOAD_FACTOR = 0.25f;
static const f64 MAX_DELETED_FACTOR = 0.50f;

typedef enum {
    ABSENT,
    PRESENT,
    DELETED,
} EntryKind;

#define define_map(type)\
\
typedef struct {\
    EntryKind kind;\
    StringSlice key;\
    type value;\
} entry_ ## type ;\
\
\
typedef struct map_ ## type map_ ## type ;\
\
\
struct map_ ## type {\
    usize capacity;\
    usize length;\
    usize deleted;\
    entry_ ## type *entries;\
\
    void (*free)(map_ ## type *mm);\
\
    bool (*insert)(map_ ## type *mm, const StringSlice *key, const type *value);\
    bool (*contains_key)(const map_ ## type *mm, const StringSlice *key);\
    type *(*get)(const map_ ## type *mm, const StringSlice *key);\
    bool (*remove)(map_ ## type *mm, const StringSlice *key, type *value);\
};\
\
\
static usize map_ ## type ## _insert_index(const entry_ ## type *entries, usize capacity, const StringSlice *key) {\
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
static void map_ ## type ## _resize(map_ ## type *mm, usize new_cap) {\
\
    entry_ ## type *entries = calloc(new_cap, sizeof(entry_ ## type));\
    for (usize i = 0; i < new_cap; ++i) {\
        entries[i].kind = ABSENT;\
    }\
\
    for (usize i = 0; i < mm->capacity; ++i) {\
        if (mm->entries[i].kind == PRESENT) {\
            usize j = map_ ## type ## _insert_index(entries, new_cap, &mm->entries[i].key);\
            entries[j].kind = PRESENT;\
            entries[j].key = mm->entries[i].key;\
            entries[j].value = mm->entries[i].value;\
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
static usize map_ ## type ## _search(const map_ ## type *mm, const StringSlice *key) {\
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
static bool map_ ## type ## _insert(map_ ## type *mm, const StringSlice *key, const type *value) {\
\
    if (mm->length + 1 >= MAX_LOAD_FACTOR * mm->capacity) {\
        usize new_cap = (mm->capacity > 0 ? mm->capacity * 2 : 1);\
        map_ ## type ## _resize(mm, new_cap);\
    }\
\
    usize i = map_ ## type ## _insert_index(mm->entries, mm->capacity, key);\
    switch (mm->entries[i].kind) {\
        case ABSENT: {\
            ++mm->length;\
            mm->entries[i].kind = PRESENT;\
            mm->entries[i].key = *key;\
            mm->entries[i].value = *value;\
            return true;\
        }\
        case PRESENT: {\
            return false;\
        }\
        case DELETED: {\
            ++mm->length;\
            --mm->deleted;\
            mm->entries[i].kind = PRESENT;\
            mm->entries[i].key = *key;\
            mm->entries[i].value = *value;\
            return true;\
        }\
    }\
}\
\
\
static bool map_ ## type ## _contains_key(const map_ ## type *mm, const StringSlice *key) {\
\
    if (mm->length == 0) {\
        return false;\
    }\
\
    usize i = map_ ## type ## _search(mm, key);\
    return i < mm->capacity;\
}\
\
\
static type* map_ ## type ## _get(const map_ ## type *mm, const StringSlice *key) {\
\
    if (mm->length == 0) {\
        return NULL;\
    }\
\
    usize i = map_ ## type ## _search(mm, key);\
    return (i < mm->capacity ? &mm->entries[i].value : NULL);\
}\
\
\
static bool map_ ## type ## _remove(map_ ## type *mm, const StringSlice *key, type *value) {\
\
    bool deleted = false;\
    usize i = map_ ## type ## _search(mm, key);\
    if (i < mm->capacity) {\
        deleted = true;\
        mm->entries[i].kind = DELETED;\
        ++mm->deleted;\
        --mm->length;\
\
        if (value != NULL) {\
            *value = mm->entries[i].value;\
        }\
    }\
\
    if (mm->length < mm->capacity * MIN_LOAD_FACTOR) {\
        map_ ## type ## _resize(mm, (usize) ((f64) mm->capacity * 0.5f));\
    }\
    if (mm->deleted > mm->capacity * MAX_DELETED_FACTOR) {\
        map_ ## type ## _resize(mm, mm->capacity);\
    }\
}\
\
\
static void map_ ## type ## _free(map_ ## type *mm) {\
\
    free(mm->entries);\
}\
\
\
static map_ ## type map_ ## type ## _new() {\
\
    map_ ## type mm = {0, 0, 0, NULL};\
\
    mm.free = &( map_ ## type ## _free);\
    mm.insert = &( map_ ## type ## _insert);\
    mm.contains_key = &( map_ ## type ## _contains_key);\
    mm.get = &( map_ ## type ## _get);\
    mm.remove = &( map_ ## type ## _remove);\
\
    return mm;\
}\

#define Map(type) map_ ## type

#define Entry(type) entry_ ## type

#define new_map(type) map_ ## type ## _new()

#endif //CHORA_MAP_H
