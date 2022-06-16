//
// Created by francesco on 08/12/21.
//

#include "item.h"
#include "util.h"
#include "stmt_expr.h"

/// -------------------------------------------------
/// PRIVATE FUNCTIONS
/// -------------------------------------------------

/// -------------------------------------------------
/// FUNCTION ITEM
/// -------------------------------------------------

static bool create_fun_param(Parser *p, ASTFnParam *fp, Type *type, usize pos) {

    FnParam *param = (FnParam *) calloc(1, sizeof(FnParam));
    param->expr = fp->expr;
    param->type = type;

    if (is_named_pattern(fp->pattern)) {
        param->name = *get_pattern_id(fp->pattern);
        param->is_mutable = is_mutable_pattern(fp->pattern);

        Variable *var = (Variable *) calloc(1, sizeof(Variable));
        init_param_variable(var, &param->name, param->type, param->is_mutable, pos);
        param->var = var;

        if (!add_variable(p->st, var, false)) {
            char *param_str = to_cstring(&param->name);
            post_error(p->eh, MSG_FUN_PARAM_REDEFINITION, get_pattern_id_start(fp->pattern),
                       get_pattern_id_end(fp->pattern), param_str);
            free(param_str);
        }

    } else {
        init_string(&param->name, NULL, 0);
        param->is_mutable = false;
        param->var = NULL;
    }

    fp->param = param;
}

static Type *parse_param_invalid_type(Parser *p, ASTFnParam *fp) {

    // if the type declared is not valid, we have two possibilities:
    // - if the parameter has a default value, we assign to it the expression type
    // (if also the expression is malformed, we assign to it a NULL type)
    // - if the parameter has no default value we assign to it a NULL type

    TokenKind tk = peek_kind(p->lex);
    while (tk != TK_EQ && tk != TK_COMMA && tk != TK_CLOSE_PAREN) {
        next_token(p->lex, NULL);
        tk = peek_kind(p->lex);
    }

    Type *param_type;
    if (consume_if(p, TK_EQ)) {
        fp->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
        bool valid_expr = parse_expression(p, fp->expr, NULL);

        if (valid_expr) {
            Type *expr_type = get_expr_type(fp->expr);

            if (expr_type == get_builtin_type(TYPE_NO_RETURN)) {
                post_error(p->eh, MSG_NO_RETURN_ASSIGNMENT, get_expr_start(fp->expr), get_expr_end(fp->expr));
                param_type = NULL;
            } else {
                param_type = expr_type;
            }
        } else {
            param_type = NULL;
        }

    } else {
        fp->expr = NULL;
        param_type = NULL;
    }

    return param_type;
}

static Type *parse_param_valid_type(Parser *p, ASTFnParam *fp) {

    Type *param_type;

    if (is_inferred_type(fp->type)) {
        post_error(p->eh, MSG_FUNCTION_PARAM_TYPE, get_type_start(fp->type), get_type_end(fp->type));

        if (consume_if(p, TK_EQ)) {
            fp->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
            bool valid_expr = parse_expression(p, fp->expr, NULL);

            if (valid_expr) {
                Type *expr_type = get_expr_type(fp->expr);
                if (expr_type == get_builtin_type(TYPE_NO_RETURN)) {
                    post_error(p->eh, MSG_NO_RETURN_ASSIGNMENT, get_expr_start(fp->expr), get_expr_end(fp->expr));
                    param_type = NULL;
                } else {
                    param_type = expr_type;
                }
            } else {
                param_type = NULL;
            }
        }

    } else if (consume_if(p, TK_EQ)) {
        fp->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));

        Type *declared = get_type(fp->type);
        Vector(TypeRef) possible = create_possible_types(1, declared);
        bool valid_expr = parse_expression(p, fp->expr, &possible);
        possible.free(&possible);

        Type *expr_type = get_expr_type(fp->expr);
        if (valid_expr && expr_type != declared) {
            post_type_mismatch(p->eh, MSG_TYPE_MISMATCH, get_expr_start(fp->expr), get_expr_end(fp->expr),
                               declared, expr_type);
        }
        param_type = declared;

    } else {
        fp->expr = NULL;
        param_type = get_type(fp->type);
    }

    return param_type;
}

