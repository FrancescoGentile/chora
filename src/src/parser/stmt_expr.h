//
// Created by francesco on 07/12/21.
//

#ifndef CHORA_STMT_EXPR_H
#define CHORA_STMT_EXPR_H

#include "../ast/element.h"

#include <stdbool.h>

// -------------------------------------------------

bool parse_expression(Parser *p, ASTExpr *e, Vector(TypeRef) *possible);

void parse_block(Parser *p, ASTBlock *b, Type *must_ret, Vector(TypeRef) *possible,
                 const StringSlice *alt_name, bool final);

Vector(TypeRef) create_possible_types(usize count, ...);

#endif //CHORA_STMT_EXPR_H
