//
// Created by francesco on 06/12/21.
//

#include "parser.h"
#include "stmt_expr.h"
#include "util.h"
#include "item.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static u8 INFIX_LEFT_POWER = 3;
static u8 INFIX_RIGHT_POWER = 4;

static u8 EQ_LEFT_POWER = 2;
static u8 EQ_RIGHT_POWER = 1;

static u8 AS_LEFT_POWER = 5;

static u8 POSTFIX_LEFT_POWER = 6;

/// -------------------------------------------------
/// PRIVATE FUNCTIONS
/// -------------------------------------------------

static bool pratt_parser(Parser *p, ASTExpr *e, Vector(TypeRef) *possible, u8 min_bp);

static bool parse_statement(Parser *p, ASTStmt *s, Vector(TypeRef) *possible);

static void parse_label_declaration(Parser *p, ASTLabel *l) {

    consume(p, TK_AT);

    if (check(p, TK_NAME_ID)) {
        next_token(p->lex, &l->name);

        if (consume_if(p, TK_COLON)) { // everything is ok
            return;
        }

    } else { // error in label name
        post_error(p->eh, MSG_LABEL_DECL_ERROR1, peek_start_coordinates(p->lex), peek_end_coordinates(p->lex));
        if (consume_if(p, TK_COLON)) {
            return;
        }
    }

    post_error(p->eh, MSG_LABEL_DECL_ERROR2, peek_start_coordinates(p->lex), peek_end_coordinates(p->lex));

    // after the label name we dit not find ':'
    // thus, we skip all tokens until we find ':' or '{'
    while (!check(p, TK_COLON) && !check(p, TK_OPEN_CURLY)) {
        next_token(p->lex, NULL);
    }

    if (check(p, TK_COLON)) {
        next_token(p->lex, NULL);
    }
}

static void parse_label_ref(Parser *p, ASTLabel *l) {

    consume(p, TK_AT);
    switch (peek_kind(p->lex)) {
        case TK_NAME_ID:
        case TK_LOOP:
        case TK_WHILE:
        case TK_IF:
        case TK_ELSE: {
            next_token(p->lex, &l->name);
            break;
        }
        default: {
            post_error(p->eh, MSG_LABEL_REF_ERROR2, peek_start_coordinates(p->lex), peek_end_coordinates(p->lex));
            next_token(p->lex, NULL);
        }
    }
}

/// -------------------------------------------------
/// EXPRESSIONS
/// -------------------------------------------------

static bool next_is_expression(Parser *p) {
    switch (peek_kind(p->lex)) {
        case TK_OPEN_PAREN:
        case TK_OPEN_CURLY:
        case TK_NUM_LIT:
        case TK_BOOL_LIT:
        case TK_NAME_ID:
        case TK_OP_ID:
        case TK_BREAK:
        case TK_RETURN:
        case TK_LOOP:
        case TK_WHILE: {
            return true;
        }
        default: {
            return false;
        }
    }
}

static bool parse_if(Parser *p, ASTIf *i, Type *must_ret, Vector(TypeRef) *possible);

static bool parse_else(Parser *p, ASTElse *e, Type *must_ret) {

    consume(p, TK_ELSE);

    if (check(p, TK_IF)) {
        e->kind = ELSE_IF;
        return parse_if(p, &e->else_if, must_ret, NULL);
    } else {
        e->kind = ELSE_BLOCK;

        StringSlice else_name;
        init_string(&else_name, token_kind_to_string(TK_ELSE), strlen(token_kind_to_string(TK_ELSE)));
        parse_block(p, &e->else_block, must_ret, NULL, &else_name, true);
        return true;
    }
}

static bool parse_if(Parser *p, ASTIf *i, Type *must_ret, Vector(TypeRef) *possible) {

    consume_with_token(p, TK_IF, &i->if_lex);

    i->condition = (ASTExpr *) calloc(1, sizeof(ASTExpr));

    Type *bool_type = get_builtin_type(TYPE_BOOL);
    Vector(TypeRef) cond_poss = create_possible_types(1, bool_type);
    bool valid_cond = parse_expression(p, i->condition, &cond_poss);
    cond_poss.free(&cond_poss);

    if (valid_cond) { // check condition only if the expression is valid
        // check if expression type is boolean -> if not post error
        Type *expr_type = get_expr_type(i->condition);
        if (expr_type != bool_type) {
            post_type_mismatch(p->eh, MSG_TYPE_MISMATCH, get_expr_start(i->condition), get_expr_end(i->condition),
                               bool_type, expr_type);
        }
    }

    i->if_body = (ASTBlock *) calloc(1, sizeof(ASTBlock));
    StringSlice if_name;
    init_string(&if_name, token_kind_to_string(TK_IF), strlen(token_kind_to_string(TK_IF)));

    parse_block(p, i->if_body, must_ret, possible, &if_name, true);

    Type *if_ret_type = get_block_type(i->if_body, true);

    bool valid;
    if (check(p, TK_ELSE)) {
        i->else_body = (ASTElse *) calloc(1, sizeof(ASTElse));
        valid = parse_else(p, i->else_body, if_ret_type);
    } else {
        i->else_body = NULL;

        if (if_ret_type != NULL && if_ret_type != get_builtin_type(TYPE_EMPTY)) {
            char *type_name = type_to_cstring(if_ret_type);
            post_error(p->eh, MSG_INCOMPLETE_IF, get_token_start(&i->if_lex), NULL, type_name);
            free(type_name);
            valid = false;
        } else {
            valid = true;
        }
    }

    return valid;
}

static void parse_loop(Parser *p, ASTLoop *l, Vector(TypeRef) *possible) {

    consume_with_token(p, TK_LOOP, &l->loop_lex);
    l->body = (ASTBlock *) calloc(1, sizeof(ASTBlock));

    StringSlice body_name;
    init_string(&body_name, token_kind_to_string(TK_LOOP), strlen(token_kind_to_string(TK_LOOP)));

    parse_block(p, l->body, NULL, possible, &body_name, false);
}

