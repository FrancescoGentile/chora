//
// Created by francesco on 26/11/21.
//

#ifndef CHORA_FILE_H
#define CHORA_FILE_H

#include "../std/vector/u64.h"
#include "../std/types.h"

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    const char *filename;
    char *data;
    u64 len;
    Vector(u64) lines;
} File;

// -------------------------------------------------

int init_file(File *f, const char *filename);

int free_file(File *f);

// -------------------------------------------------

/// add a new line start offset
/// @returns true if the offset is added
/// @returns false if the offset was already present
bool add_line(File *f, u64 offset);

// -------------------------------------------------

const char *get_line_start(const File *f, u64 line);

#endif //CHORA_FILE_H