static void parse_fun_param(Parser *p, ASTFnParam *fp, usize pos) {

    fp->pattern = (ASTPattern *) calloc(1, sizeof(ASTPattern));
    parse_pattern(p, fp->pattern);

    consume(p, TK_COLON);

    fp->type = (ASTType *) calloc(1, sizeof(ASTType));
    bool valid_type = parse_type(p, fp->type);

    Type *param_type;
    if (!valid_type) {
        param_type = parse_param_invalid_type(p, fp);
    } else if (get_type(fp->type) == get_builtin_type(TYPE_NO_RETURN)) {
        post_error(p->eh, MSG_NO_RETURN_ASSIGNMENT, get_type_start(fp->type), get_type_end(fp->type));

        if (consume_if(p, TK_EQ)) {
            fp->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
            parse_expression(p, fp->expr, NULL);
        } else {
            fp->expr = NULL;
        }
        param_type = NULL;
    } else {
        param_type = parse_param_valid_type(p, fp);
    }

    create_fun_param(p, fp, param_type, pos);
}

static void create_function(Parser *p, ASTFunction *f, Type *ret_type) {

    Function *fun = (Function *) calloc(1, sizeof(Function));
    init_function(fun, get_token_slice(&f->name), ret_type);
    for (usize i = 0; i < f->params.length; ++i) {
        fun->params.push(&fun->params, &f->params.entries[i]->param);
    }
    f->fun = fun;

    if (!add_function(p->st, fun)) {
        post_error(p->eh, MSG_DUPLICATE_FUNCTION, get_token_start(&f->name), get_token_end(&f->name));
        return;
    }

    if (cstr_equals(get_token_slice(&f->name), "main")) {
        p->has_main = true;
    }
}

static void parse_function(Parser *p, ASTFunction *f) {

    if (get_scope_level(p->st) > 0) {
        post_error(p->eh, MSG_FUNCTION_INSIDE_BLOCK, peek_start_coordinates(p->lex), NULL);
    }

    consume_with_token(p, TK_FN, &f->fn_lex);
    parse_name_id(p, &f->name);
    consume(p, TK_OPEN_PAREN);

    enter_scope(p->st);

    f->params = new_vector(ASTFnParamRef);
    usize pos = 0;
    while (!check(p, TK_CLOSE_PAREN)) {
        ASTFnParam *fp = (ASTFnParam *) calloc(1, sizeof(ASTFnParam));
        parse_fun_param(p, fp, pos);
        f->params.push(&f->params, &fp);
        ++pos;

        if (!consume_if(p, TK_COMMA)) {
            break;
        }
    }

    consume(p, TK_CLOSE_PAREN);

    Type *ret_type;
    if (consume_if(p, TK_COLON)) {
        f->ret_type = (ASTType *) calloc(1, sizeof(ASTType));
        bool valid_type = parse_type(p, f->ret_type);
        if (valid_type) {
            ret_type = get_type(f->ret_type);
        } else {
            ret_type = NULL;
        }
    } else if (check(p, TK_OPEN_CURLY) || check(p, TK_AT)) {
        f->ret_type = NULL;
        ret_type = get_builtin_type(TYPE_EMPTY);
    } else {
        f->ret_type = NULL;
        ret_type = NULL;
    }

    create_function(p, f, ret_type);

    f->body = (ASTBlock *) calloc(1, sizeof(ASTBlock));
    parse_block(p, f->body, f->fun->ret_type, NULL, &f->name.id, true);

    leave_scope(p->st);
}

/// -------------------------------------------------
/// STATIC ITEM
/// -------------------------------------------------

static void create_static(Parser *p, ASTStatic *s, Type *type) {

    if (is_named_pattern(s->pattern)) {
        Variable *v = (Variable *) calloc(1, sizeof(Variable));
        init_static_variable(v, get_pattern_id(s->pattern), type, is_mutable_pattern(s->pattern));
        s->var = v;
        add_variable(p->st, v, true);
    } else { // pattern == '_'
        s->var = NULL; // user not interested in this variable
    }
}