static void parse_while(Parser *p, ASTWhile *w) {

    consume_with_token(p, TK_WHILE, &w->while_lex);

    w->condition = (ASTExpr *) calloc(1, sizeof(ASTExpr));

    Type *bool_type = get_builtin_type(TYPE_BOOL);
    Vector(TypeRef) possible = create_possible_types(1, bool_type);
    bool valid_cond = parse_expression(p, w->condition, &possible);
    possible.free(&possible);

    if (valid_cond) {
        // check if expression type is boolean -> if not post error
        Type *expr_type = get_expr_type(w->condition);
        if (expr_type != bool_type) {
            post_type_mismatch(p->eh, MSG_TYPE_MISMATCH, get_expr_start(w->condition), get_expr_end(w->condition),
                               bool_type, expr_type);
        }
    }

    w->body = (ASTBlock *) calloc(1, sizeof(ASTBlock));

    StringSlice body_name;
    init_string(&body_name, token_kind_to_string(TK_WHILE), strlen(token_kind_to_string(TK_WHILE)));

    parse_block(p, w->body, get_builtin_type(TYPE_EMPTY), NULL, &body_name, false);
}

static void parse_return(Parser *p, ASTReturn *r) {

    consume_with_token(p, TK_RETURN, &r->ret_lex);

    if (check(p, TK_AT)) {
        r->label = (ASTLabel *) calloc(1, sizeof(ASTLabel));
        parse_label_ref(p, r->label);
    } else {
        r->label = NULL;
    }

    const StringSlice *label_string = get_label_string(r->label);
    Block *block = get_external_block(p->st, label_string);
    r->block = block;

    if (block == NULL) {
        // no block with the given name found
        // since we do not know the block, we parse the expression, but we do nothing

        if (r->label != NULL) {
            char *label_name = to_cstring(label_string);
            post_error(p->eh, MSG_LABEL_REF_ERROR1, get_token_start(&r->label->name), get_token_end(&r->label->name),
                       label_name);
            free(label_name);
        } else {
            //if the label is NULL and no block was found, it means that return is outside a block
            post_error(p->eh, MSG_RETURN_OUTSIDE_BLOCK, get_token_start(&r->ret_lex), get_token_end(&r->ret_lex));
        }

        if (next_is_expression(p)) {
            parse_expression(p, NULL, 0);
        }
    } else {
        // we know the block
        Type *ret;
        if (next_is_expression(p)) {
            r->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));

            bool valid_expr;
            if (block->must_return != NULL) {
                Vector(TypeRef) possible = create_possible_types(1, block->must_return);
                valid_expr = parse_expression(p, r->expr, &possible);
                possible.free(&possible);
            } else {
                valid_expr = parse_expression(p, r->expr, block->possible);
            }
            ret = (valid_expr ? get_expr_type(r->expr) : NULL);
        } else {
            r->expr = NULL;
            ret = get_builtin_type(TYPE_EMPTY);
        }

        // if ret == NULL, the expression is malformed, so we already posted an error
        if (ret != NULL) {
            if (block->must_return != NULL && ret != block->must_return) {
                post_type_mismatch(p->eh, MSG_RETURN_TYPE_ERROR2, get_token_start(&r->ret_lex),
                                   (r->expr != NULL ? get_expr_end(r->expr) : get_token_end(&r->ret_lex)),
                                   ret, block->must_return);

            } else if (block->possible != NULL) {
                for (usize i = 0; i < block->possible->length; ++i) {
                    if (ret == block->possible->entries[i]) {
                        // we may overwrite the previous must_return, but the type must be equal
                        block->must_return = ret;
                        return;
                    }
                }
            } else {
                block->must_return = ret;
            }
        }
    }
}

static void parse_break(Parser *p, ASTBreak *b) {

    consume_with_token(p, TK_BREAK, &b->break_lex);

    if (check(p, TK_AT)) {
        b->label = (ASTLabel *) calloc(1, sizeof(ASTLabel));
        parse_label_ref(p, b->label);
    } else {
        b->label = NULL;
    }

    const StringSlice *label_string = get_label_string(b->label);
    Block *block = get_internal_block(p->st, label_string);
    b->block = block;

    if (block == NULL) {
        // no block with the given name found
        // since we do not know the block, we parse the expression, but we do nothing

        if (b->label != NULL) {
            char *label_name = to_cstring(label_string);
            post_error(p->eh, MSG_LABEL_REF_ERROR1, get_token_start(&b->label->name), get_token_end(&b->label->name),
                       label_name);
            free(label_name);
        } else {
            //if the label is NULL and no block was found, it means that break is outside a block
            post_error(p->eh, MSG_BREAK_OUTSIDE_BLOCK, get_token_start(&b->break_lex), get_token_end(&b->break_lex));
        }

        if (next_is_expression(p)) {
            parse_expression(p, NULL, 0);
        }
    } else {
        // we know the block
        Type *ret;
        if (next_is_expression(p)) {
            b->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));

            bool valid_expr;
            if (block->must_return != NULL) {
                Vector(TypeRef) possible = create_possible_types(1, block->must_return);
                valid_expr = parse_expression(p, b->expr, &possible);
                possible.free(&possible);
            } else {
                valid_expr = parse_expression(p, b->expr, block->possible);
            }
            ret = (valid_expr ? get_expr_type(b->expr) : NULL);
        } else {
            b->expr = NULL;
            ret = get_builtin_type(TYPE_EMPTY);
        }

        if (ret != NULL) {
            if (block->must_return != NULL && ret != block->must_return) {
                post_type_mismatch(p->eh, MSG_RETURN_TYPE_ERROR2, get_token_start(&b->break_lex),
                                   (b->expr != NULL ? get_expr_end(b->expr) : get_token_end(&b->break_lex)),
                                   ret, block->must_return);
            } else if (block->possible != NULL) {
                for (usize i = 0; i < block->possible->length; ++i) {
                    if (ret == block->possible->entries[i]) {
                        // we may overwrite the previous must_return, but the type must be equal
                        block->must_return = ret;
                        return;
                    }
                }
            } else {
                block->must_return = ret;
            }
        }
    }
}

