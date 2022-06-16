//
// Created by francesco on 26/11/21.
//

#include "buffer.h"

#include <stdlib.h>
#include <string.h>

// -------------------------------------------------
// PUBLIC API
// -------------------------------------------------

void init_buffer(FileBuffer *fb, File *file) {

    fb->file = file;
    fb->current = 0;
    fb->coord.line = 1;
    fb->coord.column = 1;
    add_line(fb->file, fb->current);
}

// -------------------------------------------------

char current_char(const FileBuffer *fb) {
    return fb->file->data[fb->current];
}

char next_char(FileBuffer *fb) {

    char c = current_char(fb);
    if (c == '\0') { // reached end of data
        return '\0';
    }

    ++fb->current;

    if (c == '\n') { // we move to a new line
        ++fb->coord.line;
        fb->coord.column = 1;
        add_line(fb->file, fb->current);
    } else {
        ++fb->coord.column;
    }

    return current_char(fb);
}

char peek_char(const FileBuffer *fb) {

    char c = current_char(fb);
    if (c == '\0') {
        return '\0';
    } else {
        return fb->file->data[fb->current + 1];
    }
}

char peek_char_index(const FileBuffer *fb, usize ahead) {

    if (fb->current + ahead < fb->file->len) {
        return fb->file->data[fb->current + ahead];
    } else {
        return '\0';
    }
}

// -------------------------------------------------

void get_lexeme(const FileBuffer *fb, StringSlice *str, u64 start, u64 end) {

    if (start > end || start >= fb->file->len || end >= fb->file->len) {
        init_string(str, NULL, 0);
    } else {
        init_string(str, fb->file->data + start, end - start + 1);
    }
}

// -------------------------------------------------

u64 get_offset(const FileBuffer *fb) {
    return fb->current;
}

const Coordinates *get_coord(const FileBuffer *fb) {
    return &fb->coord;
}