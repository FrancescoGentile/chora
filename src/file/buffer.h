//
// Created by francesco on 26/11/21.
//

#ifndef CHORA_BUFFER_H
#define CHORA_BUFFER_H

#include "file.h"
#include "coord.h"
#include "../std/types.h"
#include "../std/string.h"

// This is an abstraction to handle the file.
// Currently, all the file is uploaded into memory and saved into the File.
// Thus, this buffer is just an unnecessary level of abstraction.
// However, in precedent versions we used a two buffer strategy
// to avoid loading the entire file in memory and this abstraction was useful.
// In future versions, this may be modified.

typedef struct {
    File *file;

    /// offset of the current byte
    u64 current;

    /// coordinates of the current byte in the data
    Coordinates coord;

} FileBuffer;

// -------------------------------------------------

void init_buffer(FileBuffer *fb, File *file);

// -------------------------------------------------

/// @returns the current byte
char current_char(const FileBuffer *fb);

/// advances in the buffer of one byte
/// @returns the new current char
/// if the current position is over the end of the data, '\0' is returned
char next_char(FileBuffer *fb);

/// @returns the byte followinf the current one
char peek_char(const FileBuffer *fb);

char peek_char_index(const FileBuffer *fb, usize ahead);

// -------------------------------------------------

/// @returns the lexeme composed by the bites between [start, end]
/// @returns NULL if the specified offset are not correct or in case of an allocation error
void get_lexeme(const FileBuffer *fb, StringSlice *str, u64 start, u64 end);

// -------------------------------------------------

/// @returns the offset of the current byte
u64 get_offset(const FileBuffer *fb);

/// @returns the coordinates of the current byte in the data
const Coordinates *get_coord(const FileBuffer *fb);

#endif //CHORA_BUFFER_H
