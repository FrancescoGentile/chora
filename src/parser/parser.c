//
// Created by francesco on 03/12/21.
//

#include "parser.h"
#include "item.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

/// -------------------------------------------------
/// PRIVATE FUNCTIONS
/// -------------------------------------------------

static void parse_file(Parser *p, ASTFile *f) {

    do {
        if (peek_kind(p->lex) != TK_EOF) {
            ASTItem *it = (ASTItem *) calloc(1, sizeof(ASTItem));
            parse_item(p, it);
            f->items.push(&f->items, &it);
        } else {
            break;
        }
    } while (true);
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

bool init_parser(Parser *p, Lexer *l, SymbolTable *st, ErrorHandler *eh) {

    if (p == NULL || l == NULL || st == NULL || eh == NULL) {
        return false;
    }

    p->lex = l;
    p->st = st;
    p->eh = eh;
    p->has_main = false;

    return true;
}

bool consume(Parser *p, TokenKind tk) {

    Token t;
    peek_token(p->lex, &t);
    if (t.kind != tk) {
        post_error(p->eh, MSG_UNEXPECTED_TOKEN, get_token_start(&t), get_token_end(&t),
                   token_kind_to_string(tk), token_kind_to_string(t.kind));
        return false;
    } else {
        next_token(p->lex, NULL);
        return true;
    }
}

bool consume_with_token(Parser *p, TokenKind tk, Token *t) {

    peek_token(p->lex, t);
    if (t->kind != tk) {
        post_token_mismatch(p->eh, MSG_UNEXPECTED_TOKEN, get_token_start(t), get_token_end(t),
                            tk, t->kind);
        return false;
    } else {
        next_token(p->lex, NULL);
        return true;
    }
}

bool consume_if(Parser *p, TokenKind tk) {
    bool equal = (peek_kind(p->lex) == tk);
    if (equal) {
        next_token(p->lex, NULL);
    }
    return equal;
}

bool check(Parser *p, TokenKind tk) {
    return peek_kind(p->lex) == tk;
}

ASTFile *generate_ast(Parser *p) {

    ASTFile *f = (ASTFile *) calloc(1, sizeof(ASTFile));
    f->items = new_vector(ASTItemRef);
    parse_file(p, f);

    if (!p->has_main) {
        Coordinates c = (Coordinates) {1, 1};
        post_error(p->eh, MSG_MISSING_MAIN, &c, NULL);
    }

    return f;
}