//
// Created by francesco on 28/11/21.
//

#include "handler.h"
#include "style.h"

#include <stdio.h>
#include <stdarg.h>

/// -------------------------------------------------
/// PRIVATE DATA
/// -------------------------------------------------

const u8 MAX_MSG_LENGTH = 100;

static char *msg_format[] = {
        "missing main function",
        "unknown start of token",
        "expected token '%s', but found '%s'",
        "expected expression, but found '%s'",
        "expected type, but found '%s'",
        "const cannot be mutable",
        "expected type '%s', but found type '%s'",
        "cannot initialize const with non literal values",
        "label name must be a valid name-identifier",
        "label declaration must be followed by a colon",
        "no label found with the name '%s'",
        "label reference must be a valid name-identifier, 'loop', 'while', 'if' or 'else",
        "invalid item starter",
        "invalid pattern",
        "type '%s' is unknown",
        "'%c' is not a valid digit in a %s number",
        "'%s' is not a valid integer suffix",
        "'%s' is not a valid float suffix",
        "cannot return type '%s' from block '%s'",
        "return type '%s' does not match previously declared type '%s'",
        "return outside of a block",
        "cannot break block '%s' with type '%s'",
        "breaking block '%s' with type '%s' that is different from the one used previously",
        "break outside of a block",
        "block implicitly returns '%s', but expected type '%s'",
        "cannot return '%s' from an incomplete if expression",
        "cannot find variable '%s' in this scope",
        "cannot find function '%s' in this scope",
        "no function matches",
        "%d overloaded functions match",
        "type '%s' has no operator '%s'",
        "type in function parameters has to be declared",
        "parameter '%s' already present in the parameter list",
        "type annotation needed",
        "missing final expression from a block with non empty return type",
        "no existing operator '%s' between '%s' and '%s'",
        "invalid left-hand side of assignment",
        "cannot assign twice to immutable variable '%s'",
        "no know conversion from '%s' to '%s'",
        "cannot declare a function inside a block",
        "a function with the same signature has already been declared",
        "type '!' cannot be assigned to any value",
        "final expression is not allowed inside a loop/while block",
        "argument '%s' already specified"
};

/// -------------------------------------------------
/// PRIVATE FUNCTIONS
/// -------------------------------------------------

// most efficient method to count digits of a positive integer
// see: https://stackoverflow.com/questions/1068849/how-do-i-determine-the-number-of-digits-of-an-integer-in-c
static u8 get_digit(u64 n) {

    if (n < 10u) return 1;
    if (n < 100u) return 2;
    if (n < 1000u) return 3;
    if (n < 10000u) return 4;
    if (n < 100000u) return 5;
    if (n < 1000000u) return 6;
    if (n < 10000000u) return 7;
    if (n < 100000000u) return 8;
    if (n < 1000000000u) return 9;
    if (n < 10000000000u) return 10;
    if (n < 100000000000u) return 11;
    if (n < 1000000000000u) return 12;
    if (n < 10000000000000u) return 13;
    if (n < 100000000000000u) return 14;
    if (n < 1000000000000000u) return 15;
    if (n < 10000000000000000u) return 16;
    if (n < 100000000000000000u) return 17;
    if (n < 1000000000000000000u) return 18;
    if (n < 10000000000000000000u) return 19;
    return 20;
}

static void print_one_line_message(const ErrorHandler *eh, const Message *m, u8 space) {

    const u64 start_line = m->span.start.line;
    const u64 start_col = m->span.start.column;
    const u64 end_col = m->span.end.column;

    fprintf(stderr, "%zu%*c| ", start_line, space - get_digit(start_line) + 1, ' ');
    fprintf(stderr, "%s", WHITE);
    const char *line = get_line_start(eh->file, start_line);
    for (; *line != '\0' && *line != '\n'; ++line) {
        fprintf(stderr, "%c", *line);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "%s%*c| ", BLUE, space + 1, ' ');
    if (start_col > 1) {
        fprintf(stderr, "%*c", (int) start_col - 1, ' ');
    }
    fprintf(stderr, "%s", RED);
    for (u64 i = start_col; i <= end_col; ++i) {
        fprintf(stderr, "^");
    }
    fprintf(stderr, "\n\n");
}

static void print_two_line_message(const ErrorHandler *eh, const Message *m, u8 space) {

    const u64 start_line = m->span.start.line;
    const u64 end_line = m->span.end.line;
    const u64 end_col = m->span.end.column;

    fprintf(stderr, "%zu%*c|%s/", start_line, space - get_digit(start_line) + 1, ' ', RED);
    fprintf(stderr, "%s ", WHITE);
    const char *first_line = get_line_start(eh->file, start_line);
    for (; *first_line != '\0' && *first_line != '\n'; ++first_line) {
        fprintf(stderr, "%c", *first_line);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "%s%*c|%s|\n", BLUE, space + 1, ' ', RED);

    fprintf(stderr, "%s%zu%*c|%s|", BLUE, end_line, space - get_digit(end_line) + 1, ' ', RED);
    fprintf(stderr, "%s ", WHITE);
    const char *second_line = get_line_start(eh->file, end_line);
    for (; *second_line != '\0' && *second_line != '\n'; ++second_line) {
        fprintf(stderr, "%c", *second_line);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "%s%*c|%s|_", BLUE, space + 1, ' ', RED);
    for (u64 i = 0; i < end_col - 1; ++i) {
        fprintf(stderr, "_");
    }
    fprintf(stderr, "%s^\n\n", RED);

}