static Function *match_function(Parser *p, ASTFnCall *fc, Vector(TypeRef) *possible) {

    Vector(FunctionRef) *candidates = get_functions(p->st, get_token_slice(&fc->id));
    if (candidates == NULL) {
        char *name = to_cstring(get_token_slice(&fc->id));
        post_error(p->eh, MSG_FUNCTION_NOT_FOUND, get_token_start(&fc->id), get_token_end(&fc->id), name);
        free(name);
        return false;
    }

    // indicates if we found a matching function, whose return value appears in possible
    int found_with_return = 0;
    // indicates if we found a matching function, whose return value does not appear in possible
    int found_without_return = 0;
    Function *matched_fun = NULL;

    for (usize i = 0; i < candidates->length; ++i) {
        Function *fun = candidates->entries[i];

        bool match_ret = false;
        if (possible != NULL && possible->length > 0) {
            for (usize j = 0; j < possible->length; ++j) {
                if (fun->ret_type == possible->entries[j]) {
                    match_ret = true;
                    break;
                }
            }
        }

        // verify if it matches all the parameters
        const usize params_count = fun->params.length;
        bool *matched_params = (bool *) calloc(params_count, sizeof(bool));
        bool surely_not_match = false;
        for (usize j = 0; j < params_count; ++j) {
            matched_params[j] = false;
        }
        // indicates the next index of the next parameter that a non-named argument has to match
        usize next_param = 0;

        for (usize j = 0; j < fc->arguments.length; ++j) {
            ASTFnArgument *arg = fc->arguments.entries[j];

            if (arg->name == NULL) {
                // this is a non-named argument, so it has to match the next parameter
                if (next_param >= params_count) {
                    // the next parameter does not exist, so we terminate checking
                    surely_not_match = true;
                    break;
                } else {
                    FnParam *param = fun->params.entries[next_param];
                    if (get_expr_type(arg->expr) == param->type) {
                        matched_params[next_param] = true;
                        arg->arg_number = next_param;
                        ++next_param;
                    } else {
                        break;
                    }
                }
            } else {
                usize matched_pos = params_count;
                const StringSlice *arg_name = get_token_slice(arg->name);
                for (usize k = 0; k < params_count; ++k) {
                    FnParam *param = fun->params.entries[k];
                    const StringSlice *param_name = &param->name;

                    if (str_equals(arg_name, param_name) && get_expr_type(arg->expr) == param->type) {
                        if (!matched_params[k]) {
                            matched_pos = k;
                            matched_params[k] = true;
                            arg->arg_number = k;
                            next_param = k + 1;
                            break;
                        } else {
                            char *name = to_cstring(arg_name);
                            post_error(p->eh, MSG_PARAM_ALREADY_MATCHED, get_token_start(arg->name),
                                       get_token_end(arg->name), name);
                            free(name);
                        }
                    }
                }

                if (matched_pos == params_count) {
                    surely_not_match = true;
                    break;
                }
            }
        }

        usize matched_count = 0;
        if (!surely_not_match) {
            for (usize j = 0; j < params_count; ++j) {
                if (matched_params[j]) {
                    ++matched_count;
                } else {
                    FnParam *param = fun->params.entries[j];
                    if (param->expr != NULL || param->type == get_builtin_type(TYPE_EMPTY)) {
                        ++matched_count;
                        matched_params[j] = true;
                    }
                }
            }
        }

        free(matched_params);
        if (surely_not_match || matched_count != params_count) {
            continue;
        } else if (match_ret) {
            ++found_with_return;
            matched_fun = fun;
        } else {
            ++found_without_return;
            if (found_with_return == 0) {
                matched_fun = fun;
            }
        }
    }

    if (found_with_return > 1) {
        post_error(p->eh, MSG_FUNCTION_AMBIGUITY, get_token_start(&fc->id), get_token_end(&fc->id),
                   found_with_return);
        return NULL;
    } else if (found_with_return == 1) {
        return matched_fun;
    } else if (found_without_return > 1) {
        post_error(p->eh, MSG_FUNCTION_AMBIGUITY, get_token_start(&fc->id), get_token_end(&fc->id),
                   found_without_return);
        return NULL;
    } else if (found_without_return == 1) {
        return matched_fun;
    } else {
        post_error(p->eh, MSG_NO_FUNCTION_MATCH, get_token_start(&fc->id), get_token_end(&fc->id));
        return NULL;
    }
}

static void create_fn_call(ASTFnCall *fc, Function *fun) {

    fc->fun = fun;
    Vector(ASTFnArgumentRef) new_args = new_vector(ASTFnArgumentRef);
    const usize params_count = fun->params.length;
    new_args.reserve(&new_args, params_count);
    new_args.length = params_count;
    for (usize i = 0; i < params_count; ++i) {
        new_args.entries[i] = NULL;
    }

    for (usize i = 0; i < fc->arguments.length; ++i) {
        ASTFnArgument *arg = fc->arguments.entries[i];
        new_args.entries[arg->arg_number] = arg;
    }

    for (usize i = 0; i < params_count; ++i) {
        if (new_args.entries[i] == NULL) {
            ASTFnArgument *arg = (ASTFnArgument *) calloc(1, sizeof(ASTFnArgument));
            arg->name = NULL;
            arg->own_expr = false;
            arg->expr = fun->params.entries[i]->expr;
            arg->arg_number = i;
            new_args.entries[i] = arg;

        }
    }

    fc->arguments.free(&fc->arguments);
    fc->arguments = new_args;
}

static void parse_argument(Parser *p, ASTFnArgument *arg) {

    if (check(p, TK_NAME_ID)) {
        arg->name = (Token *) calloc(1, sizeof(Token));
        parse_name_id(p, arg->name);
        consume(p, TK_EQ);
    } else {
        arg->name = NULL;
    }

    arg->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
    arg->own_expr = true;
    parse_expression(p, arg->expr, NULL);
}

static bool parse_function_call(Parser *p, ASTFnCall *fc, Vector(TypeRef) *possible) {

    next_token(p->lex, &fc->id);
    consume(p, TK_OPEN_PAREN);

    fc->arguments = new_vector(ASTFnArgumentRef);
    while (!check(p, TK_CLOSE_PAREN)) {
        ASTFnArgument *arg = (ASTFnArgument *) calloc(1, sizeof(ASTFnArgument));
        parse_argument(p, arg);
        fc->arguments.push(&fc->arguments, &arg);

        if (!consume_if(p, TK_COMMA)) {
            break;
        }
    }

    consume(p, TK_CLOSE_PAREN);

    Function *matched = match_function(p, fc, possible);
    if (matched == NULL) {
        return false;
    } else {
        create_fn_call(fc, matched);
        return true;
    }
}