static Type *parse_static_with_type(Parser *p, ASTStatic *s) {

    Type *var_type;

    s->type = (ASTType *) calloc(1, sizeof(ASTType));
    bool valid_type = parse_type(p, s->type);

    if (!valid_type) {

        while (!check(p, TK_EQ)) {
            next_token(p->lex, NULL);
        }
        consume(p, TK_EQ);

        // If the type is not valid, we check the expression type and we assign its type to the variable.
        // However, if the expression is malformed, we assign to the variable a NULL type (which means that
        // we don't know its actual type).

        // we do not save the expression since there won't be the code generation phase
        s->expr = NULL;
        ASTExpr expr;
        bool valid_expr = parse_expression(p, &expr, NULL);

        if (valid_expr) {
            var_type = get_expr_type(&expr);
        } else {
            var_type = NULL;
        }

    } else if (get_type(s->type) == get_builtin_type(TYPE_NO_RETURN)) {
        // we cannot assign to a variable a NO RETURN type
        post_error(p->eh, MSG_NO_RETURN_ASSIGNMENT, get_type_start(s->type), get_type_end(s->type));

        // also in this case we assign to the variable a NULL type so that it doesn't cause any more error
        var_type = NULL;

        consume(p, TK_EQ);
        // we do not consider the expression since there won't be the code generation phase
        s->expr = NULL;
        parse_expression(p, NULL, NULL);

    } else {
        // the declared type is a valid type
        consume(p, TK_EQ);

        s->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
        if (is_inferred_type(s->type)) {
            bool valid_expr = parse_expression(p, s->expr, NULL);
            if (valid_expr) {
                var_type = get_expr_type(s->expr);
            } else {
                var_type = NULL;
            }

        } else {
            Type *declared = get_type(s->type);
            Vector(TypeRef) possible = create_possible_types(1, declared);
            bool valid_expr = parse_expression(p, s->expr, &possible);
            possible.free(&possible);

            Type *expr_type = get_expr_type(s->expr);
            if (valid_expr && expr_type != declared) {
                post_type_mismatch(p->eh, MSG_TYPE_MISMATCH, get_expr_start(s->expr), get_expr_end(s->expr),
                                   declared, expr_type);
            }
            var_type = declared;
        }
    }

    return var_type;
}

static Type *parse_static_without_type(Parser *p, ASTStatic *s) {

    Type *var_type;
    s->type = NULL;
    consume(p, TK_EQ);

    s->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
    bool valid_expr = parse_expression(p, s->expr, NULL);
    if (valid_expr) {
        Type *expr_type = get_expr_type(s->expr);

        if (expr_type == get_builtin_type(TYPE_NO_RETURN)) {
            post_error(p->eh, MSG_NO_RETURN_ASSIGNMENT, get_expr_start(s->expr), get_expr_start(s->expr));
            var_type = NULL;
        } else {
            var_type = expr_type;
        }
    } else {
        var_type = NULL;
    }

    return var_type;
}

static void parse_static(Parser *p, ASTStatic *s) {

    consume(p, TK_STATIC);

    s->pattern = (ASTPattern *) calloc(1, sizeof(ASTPattern));
    parse_pattern(p, s->pattern);

    Type *var_type;

    if (consume_if(p, TK_COLON)) { // type declared
        var_type = parse_static_with_type(p, s);
    } else {
        var_type = parse_static_without_type(p, s);
    }

    consume(p, TK_SEMI);

    create_static(p, s, var_type);
}

/// -------------------------------------------------
/// CONST ITEM
/// -------------------------------------------------

static void create_const(Parser *p, ASTConst *c, Type *type) {

    if (is_named_pattern(c->pattern)) {
        Variable *v = (Variable *) calloc(1, sizeof(Variable));
        init_const_variable(v, get_pattern_id(c->pattern), type);
        c->var = v;
        add_variable(p->st, v, true);
    } else { // pattern == '_'
        c->var = NULL; // user not interested in this variable
    }
}