static void print_multi_line_message(const ErrorHandler *eh, const Message *m, u8 space) {

    const u64 start_line = m->span.start.line;
    const u64 end_line = m->span.end.line;
    const u64 end_col = m->span.end.column;

    fprintf(stderr, "%zu%*c|%s/", start_line, space - get_digit(start_line) + 1, ' ', RED);
    fprintf(stderr, "%s ", WHITE);
    const char *first_line = get_line_start(eh->file, start_line);
    for (; *first_line != '\0' && *first_line != '\n'; ++first_line) {
        fprintf(stderr, "%c", *first_line);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "%s%*c|%s|\n", BLUE, space + 1, ' ', RED);
    fprintf(stderr, "%s...%*c%s|\n", BLUE, space - 1, ' ', RED);

    fprintf(stderr, "%s%zu%*c|%s|", BLUE, end_line, space - get_digit(end_line) + 1, ' ', RED);
    fprintf(stderr, "%s ", WHITE);
    const char *second_line = get_line_start(eh->file, end_line);
    for (; *second_line != '\0' && *second_line != '\n'; ++second_line) {
        fprintf(stderr, "%c", *second_line);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "%s%*c|%s|_", BLUE, space + 1, ' ', RED);
    for (usize i = 0; i < end_col - 1; ++i) {
        fprintf(stderr, "_");
    }
    fprintf(stderr, "%s^\n\n", RED);
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

void free_message(Message *m) {
    if (m == NULL) { return; }
    free(m->message);
}

// -------------------------------------------------

void init_handler(ErrorHandler *eh, File *file) {
    eh->file = file;
    eh->errors = new_vector(Message);
}

void free_handler(ErrorHandler *eh) {

    for (usize i = 0; i < eh->errors.length; ++i) {
        free_message(&eh->errors.entries[i]);
    }

    eh->errors.free(&eh->errors);
}

// -------------------------------------------------

bool any_error(const ErrorHandler *eh) {
    return !eh->errors.is_empty(&eh->errors);
}

// -------------------------------------------------

void post_error(ErrorHandler *eh, MessageKind mk, const Coordinates *s, const Coordinates *e, ...) {

    Message m;
    m.kind = mk;
    m.span.start = *s;
    m.span.end = (e != NULL ? *e : *s);
    m.message = (char *) calloc(MAX_MSG_LENGTH, sizeof(char));

    va_list args;
    va_start(args, e);
    vsnprintf(m.message, MAX_MSG_LENGTH, msg_format[mk], args);
    va_end(args);

    eh->errors.push(&eh->errors, &m);
}

void post_type_mismatch(ErrorHandler *eh, MessageKind mk, const Coordinates *s, const Coordinates *e,
                        Type *first, Type *second) {

    char *d_name = type_to_cstring(first);
    char *e_name = type_to_cstring(second);
    post_error(eh, mk, s, e, d_name, e_name);
    free(d_name);
    free(e_name);
}

void post_token_mismatch(ErrorHandler *eh, MessageKind mk, const Coordinates *s, const Coordinates *e,
                         TokenKind first, TokenKind second) {

    const char *f_name = token_kind_to_string(first);
    const char *s_name = token_kind_to_string(second);
    post_error(eh, mk, s, e, f_name, s_name);
}

// -------------------------------------------------

void print_messages(const ErrorHandler *eh) {

    const char *filename = eh->file->filename;

    for (usize i = 0; i < eh->errors.length; ++i) {
        const Message *m = &eh->errors.entries[i];
        u64 start_line = m->span.start.line;
        u64 start_col = m->span.start.column;
        u64 end_line = m->span.end.line;

        fprintf(stderr, "%s", BOLD);
        fprintf(stderr, "%serror%s: %s\n", RED, WHITE, m->message);
        fprintf(stderr, "%s", RESET);

        u64 max = (start_line > end_line ? start_line : end_line);
        u8 space = get_digit(max);
        if (start_line != end_line && space < 3) {
            space = 3;
        }

        fprintf(stderr, "%s%*c--> %s%s:%lu:%lu\n", BLUE, space + 1, ' ', WHITE, filename, start_line, start_col);
        fprintf(stderr, "%s%*c|\n", BLUE, space + 1, ' ');

        if (start_line == end_line) {
            print_one_line_message(eh, m, space);
        } else if (end_line == start_line + 1) {
            print_two_line_message(eh, m, space);
        } else {
            print_multi_line_message(eh, m, space);
        }

    }
}