static bool parse_variable(Parser *p, ASTVariable *v) {

    next_token(p->lex, &v->id);

    Variable **var = get_variable(p->st, get_token_slice(&v->id));
    if (var == NULL) {
        char *name = to_cstring(get_token_slice(&v->id));
        post_error(p->eh, MSG_VARIABLE_NOT_FOUND, get_token_start(&v->id), get_token_end(&v->id), name);
        free(name);
        return false;
    } else {
        v->var = *var;
        // if a variable is declared but we cannot infer its type (because its type declaration is absent
        // or wrong) and because the initializing expression is wrong, we assign to its type NULL.
        // Since we do not know its type, we tell that the left expression is not valid so that
        // its type is not checked.
        return get_var_type(*var) != NULL;
    }
}

// the current token is a name identifier,
// so it could be a variable or a function call
static bool parse_name_identifier(Parser *p, ASTExpr *e, Vector(TypeRef) *possible) {

    if (peek_kind_2(p->lex) == TK_OPEN_PAREN) { // function
        e->kind = EXPR_FN_CALL;
        return parse_function_call(p, &e->e_fn_call, possible);
    } else { // variable
        e->kind = EXPR_VARIABLE;
        return parse_variable(p, &e->e_var);
    }
}

static void create_num_literal(Parser *p, ASTLiteral *l, Vector(TypeRef) *possible) {

    next_token(p->lex, &l->original);
    l->type = NULL;

    const StringSlice *suffix = &l->original.num_lit.suffix;
    if (suffix->len > 0) {
        Type *type = search_type(p->st, suffix);
        if (type == NULL) { // suffix not valid
            char *suffix_cstring = to_cstring(suffix);
            if (l->original.num_lit.is_float) {
                post_error(p->eh, MSG_FLOAT_SUFFIX_ERROR, get_token_start(&l->original), get_token_end(&l->original),
                           suffix_cstring);
            } else {
                post_error(p->eh, MSG_INT_SUFFIX_ERROR, get_token_start(&l->original), get_token_end(&l->original),
                           suffix_cstring);
            }
            free(suffix_cstring);
            l->type = NULL;
        } else {
            l->type = type;
            if (is_integer_type(type) && l->original.num_lit.is_float) {
                char *suffix_cstring = to_cstring(suffix);
                post_error(p->eh, MSG_FLOAT_SUFFIX_ERROR, get_token_start(&l->original), get_token_end(&l->original),
                           suffix_cstring);
                free(suffix_cstring);
                l->type = NULL;
            } else if (is_integer_type(type)) {
                l->kind = LIT_INT;
                l->int_lit = l->original.num_lit.i_value;
            } else {
                l->kind = LIT_FLOAT;
                l->float_lit = l->original.num_lit.f_value;
            }
        }
    }

    if (l->type == NULL && possible != NULL) {
        for (usize i = 0; i < possible->length; ++i) {
            if (is_float_type(possible->entries[i])) {
                l->kind = LIT_FLOAT;
                l->float_lit = (l->original.num_lit.is_float ? l->original.num_lit.f_value
                                                             : l->original.num_lit.i_value);
                l->type = possible->entries[i];
                break;
            } else if (!l->original.num_lit.is_float && is_integer_type(possible->entries[i])) {
                l->kind = LIT_INT;
                l->int_lit = l->original.num_lit.i_value;
                l->type = possible->entries[i];
                break;
            }
        }
    }

    if (l->type == NULL) {
        if (l->original.num_lit.is_float) {
            Type *t = get_builtin_type(TYPE_F64);
            if (t == NULL) {
                fprintf(stderr, "CODE-ERROR: searched for type f64 but not found in symbol table\n");
                exit(1);
            } else {
                l->type = t;
                l->kind = LIT_FLOAT;
                l->float_lit = l->original.num_lit.f_value;
            }
        } else {
            Type *t = get_builtin_type(TYPE_I32);
            if (t == NULL) {
                fprintf(stderr, "CODE-ERROR: searched for type i32 but not found in symbol table\n");
                exit(1);
            } else {
                l->type = t;
                l->kind = LIT_INT;
                l->int_lit = l->original.num_lit.i_value;
            }
        }
    }
}

static void create_bool_literal(Parser *p, ASTLiteral *l) {

    l->kind = LIT_BOOL;
    next_token(p->lex, &l->original);
    l->bool_lit = l->original.bool_lit;
    l->type = get_builtin_type(TYPE_BOOL);
}

static bool can_start_expression(TokenKind tk) {

    switch (tk) {
        case TK_NUM_LIT:
        case TK_BOOL_LIT:
        case TK_NAME_ID:
        case TK_OP_ID:
        case TK_OPEN_PAREN:
        case TK_OPEN_CURLY:
        case TK_LOOP:
        case TK_WHILE:
        case TK_IF:
        case TK_EQ:
        case TK_AS:
        case TK_RETURN:
        case TK_BREAK: {
            return true;
        }
        default: {
            return false;
        }
    }
}

static bool parse_prefix(Parser *p, ASTUnary *u) {

    next_token(p->lex, &u->op_id);

    u->is_prefix = true;
    u->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
    bool is_valid = pratt_parser(p, u->expr, NULL, 4);

    if (!is_valid) { // if the expression after the is_prefix operator wasn't valid, we don't do any other check
        return false;
    }

    Type *expr_type = get_expr_type(u->expr);
    Operator *op = get_prefix_operators(expr_type, &u->op_id.id);

    if (op == NULL) { // no operator available
        char *e_name = type_to_cstring(expr_type);
        char *op_name = to_cstring(&u->op_id.id);
        post_error(p->eh, MSG_NO_OPERATOR, get_token_start(&u->op_id), get_token_end(&u->op_id), e_name, op_name);
        free(e_name);
        free(op_name);
        is_valid = false;
    } else {
        u->op = op;
        is_valid = true;
    }

    return is_valid;
}

