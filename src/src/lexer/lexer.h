//
// Created by francesco on 26/11/21.
//

#ifndef CHORA_LEXER_H
#define CHORA_LEXER_H

#include "../file/buffer.h"
#include "../error/handler.h"
#include "../std/map/map.h"
#include "token.h"

define_map(TokenKind)

typedef struct {
    FileBuffer *fb;
    ErrorHandler *eh;
    Map(TokenKind) keywords;

    bool valid;
    Token next;

    // this is used only for parsing a wrong expression
    // we need to check if an operator is an infix operator or a postfix operator
    // check the comments on expressions parsing for more details
    bool valid2;
    Token next2;
} Lexer;

// -------------------------------------------------

void init_lexer(Lexer *l, FileBuffer *fb, ErrorHandler *eh);

void free_lexer(Lexer *l);

// -------------------------------------------------

void next_token(Lexer *l, Token *t);

void peek_token(Lexer *l, Token *t);

TokenKind peek_kind(Lexer *l);

const Coordinates *peek_start_coordinates(Lexer *l);

const Coordinates *peek_end_coordinates(Lexer *l);

// -------------------------------------------------

TokenKind peek_kind_2(Lexer *l);

#endif //CHORA_LEXER_H
