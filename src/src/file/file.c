//
// Created by francesco on 26/11/21.
//

#include "file.h"

// -------------------------------------------------
// PUBLIC API
// -------------------------------------------------

int init_file(File *f, const char *filename) {

    FILE *fptr = fopen(filename, "rb");
    if (fptr == NULL) {
        return -1;
    }

    if (fseek(fptr, 0, SEEK_END) == 0) {

        long buf_size = ftell(fptr);
        if (buf_size >= 0) {
            rewind(fptr);

            char *tmp = calloc(sizeof(char), buf_size + 1);
            if (tmp != NULL) {
                f->data = tmp;

                size_t len = fread(f->data, sizeof(char), buf_size, fptr);
                f->data[len] = '\0';
                f->len = len;

                f->filename = filename;
                f->lines = new_vector(u64);

                return fclose(fptr);
            }
        }
    }

    return -1;
}

int free_file(File *f) {
    free(f->data);
    f->lines.free(&f->lines);
}

// -------------------------------------------------

bool add_line(File *f, u64 offset) {

    u64 *last = f->lines.top(&f->lines);
    if (last == NULL || *last < offset) {
        f->lines.push(&f->lines, &offset);
    } else {
        return false;
    }
}

// -------------------------------------------------

const char *get_line_start(const File *f, u64 line) {

    u64 *offset = f->lines.get(&f->lines, line - 1);
    if (offset == NULL) {
        return NULL;
    } else {
        return f->data + *offset;
    }
}