static bool parse_left(Parser *p, ASTExpr *e, Vector(TypeRef) *possible) {

    bool valid = true;
    switch (peek_kind(p->lex)) {
        case TK_NAME_ID: {
            valid = parse_name_identifier(p, e, possible);
            break;
        }
        case TK_NUM_LIT: {
            e->kind = EXPR_LITERAL;
            create_num_literal(p, &e->e_literal, possible);
            break;
        }
        case TK_BOOL_LIT: {
            e->kind = EXPR_LITERAL;
            create_bool_literal(p, &e->e_literal);
            break;
        }
        case TK_OP_ID: {
            e->kind = EXPR_UNARY;
            valid = parse_prefix(p, &e->e_unary);
            break;
        }
        case TK_RETURN: {
            e->kind = EXPR_RETURN;
            parse_return(p, &e->e_return);
            break;
        }
        case TK_BREAK: {
            e->kind = EXPR_BREAK;
            parse_break(p, &e->e_break);
            break;
        }
        case TK_OPEN_PAREN: {
            e->kind = EXPR_GROUPED;
            consume_with_token(p, TK_OPEN_PAREN, &e->e_grouped.open);
            if (check(p, TK_CLOSE_PAREN)) {
                e->e_grouped.expr = NULL;
            } else {
                e->e_grouped.expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
                valid = pratt_parser(p, e->e_grouped.expr, possible, 0);
            }
            consume_with_token(p, TK_CLOSE_PAREN, &e->e_grouped.close);
            break;
        }
        case TK_AT:
        case TK_OPEN_CURLY: {
            e->kind = EXPR_BLOCK;
            parse_block(p, &e->e_block, NULL, possible, NULL, true);
            break;
        }
        case TK_WHILE: {
            e->kind = EXPR_WHILE;
            parse_while(p, &e->e_while);
            break;
        }
        case TK_LOOP: {
            e->kind = EXPR_LOOP;
            parse_loop(p, &e->e_loop, possible);
            break;
        }
        case TK_IF: {
            e->kind = EXPR_IF;
            valid = parse_if(p, &e->e_if, NULL, possible);
            break;
        }
        default: {
            const char *found_name = token_kind_to_string(peek_kind(p->lex));
            post_error(p->eh, MSG_EXPECTED_EXPRESSION, peek_start_coordinates(p->lex), NULL, found_name);
            valid = false;
            TokenKind pk = peek_kind(p->lex);
            if (pk != TK_SEMI && pk != TK_CLOSE_CURLY) {
                next_token(p->lex, NULL);
            }
        }
    }

    return valid;
}

