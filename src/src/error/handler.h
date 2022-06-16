//
// Created by francesco on 26/11/21.
//

#ifndef CHORA_HANDLER_H
#define CHORA_HANDLER_H

#include "../std/vector.h"
#include "../file/coord.h"
#include "../file/file.h"
#include "../ast/element.h"
#include "../lexer/token.h"

typedef enum {
    MSG_MISSING_MAIN,
    MSG_UNKNOWN_TOKEN,
    MSG_UNEXPECTED_TOKEN,
    MSG_EXPECTED_EXPRESSION,
    MSG_EXPECTED_TYPE,
    MSG_CONST_MUT,
    MSG_TYPE_MISMATCH,
    MSG_CONST_INITIALIZER,
    MSG_LABEL_DECL_ERROR1,
    MSG_LABEL_DECL_ERROR2,
    MSG_LABEL_REF_ERROR1,
    MSG_LABEL_REF_ERROR2,
    MSG_INVALID_ITEM,
    MSG_INVALID_PATTERN,
    MSG_UNKNOWN_TYPE,
    MSG_INVALID_DIGIT,
    MSG_INT_SUFFIX_ERROR,
    MSG_FLOAT_SUFFIX_ERROR,
    MSG_RETURN_TYPE_ERROR1,
    MSG_RETURN_TYPE_ERROR2,
    MSG_RETURN_OUTSIDE_BLOCK,
    MSG_BREAK_TYPE_ERROR1,
    MSG_BREAK_TYPE_ERROR2,
    MSG_BREAK_OUTSIDE_BLOCK,
    MSG_BLOCK_RETURN_EMPTY,
    MSG_INCOMPLETE_IF,
    MSG_VARIABLE_NOT_FOUND,
    MSG_FUNCTION_NOT_FOUND,
    MSG_NO_FUNCTION_MATCH,
    MSG_FUNCTION_AMBIGUITY,
    MSG_NO_OPERATOR,
    MSG_FUNCTION_PARAM_TYPE,
    MSG_FUN_PARAM_REDEFINITION,
    MSG_TYPE_ANNOTATION,
    MSG_MISSING_FINAL_EXPRESSION,
    MSG_INFIX_OPERATOR_NOT_FOUND,
    MSG_INVALID_LEFT_ASSIGN,
    MSG_ASSIGN_IMMUTABLE,
    MSG_NO_CONVERSION,
    MSG_FUNCTION_INSIDE_BLOCK,
    MSG_DUPLICATE_FUNCTION,
    MSG_NO_RETURN_ASSIGNMENT,
    MSG_EXPRESSION_NOT_ALLOWED,
    MSG_PARAM_ALREADY_MATCHED,
} MessageKind;

typedef struct {
    MessageKind kind;
    char *message;
    Span span;
} Message;

void free_message(Message *m);

define_vector(Message)

typedef struct {
    File *file;
    Vector(Message) errors;
} ErrorHandler;

// -------------------------------------------------

void init_handler(ErrorHandler *eh, File *file);

void free_handler(ErrorHandler *eh);

// -------------------------------------------------

bool any_error(const ErrorHandler *eh);

// -------------------------------------------------

//void post_error(ErrorHandler *eh, MessageKind mk, const Coordinates *c, ...);

void post_error(ErrorHandler *eh, MessageKind mk, const Coordinates *s, const Coordinates *e, ...);

// function that can be used to post all errors regarding a type mismatch (not only MSG_TYPE_MISMATCH)
void post_type_mismatch(ErrorHandler *eh, MessageKind mk, const Coordinates *s, const Coordinates *e,
                        Type *first, Type *second);

void post_token_mismatch(ErrorHandler *eh, MessageKind mk, const Coordinates *s, const Coordinates *e,
                         TokenKind first, TokenKind second);

// -------------------------------------------------

void print_messages(const ErrorHandler *eh);

#endif //CHORA_HANDLER_H
