//
// Created by francesco on 28/11/21.
//

#include "string.h"

#include <string.h>

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

usize hash_string(const char *data, usize len) {
    usize hash = 2166136261u;
    for (int i = 0; i < len; i++) {
        hash ^= (u8) data[i];
        hash *= 16777619;
    }
    return hash;
}

void init_string(StringSlice *str, const char *d, usize len) {
    str->data = d;
    str->len = len;
    str->hash = hash_string(str->data, len);
}

char *to_cstring(const StringSlice *str) {

    char *res;
    if (str == NULL || str->data == NULL) {
        res = (char *) calloc(1, sizeof(char));
        *res = '\0';
    } else {
        res = (char *) calloc(str->len + 1, sizeof(char));
        strncpy(res, str->data, str->len);
        res[str->len] = '\0';
    }

    return res;
}

// -------------------------------------------------

// NULL pointers to StringSlices and StringSlices of length 0 are considered equals
bool str_equals(const StringSlice *s1, const StringSlice *s2) {

    if (s1 == NULL && s2 == NULL) {
        return true;
    } else if (s1 == NULL || s2 == NULL || s1->len != s2->len || s1->hash != s2->hash) {
        return false;
    } else if (s1->len == 0 && s2->len == 0) {
        return true;
    } else {
        return strncmp(s1->data, s2->data, s1->len) == 0;
    }
}

bool cstr_equals(const StringSlice *s1, const char *s2) {

    if (s1 == NULL && s2 == NULL) {
        return true;
    } else if (s1 == NULL || s2 == NULL) {
        return false;
    } else if (s1->len == 0 && *s2 == '\0') {
        return true;
    } else {
        return strncmp(s1->data, s2, s1->len) == 0;
    }
}