// TODO: modularize this function
// this function is extremely long (more than 300 lines with comments)
static bool pratt_parser(Parser *p, ASTExpr *e, Vector(TypeRef) *possible, u8 min_bp) {

    ASTExpr lhs;
    bool left_valid = parse_left(p, &lhs, possible);
    if (!left_valid) {
        free_ast_expr(&lhs);
    }
    lhs.is_valid = left_valid;

    while (true) {

        if (!left_valid) {
            // the left expression was not valid, so we can't get its type
            // we proceed in this way:
            // 1 - if we find an operator not followed by an expression, we assume it is a postfix operator
            // 2- if we find an operator followed by an expression, we assume it is an infix operator
            // and we parse the right expression
            // 3 - if it is the assignment operator, we parse the right expression
            // 4 - if it is the conversion operator, we parse the type
            // 5 - in all the other cases, we terminate the expression parsing

            if (check(p, TK_OP_ID)) {
                TokenKind tk = peek_kind_2(p->lex);

                if (!can_start_expression(tk)) { // FIRST case
                    next_token(p->lex, NULL);
                    lhs.kind = EXPR_UNARY;
                    continue;
                } else { // SECOND case
                    if (INFIX_LEFT_POWER < min_bp) {
                        break;
                    } else {
                        lhs.kind = EXPR_BINARY;
                        next_token(p->lex, NULL);

                        pratt_parser(p, NULL, NULL, INFIX_RIGHT_POWER);
                        continue;
                    }
                }
            } else if (check(p, TK_EQ)) { // THIRD case
                if (EQ_LEFT_POWER < min_bp) {
                    break;
                } else {
                    lhs.kind = EXPR_ASSIGN;

                    Token t;
                    next_token(p->lex, &t);

                    pratt_parser(p, NULL, NULL, EQ_RIGHT_POWER);

                    // assignment operator can be applied only if on the left there is a variable
                    // even though the left expression is not valid, we can still verify if its most probable kind is EXPR_VAR
                    // it is not we post an error
                    bool is_var = is_expr_var(&lhs);
                    if (is_var) {
                        continue;
                    } else {
                        post_error(p->eh, MSG_INVALID_LEFT_ASSIGN, get_token_start(&t), get_token_end(&t));
                    }
                }

            } else if (check(p, TK_AS)) { // FOURTH case
                if (AS_LEFT_POWER < min_bp) {
                    break;
                } else {
                    next_token(p->lex, NULL);
                    ASTType tmp;
                    parse_type(p, &tmp);
                    continue;
                }
            } else { // FIFTH case
                break;
            }
        } else {
            // the left expression is valid, so we can get its type
            Type *t_lhs = get_expr_type(&lhs);
            Token op_id;
            peek_token(p->lex, &op_id);

            if (op_id.kind == TK_AS) {
                if (AS_LEFT_POWER < min_bp) {
                    break;
                } else {
                    next_token(p->lex, NULL);

                    ASTType ast_type;
                    bool valid_type = parse_type(p, &ast_type);

                    if (!valid_type) {
                        free_ast_expr(&lhs);
                        lhs.kind = EXPR_ASSIGN;
                        left_valid = false;
                        continue;
                    } else if (is_inferred_type(&ast_type)) {
                        post_error(p->eh, MSG_TYPE_ANNOTATION, get_type_start(&ast_type), get_type_end(&ast_type));
                        free_ast_expr(&lhs);
                        lhs.kind = EXPR_ASSIGN;
                        left_valid = false;
                    } else {
                        Operator *op = get_conversion_operator(t_lhs, ast_type.type);
                        if (op == NULL) {
                            post_type_mismatch(p->eh, MSG_NO_CONVERSION, get_expr_start(&lhs), get_type_end(&ast_type),
                                               t_lhs, get_type(&ast_type));
                            free_ast_expr(&lhs);
                            lhs.kind = EXPR_AS;
                            left_valid = false;
                        } else {
                            ASTExpr *left_sub = (ASTExpr *) calloc(1, sizeof(ASTExpr));
                            *left_sub = lhs;

                            ASTType *type = (ASTType *) calloc(1, sizeof(ASTType));
                            *type = ast_type;

                            lhs.kind = EXPR_AS;
                            lhs.e_as.expr = left_sub;
                            lhs.e_as.type = type;
                            lhs.e_as.op = op;

                            continue;
                        }
                    }
                }

            } else if (op_id.kind == TK_EQ) {

                if (EQ_LEFT_POWER < min_bp) {
                    break;
                } else {
                    Token eq;
                    next_token(p->lex, &eq);

                    ASTExpr rhs;
                    Vector(TypeRef) allowed = create_possible_types(1, t_lhs);
                    bool right_valid = pratt_parser(p, &rhs, &allowed, EQ_RIGHT_POWER);
                    allowed.free(&allowed);

                    if (right_valid) {
                        Type *t_rhs = get_expr_type(&rhs);
                        bool assign_valid = true;

                        if (t_rhs != t_lhs) {
                            post_type_mismatch(p->eh, MSG_TYPE_MISMATCH, get_expr_start(&rhs), get_expr_end(&rhs),
                                               t_lhs, t_rhs);
                            assign_valid = false;
                        }

                        if (!is_expr_var(&lhs)) {
                            post_error(p->eh, MSG_INVALID_LEFT_ASSIGN, get_token_start(&eq), get_token_end(&eq));
                            assign_valid = false;
                        } else {
                            Variable *v = get_expr_var(&lhs);
                            bool is_mut = is_var_mutable(v);

                            if (!is_mut) {
                                assign_valid = false;
                                char *var_name = to_cstring(get_var_name(v));
                                post_error(p->eh, MSG_ASSIGN_IMMUTABLE, get_token_start(&eq),
                                           get_token_end(&eq), var_name);
                                free(var_name);
                            }
                        }

                        if (assign_valid) {
                            ASTExpr *left_sub = (ASTExpr *) calloc(1, sizeof(ASTExpr));
                            *left_sub = lhs;

                            ASTExpr *right_sub = (ASTExpr *) calloc(1, sizeof(ASTExpr));
                            *right_sub = rhs;

                            lhs.kind = EXPR_ASSIGN;
                            lhs.e_assign.eq = eq;
                            lhs.e_assign.left = left_sub;
                            lhs.e_assign.right = right_sub;

                            continue;

                        } else {
                            free_ast_expr(&lhs);
                            lhs.kind = EXPR_ASSIGN;
                            left_valid = false;
                            continue;
                        }

                    } else {

                        free_ast_expr(&rhs);
                        if (!is_expr_var(&lhs)) {
                            post_error(p->eh, MSG_INVALID_LEFT_ASSIGN, get_token_start(&eq), get_token_end(&eq));
                        }

                        lhs.kind = EXPR_ASSIGN;
                        continue;
                    }
                }
            } else if (op_id.kind == TK_OP_ID) {
                // this is a user defined or compiler provided operator
                // we have to verify if it is a postfix or infix operator

                Operator *op = get_postfix_operators(t_lhs, &op_id.id);
                if (op != NULL) { // it is a valid postfix operator for the left expression
                    if (POSTFIX_LEFT_POWER < min_bp) {
                        break;
                    } else {
                        next_token(p->lex, NULL);

                        ASTExpr *tmp = (ASTExpr *) calloc(1, sizeof(ASTExpr));
                        *tmp = lhs;

                        lhs.kind = EXPR_UNARY;
                        lhs.e_unary.op_id = op_id;
                        lhs.e_unary.op = op;
                        lhs.e_unary.expr = tmp;

                        t_lhs = op->ret_type;

                        continue;
                    }
                } else { // it is not a valid postfix operator, so we check if it is an infix operator

                    Vector(OperatorRef) ops = new_vector(OperatorRef);
                    get_infix_operators(t_lhs, &op_id.id, &ops);

                    if (ops.length > 0) { // there is at least one possible infix operator
                        if (INFIX_LEFT_POWER < min_bp) {
                            ops.free(&ops);
                            break;
                        } else {
                            next_token(p->lex, NULL);

                            // we parse the right expression
                            ASTExpr rhs;
                            // we set as possible types for the right expression the types of the second parameter
                            // of the valid infix operators we found
                            Vector(TypeRef) possible_right = new_vector(TypeRef);
                            for (usize i = 0; i < ops.length; ++i) {
                                Operator *infix_op = ops.entries[i];
                                possible_right.push(&possible_right, &infix_op->second);
                            }

                            bool right_valid = pratt_parser(p, &rhs, &possible_right, INFIX_RIGHT_POWER);
                            possible_right.free(&possible_right);

                            if (!right_valid) {
                                // the right expression was not correct, so we don't do other checks,
                                // but we simply set the new left expression to not valid, and we continue parsing
                                // in an error state (seen before)
                                free_ast_expr(&lhs);
                                free_ast_expr(&rhs);
                                left_valid = false;
                                continue;
                            } else {
                                // both the left and the right expressions are correct, so we verify if we can apply
                                // one of the found infix operators
                                Type *t_rhs = get_expr_type(&rhs);

                                Operator *infix_op = NULL;
                                for (usize i = 0; i < ops.length; ++i) {
                                    if (ops.entries[i]->second == t_rhs) {
                                        infix_op = ops.entries[i];
                                        break;
                                    }
                                }
                                ops.free(&ops);

                                if (infix_op != NULL) {
                                    // we found an operator
                                    ASTExpr *sub_left = (ASTExpr *) calloc(1, sizeof(ASTExpr));
                                    *sub_left = lhs;

                                    ASTExpr *sub_right = (ASTExpr *) calloc(1, sizeof(ASTExpr));
                                    *sub_right = rhs;

                                    lhs.kind = EXPR_BINARY;
                                    lhs.e_binary.left = sub_left;
                                    lhs.e_binary.right = sub_right;
                                    lhs.e_binary.op_id = op_id;
                                    lhs.e_binary.op = infix_op;

                                    continue;

                                } else {
                                    // we did not find an operator, that means that the right expression is valid,
                                    // but its type does not match with any of the second parameter types of the possible operators
                                    // we post the error, and we continue parsing setting the new left expression as not valid
                                    char *left_name = type_to_cstring(t_lhs);
                                    char *right_name = type_to_cstring(t_rhs);
                                    char *op_name = to_cstring(&op_id.id);
                                    post_error(p->eh, MSG_INFIX_OPERATOR_NOT_FOUND, get_token_start(&op_id),
                                               get_token_end(&op_id), op_name, left_name, right_name);
                                    free(left_name);
                                    free(right_name);
                                    free(op_name);

                                    free_ast_expr(&lhs);
                                    free_ast_expr(&rhs);
                                    left_valid = false;
                                }
                            }
                        }
                    } else {
                        ops.free(&ops);

                        // we found an operator-identifier that is neither a postfix operator nor an infix operator
                        // for the left expression. Thus, we signal this error (no operator found), we set the
                        // left expression as invalid and we continue parsing
                        char *left_name = type_to_cstring(t_lhs);
                        char *op_name = to_cstring(&op_id.id);
                        post_error(p->eh, MSG_NO_OPERATOR, get_token_start(&op_id), get_token_end(&op_id),
                                   left_name, op_name);
                        free(left_name);
                        free(op_name);

                        free_ast_expr(&lhs);
                        left_valid = false;
                        continue;
                    }
                }
            } else {
                // we found a token that signals the end of an expression -> we terminate parsing the expression
                break;
            }
        }
    }

    if (e != NULL) {
        *e = lhs;
        e->is_valid = left_valid;
    }
    return left_valid;
}