static Type *parse_const_with_type(Parser *p, ASTConst *c) {

    Type *var_type;

    c->type = (ASTType *) calloc(1, sizeof(ASTType));
    bool valid_type = parse_type(p, c->type);

    if (!valid_type) {

        while (!check(p, TK_EQ)) {
            next_token(p->lex, NULL);
        }
        consume(p, TK_EQ);

        // If the type is not valid, we check the expression type and we assign its type to the variable.
        // However, if the expression is malformed, we assign to the variable a NULL type (which means that
        // we don't know its actual type).

        // we do not save the expression since there won't be the code generation phase
        c->expr = NULL;
        ASTExpr expr;
        bool valid_expr = parse_expression(p, &expr, NULL);

        if (valid_expr) {
            var_type = get_expr_type(&expr);
            if (expr.kind != EXPR_LITERAL) {
                post_error(p->eh, MSG_CONST_INITIALIZER, get_expr_start(&expr), get_expr_end(&expr));
            }
        } else {
            var_type = NULL;
        }

    } else if (get_type(c->type) == get_builtin_type(TYPE_NO_RETURN)) {
        // we cannot assign to a variable a NO RETURN type
        post_error(p->eh, MSG_NO_RETURN_ASSIGNMENT, get_type_start(c->type), get_type_end(c->type));

        // also in this case we assign to the variable a NULL type so that it doesn't cause any more error
        var_type = NULL;

        consume(p, TK_EQ);
        // we do not consider the expression since there won't be the code generation phase
        c->expr = NULL;

        ASTExpr expr;
        bool valid_expr = parse_expression(p, &expr, NULL);

        if (valid_expr && expr.kind != EXPR_LITERAL) {
            post_error(p->eh, MSG_CONST_INITIALIZER, get_expr_start(&expr), get_expr_end(&expr));
        }

    } else {
        // the declared type is a valid type
        consume(p, TK_EQ);

        c->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
        if (is_inferred_type(c->type)) {
            bool valid_expr = parse_expression(p, c->expr, NULL);
            if (valid_expr) {
                var_type = get_expr_type(c->expr);
            } else {
                var_type = NULL;
            }

        } else {
            Type *declared = get_type(c->type);
            Vector(TypeRef) possible = create_possible_types(1, declared);
            bool valid_expr = parse_expression(p, c->expr, &possible);
            possible.free(&possible);

            if (valid_expr) {
                if (c->expr->kind != EXPR_LITERAL) {
                    post_error(p->eh, MSG_CONST_INITIALIZER, get_expr_start(c->expr), get_expr_end(c->expr));
                }

                Type *expr_type = get_expr_type(c->expr);
                if (expr_type != declared) {
                    post_type_mismatch(p->eh, MSG_TYPE_MISMATCH, get_expr_start(c->expr), get_expr_end(c->expr),
                                       declared, expr_type);
                }
            }
            var_type = declared;
        }
    }

    return var_type;
}

static Type *parse_const_without_type(Parser *p, ASTConst *c) {

    Type *var_type;
    c->type = NULL;
    consume(p, TK_EQ);

    c->expr = (ASTExpr *) calloc(1, sizeof(ASTExpr));
    bool valid_expr = parse_expression(p, c->expr, NULL);
    if (valid_expr) {
        Type *expr_type = get_expr_type(c->expr);

        if (expr_type == get_builtin_type(TYPE_NO_RETURN)) {
            post_error(p->eh, MSG_NO_RETURN_ASSIGNMENT, get_expr_start(c->expr), get_expr_end(c->expr));
            var_type = NULL;
        } else {
            var_type = expr_type;
            if (c->expr->kind != EXPR_LITERAL) {
                post_error(p->eh, MSG_CONST_INITIALIZER, get_expr_start(c->expr), get_expr_end(c->expr));
            }
        }
    } else {
        var_type = NULL;
    }

    return var_type;
}

static void parse_const(Parser *p, ASTConst *c) {

    consume(p, TK_CONST);

    c->pattern = (ASTPattern *) calloc(1, sizeof(ASTPattern));
    parse_pattern(p, c->pattern);

    if (is_mutable_pattern(c->pattern)) {
        post_error(p->eh, MSG_CONST_MUT, get_pattern_mut_start(c->pattern), get_pattern_mut_end(c->pattern));
    }

    Type *var_type;
    if (consume_if(p, TK_COLON)) { // type declared
        var_type = parse_const_with_type(p, c);
    } else {
        var_type = parse_const_without_type(p, c);
    }

    consume(p, TK_SEMI);

    create_const(p, c, var_type);
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

void parse_item(Parser *p, ASTItem *it) {

    bool found_error = false;
    while (true) {
        switch (peek_kind(p->lex)) {
            case TK_CONST: {
                it->kind = IT_CONST;
                parse_const(p, &it->it_const);
                return;
            }
            case TK_STATIC: {
                it->kind = IT_STATIC;
                parse_static(p, &it->it_static);
                return;
            }
            case TK_FN: {
                it->kind = IT_FUNCTION;
                parse_function(p, &it->it_function);
                return;
            }
            default: {
                if (!found_error) {
                    found_error = true;
                    post_error(p->eh, MSG_INVALID_ITEM, peek_start_coordinates(p->lex), NULL);
                }
                next_token(p->lex, NULL);
            }
        }
    }
}