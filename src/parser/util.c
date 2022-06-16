//
// Created by francesco on 08/12/21.
//

#include "util.h"

void parse_pattern(Parser *p, ASTPattern *pt) {

    switch (peek_kind(p->lex)) {
        case TK_UNDERSCORE: {
            pt->kind = PT_WILDCARD;
            next_token(p->lex, &pt->underscore);
            return;
        }
        case TK_NAME_ID: {
            pt->kind = PT_IDENTIFIER;
            pt->id.mutable = NULL;
            next_token(p->lex, &pt->id.identifier);
            return;
        }
        case TK_MUT: {
            pt->kind = PT_IDENTIFIER;
            pt->id.mutable = (Token *) calloc(1, sizeof(Token));
            next_token(p->lex, pt->id.mutable);

            if (check(p, TK_NAME_ID)) {
                next_token(p->lex, &pt->id.identifier);
                return;
            }

            free(pt->id.mutable);
            pt->id.mutable = NULL;
            // error - mut not followed by a name identifier
            // error handled in default
        }
        default: {
            // invalid pattern - we create a fake wildcard pattern
            post_error(p->eh, MSG_INVALID_PATTERN, peek_start_coordinates(p->lex), peek_end_coordinates(p->lex));
            pt->kind = PT_WILDCARD;
            // we skipp all the tokens until we find ':' or '=' which always follow a pattern
            while (!check(p, TK_SEMI) && !check(p, TK_EQ)) {
                next_token(p->lex, NULL);
            }
        }
    }
}

bool parse_type(Parser *p, ASTType *t) {

    switch (peek_kind(p->lex)) {
        case TK_NAME_ID: {
            t->kind = TY_IDENTIFIER;
            next_token(p->lex, &t->symbol);

            Type *type = search_type(p->st, get_token_slice(&t->symbol));
            if (type != NULL) {
                t->type = type;
                return true;
            } else {
                t->type = NULL;
                char *name = to_cstring(&t->symbol.id);
                post_error(p->eh, MSG_UNKNOWN_TYPE, get_token_start(&t->symbol), get_token_end(&t->symbol),
                           name);
                free(name);
                return false;
            }
        }
        case TK_OP_ID: {
            t->kind = TY_IDENTIFIER;
            next_token(p->lex, &t->symbol);

            Type *type = search_type(p->st, get_token_slice(&t->symbol));
            if (type != NULL) {
                t->type = type;
                return true;
            } else {
                t->type = NULL;
                char *name = to_cstring(get_token_slice(&t->symbol));
                post_error(p->eh, MSG_UNKNOWN_TYPE, get_token_start(&t->symbol), get_token_end(&t->symbol),
                           name);
                free(name);
                return false;
            }
        }
        case TK_UNDERSCORE: {
            t->kind = TY_INFERRED;
            next_token(p->lex, &t->symbol);
            t->type = NULL;
            return true;
        }
        case TK_OPEN_PAREN: {
            Token token;
            next_token(p->lex, &token);

            if (check(p, TK_CLOSE_PAREN)) {
                t->t_empty.open = token;
                next_token(p->lex, &t->t_empty.close);
                t->kind = TY_EMPTY;
                t->type = get_builtin_type(TYPE_EMPTY);
                return true;
            } else {
                t->t_paren.open = token;
                t->kind = TY_PAREN;
                t->t_paren.type = (ASTType *) calloc(1, sizeof(ASTType));

                bool valid = parse_type(p, t->t_paren.type);
                t->type = t->t_paren.type->type;
                // even if the closing parenthesis is absent, we consider the pattern valid
                consume_with_token(p, TK_CLOSE_PAREN, &t->t_paren.close);
                return valid;
            }
        }
        default: {
            Token token;
            peek_token(p->lex, &token);
            post_error(p->eh, MSG_EXPECTED_TYPE, get_token_start(&token), get_token_end(&token),
                       token_kind_to_string(token.kind));
            return false;
        }
    }
}


void parse_name_id(Parser *p, Token *t) {

    next_token(p->lex, t);
    if (t->kind != TK_NAME_ID) {
        fprintf(stderr, "Expected identifier but found %d\n", t->kind);
        exit(1);
    }
}