static void check_block(Parser *p, ASTBlock *b, bool return_value, bool final, usize return_pos) {

    Block *block = b->block;
    if (b->expr != NULL) {
        Block *expr_ret_block = get_expr_return_block(b->expr);
        if (expr_ret_block == NULL) {

            if (!final) {
                post_error(p->eh, MSG_EXPRESSION_NOT_ALLOWED, get_expr_start(b->expr), get_expr_end(b->expr));
            }

            Type *expr_type = get_expr_type(b->expr);
            if (block->must_return != NULL && block->must_return != expr_type) {
                post_type_mismatch(p->eh, MSG_RETURN_TYPE_ERROR2, get_expr_start(b->expr), get_expr_end(b->expr),
                                   expr_type, block->must_return);
            } else if (block->must_return == NULL) {
                block->must_return = expr_type;
            }

        }
    } else if (!return_value && !block->not_return && block->must_return != NULL && final &&
               block->must_return != get_builtin_type(TYPE_EMPTY)) {
        // in this case there is no instruction that interrupts the block execution flow and there is no final expression,
        // thus the block return type is '()', which may be in contrast with the block must_return type
        post_type_mismatch(p->eh, MSG_BLOCK_RETURN_EMPTY, get_token_start(&b->open), get_token_end(&b->close),
                           get_builtin_type(TYPE_EMPTY), block->must_return);
    } else if (!return_value && !block->not_return && block->must_return == NULL) {

        if (final) {
            block->must_return = get_builtin_type(TYPE_EMPTY);
        } else {
            block->must_return = get_builtin_type(TYPE_NO_RETURN);
        }
    }

    // in case the block is interrupted in the middle, we remove all the next statements that won't be executed
    if (return_pos < b->stmts.length && (return_value || block->not_return)) {
        for (usize i = return_pos + 1; i < b->stmts.length; ++i) {
            ASTStmt *st = b->stmts.entries[i];
            free_ast_stmt(st);
            free(st);
        }

        free_ast_expr(b->expr);
        free(b->expr);
        b->expr = NULL;

        b->stmts.length = return_pos + 1;
        block->last_is_jump = true;
    }
}

static void parse_inner_block(Parser *p, ASTBlock *b, Vector(TypeRef) *possible, bool final) {

    b->stmts = new_vector(ASTStmtRef);
    b->expr = NULL;
    // indicates if in the block there is a statement/expression that returns from this exact block (not an outermost one)
    bool return_value = false;
    // position of the first statement that interrupts the block execution flow
    usize return_pos = 0;

    while (true) {

        while (!check(p, TK_CLOSE_CURLY)) {

            ASTStmt *st = (ASTStmt *) calloc(1, sizeof(ASTStmt));
            bool is_expr = parse_statement(p, st, possible);

            if (is_expr) {
                b->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
                *b->expr = st->expr;
                free(st);

                const Block *ret_block = get_expr_return_block(b->expr);
                if (ret_block != NULL) {
                    if (!b->block->not_return && !return_value) {
                        return_pos = b->stmts.length;
                    }

                    if (ret_block->level < b->block->level && !return_value) {
                        b->block->not_return = true;
                    } else if (!b->block->not_return) {
                        return_value = true;
                    }
                }

                break;
            } else {
                b->stmts.push(&b->stmts, &st);
                const Block *ret_block = get_stmt_return_block(st);
                if (ret_block != NULL) {
                    if (!b->block->not_return && !return_value) {
                        return_pos = b->stmts.length - 1;
                    }

                    if (ret_block->level < b->block->level && !return_value) {
                        b->block->not_return = true;
                    } else if (!b->block->not_return) {
                        return_value = true;
                    }
                }
            }
        }

        // this is possible only if we have done the parsing of an expression which we thought was the last one.
        // but it is actually followed by other statements
        if (!check(p, TK_CLOSE_CURLY)) {
            post_token_mismatch(p->eh, MSG_UNEXPECTED_TOKEN, peek_start_coordinates(p->lex),
                                NULL, TK_SEMI, peek_kind(p->lex));

            // we convert the wrong final expression into a normal expression statement and we continue parsing the block
            ASTStmt *st = (ASTStmt *) calloc(1, sizeof(ASTStmt));
            st->kind = STMT_EXPR;
            st->expr = *b->expr;
            b->stmts.push(&b->stmts, &st);

            free(b->expr);
            b->expr = NULL;
        } else {
            next_token(p->lex, &b->close);
            break;
        }
    }

    check_block(p, b, return_value, final, return_pos);
}

/// -------------------------------------------------
/// STATEMENTS
/// -------------------------------------------------

static void create_local_var(Parser *p, ASTLocal *l, Type *type) {

    if (is_named_pattern(l->pattern)) {
        Variable *v = (Variable *) calloc(1, sizeof(Variable));
        init_local_variable(v, get_pattern_id(l->pattern), type, is_mutable_pattern(l->pattern));
        l->var = v;
        add_variable(p->st, v, true);
    } else { // pattern == '_'
        l->var = NULL; // user not interested in this variable
    }
}

