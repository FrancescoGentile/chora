//
// Created by francesco on 26/11/21.
//

#ifndef CHORA_TOKEN_H
#define CHORA_TOKEN_H

#include "../file/coord.h"
#include "../std/string.h"

#include <stdbool.h>

typedef enum {
    TK_EOF, // '\0'

    /// DELIMITERS
    TK_OPEN_PAREN, // '('
    TK_CLOSE_PAREN, // ')'
    TK_OPEN_SQUARE, // '['
    TK_CLOSE_SQUARE, // ']'
    TK_OPEN_CURLY, // '{'
    TK_CLOSE_CURLY, // '}'

    /// PUNCTUATION
    TK_COMMA, // ','
    TK_SEMI, // ';'
    TK_COLON, // ':'
    TK_AT, // '@'
    TK_UNDERSCORE, // '_'

    /// OPERATORS
    TK_EQ, // '='

    /// LITERALS
    TK_NUM_LIT,
    TK_BOOL_LIT,

    /// KEYWORDS
    TK_AS, // 'as'
    TK_BREAK, // 'break'
    TK_CONST, // 'const'
    TK_ELSE, // 'else'
    TK_FN, // 'fn'
    TK_IF, // 'if'
    TK_LET, // 'let'
    TK_LOOP, // 'loop'
    TK_MUT, // 'mut'
    TK_RETURN, // 'return'
    TK_STATIC, // 'static'
    TK_WHILE, // 'while'

    /// USER DEFINED
    TK_NAME_ID,
    TK_OP_ID,

} TokenKind;

// -------------------------------------------------
// NUMBER LITERALS
// -------------------------------------------------

typedef struct {
    bool overflown;
    bool is_float;
    union {
        u64 i_value;
        f64 f_value;
    };
    StringSlice suffix;
} NumberLit;

// -------------------------------------------------

typedef struct {
    TokenKind kind;
    Span span;
    union {
        StringSlice id;
        StringSlice keyword;
        bool bool_lit;
        NumberLit num_lit;
    };
} Token;

// -------------------------------------------------

void init_token(Token *t, TokenKind kind, const Coordinates *s, const Coordinates *e);

void init_named_id(Token *t, const StringSlice *value, const Coordinates *s, const Coordinates *e);

void init_op_id(Token *t, const StringSlice *value, const Coordinates *s, const Coordinates *e);

void init_bool_token(Token *t, bool value, const Coordinates *s, const Coordinates *e);

void init_int_token(Token *t, u64 val, const StringSlice *suf, const Coordinates *s, const Coordinates *e);

void init_float_token(Token *t, f64 val, const StringSlice *suf, const Coordinates *s, const Coordinates *e);

// -------------------------------------------------

void print_token(const Token *t);

const char *token_kind_to_string(TokenKind tk);

const StringSlice *get_token_slice(const Token *t);

// -------------------------------------------------

const Coordinates *get_token_start(const Token *t);

const Coordinates *get_token_end(const Token *t);

#endif //CHORA_TOKEN_H
