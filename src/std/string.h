//
// Created by francesco on 28/11/21.
//

#ifndef CHORA_STRING_H
#define CHORA_STRING_H

#include "types.h"

#include <stdbool.h>

typedef struct {
    const char *data;
    usize len;
    usize hash;
} StringSlice;

// -------------------------------------------------

void init_string(StringSlice *str, const char *d, usize len);

char *to_cstring(const StringSlice *str);

usize hash_string(const char *data, usize len);

// -------------------------------------------------

bool str_equals(const StringSlice *s1, const StringSlice *s2);

bool cstr_equals(const StringSlice *s1, const char *s2);

#endif //CHORA_STRING_H
