//
// Created by francesco on 08/12/21.
//

#ifndef CHORA_UTIL_H
#define CHORA_UTIL_H

#include "parser.h"

void parse_pattern(Parser *p, ASTPattern *pt);

bool parse_type(Parser *p, ASTType *t);

void parse_name_id(Parser *p, Token *t);

#endif //CHORA_UTIL_H