static Type *parse_local_with_type(Parser *p, ASTLocal *l) {

    Type *var_type;

    l->type = (ASTType *) calloc(1, sizeof(ASTType));
    bool valid_type = parse_type(p, l->type);

    if (!valid_type) {

        while (!check(p, TK_EQ)) {
            next_token(p->lex, NULL);
        }
        consume(p, TK_EQ);

        // If the type is not valid, we check the expression type and we assign its type to the variable.
        // However, if the expression is malformed, we assign to the variable a NULL type (which means that
        // we don't know its actual type).

        // we do not save the expression since there won't be the code generation phase
        l->expr = NULL;
        ASTExpr expr;
        bool valid_expr = parse_expression(p, &expr, NULL);

        if (valid_expr) {
            var_type = get_expr_type(&expr);
        } else {
            var_type = NULL;
        }

    } else if (get_type(l->type) == get_builtin_type(TYPE_NO_RETURN)) {
        // we cannot assign to a variable a NO RETURN type
        post_error(p->eh, MSG_NO_RETURN_ASSIGNMENT, get_type_start(l->type), get_type_end(l->type));

        // also in this case we assign to the variable a NULL type so that it doesn't cause any more error
        var_type = NULL;

        consume(p, TK_EQ);
        // we do not consider the expression since there won't be the code generation phase
        l->expr = NULL;
        parse_expression(p, NULL, NULL);

    } else {
        // the declared type is a valid type
        consume(p, TK_EQ);

        l->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
        if (is_inferred_type(l->type)) {
            bool valid_expr = parse_expression(p, l->expr, NULL);
            if (valid_expr) {
                var_type = get_expr_type(l->expr);
            } else {
                var_type = NULL;
            }

        } else {
            Type *declared = get_type(l->type);
            Vector(TypeRef) possible = create_possible_types(1, declared);
            bool valid_expr = parse_expression(p, l->expr, &possible);
            possible.free(&possible);

            Type *expr_type = get_expr_type(l->expr);
            if (valid_expr && expr_type != declared) {
                post_type_mismatch(p->eh, MSG_TYPE_MISMATCH, get_expr_start(l->expr), get_expr_end(l->expr),
                                   declared, expr_type);
                var_type = declared;
            }
            var_type = declared;
        }
    }

    return var_type;
}

static Type *parse_local_without_type(Parser *p, ASTLocal *l) {

    Type *var_type;
    l->type = NULL;
    consume(p, TK_EQ);

    l->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
    bool valid_expr = parse_expression(p, l->expr, NULL);
    if (valid_expr) {
        Type *expr_type = get_expr_type(l->expr);

        if (expr_type == get_builtin_type(TYPE_NO_RETURN)) {
            post_error(p->eh, MSG_NO_RETURN_ASSIGNMENT, get_expr_start(l->expr), get_expr_end(l->expr));
            var_type = NULL;
        } else {
            var_type = expr_type;
        }
    } else {
        var_type = NULL;
    }
    return var_type;
}

static void parse_local(Parser *p, ASTLocal *l) {

    consume(p, TK_LET);

    l->pattern = (ASTPattern *) calloc(1, sizeof(ASTPattern));
    parse_pattern(p, l->pattern);

    Type *var_type;

    if (consume_if(p, TK_COLON)) { // type declared
        var_type = parse_local_with_type(p, l);
    } else {
        var_type = parse_local_without_type(p, l);
    }

    consume(p, TK_SEMI);

    create_local_var(p, l, var_type);
}

static bool parse_statement(Parser *p, ASTStmt *s, Vector(TypeRef) *possible) {

    while (true) {
        TokenKind tk = peek_kind(p->lex);
        switch (tk) {
            case TK_LET: {
                s->kind = STMT_LOCAL;
                parse_local(p, &s->local);
                return false;
            }
            case TK_SEMI: {
                next_token(p->lex, NULL);
                break;
            }
            case TK_CONST:
            case TK_STATIC:
            case TK_FN: {
                s->kind = STMT_ITEM;
                parse_item(p, &s->item);
                return false;
            }
            default: {
                s->kind = STMT_EXPR;
                bool valid_expr = parse_expression(p, &s->expr, possible);

                if (!valid_expr) {
                    consume_if(p, TK_SEMI);
                    return false;
                } else if (is_expr_with_block(&s->expr)) {
                    Type *expr_type = get_expr_type(&s->expr);
                    if (expr_type != get_builtin_type(TYPE_EMPTY) && expr_type != get_builtin_type(TYPE_NO_RETURN)) {
                        return true;
                    } else {
                        return false;
                    }
                } else if (!consume_if(p, TK_SEMI)) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    }
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

Vector(TypeRef) create_possible_types(usize count, ...) {

    Vector(TypeRef) vec = new_vector(TypeRef);
    va_list args;
    va_start(args, count);
    for (usize i = 0; i < count; ++i) {
        Type *t = va_arg(args, TypeRef);
        vec.push(&vec, &t);
    }
    return vec;
}

bool parse_expression(Parser *p, ASTExpr *e, Vector(TypeRef) *possible) {
    return pratt_parser(p, e, possible, 0);
}

void parse_block(Parser *p, ASTBlock *b, Type *must_ret, Vector(TypeRef) *possible,
                 const StringSlice *alt_name, bool final) {

    if (check(p, TK_AT)) {
        b->label = (ASTLabel *) calloc(1, sizeof(ASTLabel));
        parse_label_declaration(p, b->label);
    } else {
        b->label = NULL;
    }

    enter_scope(p->st);

    Block *block = (Block *) calloc(1, sizeof(Block));
    init_block(block, get_label_string(b->label), alt_name, must_ret, possible);
    add_block(p->st, block);
    b->block = block;

    consume_with_token(p, TK_OPEN_CURLY, &b->open);

    // we add must_return (if not NULL) to possible, so that the final expression may be parsed to
    // return the correct type (i.e. must_return)
    bool free_tmp = false;
    Vector(TypeRef) tmp;
    if (possible == NULL && must_ret != NULL) {
        free_tmp = true;
        tmp = new_vector(TypeRef);
        possible = &tmp;
        possible->push(possible, &must_ret);
    } else if (must_ret != NULL) {
        possible->push(possible, &must_ret);
    }

    parse_inner_block(p, b, possible, final);

    leave_scope(p->st);

    if (free_tmp) {
        possible->free(possible);
    } else if (possible != NULL && must_ret != NULL) {
        possible->pop(possible);
    }
}