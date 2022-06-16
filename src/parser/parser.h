//
// Created by francesco on 03/12/21.
//

#ifndef CHORA_PARSER_H
#define CHORA_PARSER_H

#include "../lexer/lexer.h"
#include "../error/handler.h"
#include "../ast/ast.h"
#include "../ast/table.h"

typedef struct {
    Lexer *lex;
    SymbolTable *st;
    ErrorHandler *eh;

    // this is used to check if the file has a main function, which is mandatory
    // if not, at the end of the parsing phase, we show an error
    bool has_main;
} Parser;

// -------------------------------------------------

bool init_parser(Parser *p, Lexer *l, SymbolTable *st, ErrorHandler *eh);

bool consume(Parser *p, TokenKind tk);

bool consume_with_token(Parser *p, TokenKind tk, Token *t);

bool consume_if(Parser *p, TokenKind tk);

bool check(Parser *p, TokenKind tk);

// -------------------------------------------------

ASTFile *generate_ast(Parser *p);

#endif //CHORA_PARSER_H
