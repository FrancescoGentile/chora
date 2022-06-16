//
// Created by francesco on 26/11/21.
//

#include "token.h"

#include <stdio.h>

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

void init_token(Token *t, TokenKind kind, const Coordinates *s, const Coordinates *e) {
    if (t != NULL) {
        t->kind = kind;
        t->span.start = *s;
        t->span.end = *e;
    }
}

void init_named_id(Token *t, const StringSlice *value, const Coordinates *s, const Coordinates *e) {
    if (t != NULL) {
        t->kind = TK_NAME_ID;
        t->span.start = *s;
        t->span.end = *e;
        t->id = *value;
    }
}

void init_op_id(Token *t, const StringSlice *value, const Coordinates *s, const Coordinates *e) {
    if (t != NULL) {
        t->kind = TK_OP_ID;
        t->span.start = *s;
        t->span.end = *e;
        t->id = *value;;
    }
}

void init_bool_token(Token *t, bool value, const Coordinates *s, const Coordinates *e) {
    if (t != NULL) {
        t->kind = TK_BOOL_LIT;
        t->span.start = *s;
        t->span.end = *e;
        t->bool_lit = value;
    }
}

void init_int_token(Token *t, u64 val, const StringSlice *suf, const Coordinates *s, const Coordinates *e) {
    if (t != NULL) {
        t->num_lit.overflown = false;
        t->kind = TK_NUM_LIT;
        t->num_lit.is_float = false;
        t->num_lit.i_value = val;
        t->num_lit.suffix = *suf;
        t->span.start = *s;
        t->span.end = *e;
    }
}

void init_float_token(Token *t, f64 val, const StringSlice *suf, const Coordinates *s, const Coordinates *e) {
    if (t != NULL) {
        t->num_lit.overflown = false;
        t->kind = TK_NUM_LIT;
        t->num_lit.is_float = true;
        t->num_lit.f_value = val;
        t->num_lit.suffix = *suf;
        t->span.start = *s;
        t->span.end = *e;
    }
}

// -------------------------------------------------

const char *token_kind_to_string(TokenKind tk) {
    switch (tk) {
        case TK_EOF: {
            static char *s = "";
            return s;
        }
        case TK_OPEN_PAREN: {
            static char *s = "(";
            return s;
        }
        case TK_CLOSE_PAREN: {
            static char *s = ")";
            return s;
        }
        case TK_OPEN_SQUARE: {
            static char *s = "[";
            return s;
        }
        case TK_CLOSE_SQUARE: {
            static char *s = "]";
            return s;
        }
        case TK_OPEN_CURLY: {
            static char *s = "{";
            return s;
        }
        case TK_CLOSE_CURLY: {
            static char *s = "}";
            return s;
        }
        case TK_COMMA: {
            static char *s = ",";
            return s;
        }
        case TK_SEMI: {
            static char *s = ";";
            return s;
        }
        case TK_COLON: {
            static char *s = ":";
            return s;
        }
        case TK_AT: {
            static char *s = "@";
            return s;
        }
        case TK_UNDERSCORE: {
            static char *s = "_";
            return s;
        }
        case TK_EQ: {
            static char *s = "=";
            return s;
        }
        case TK_NUM_LIT: {
            static char *s = "numeric literal";
            return s;
        }
        case TK_BOOL_LIT: {
            static char *s = "boolean literal";
            return s;
        }
        case TK_AS: {
            static char *s = "as";
            return s;
        }
        case TK_FN: {
            static char *s = "fn";
            return s;
        }
        case TK_IF: {
            static char *s = "if";
            return s;
        }
        case TK_ELSE: {
            static char *s = "else";
            return s;
        }
        case TK_LOOP: {
            static char *s = "loop";
            return s;
        }
        case TK_WHILE: {
            static char *s = "while";
            return s;
        }
        case TK_NAME_ID: {
            static char *s = "name-id";
            return s;
        }
        case TK_OP_ID: {
            static char *s = "operator";
            return s;
        }
        case TK_LET: {
            static char *s = "let";
            return s;
        }
        case TK_BREAK: {
            static char *s = "break";
            return s;
        }
        case TK_CONST: {
            static char *s = "const";
            return s;
        }
        case TK_STATIC: {
            static char *s = "static";
            return s;
        }
        case TK_MUT: {
            static char *s = "mut";
            return s;
        }
        case TK_RETURN: {
            static char *s = "return";
            return s;
        }
        default: {
            fprintf(stderr, "CODE-ERROR: add token kind in token_kind_to_string\n");
            exit(1);
        }
    }
}

const StringSlice *get_token_slice(const Token *t) {

    if (t == NULL) {
        return NULL;
    }

    switch (t->kind) {
        case TK_NAME_ID:
        case TK_OP_ID: {
            return &t->id;
        }
        case TK_AS:
        case TK_WHILE:
        case TK_LOOP:
        case TK_ELSE:
        case TK_IF:
        case TK_BREAK:
        case TK_RETURN:
        case TK_FN:
        case TK_MUT:
        case TK_STATIC:
        case TK_CONST:
        case TK_LET: {
            return &t->keyword;
        }
        default: {
            return NULL;
        }
    }
}

const Coordinates *get_token_start(const Token *t) {

    if (t == NULL) {
        return NULL;
    } else {
        return &t->span.start;
    }
}

const Coordinates *get_token_end(const Token *t) {

    if (t == NULL) {
        return NULL;
    } else {
        return &t->span.end;
    }
}