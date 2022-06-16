//
// Created by francesco on 04/12/21.
//

#include "ast.h"

#include <stdio.h>

/// -------------------------------------------------
/// PRIVATE API
/// -------------------------------------------------

static Block *get_block_return_block(const ASTBlock *b) {

    if (b == NULL) { return NULL; }

    for (usize i = 0; i < b->stmts.length; ++i) {
        Block *block = get_stmt_return_block(b->stmts.entries[i]);
        if (block != NULL && block->level < b->block->level) {
            return block;
        }
    }

    Block *res = NULL;
    if (b->expr != NULL) {
        Block *block = get_expr_return_block(b->expr);
        if (block != NULL && block->level < b->block->level) {
            res = block;
        }
    }

    return res;
}

static Block *get_if_return_block(const ASTIf *i) {

    if (i == NULL) { return NULL; }

    // an if-then-else construct returns from a block, only if all the associated blocks
    // return from that block (or an outermost block) and if it has a final else statement
    // in all the other cases, we return NULL

    Block *block = get_block_return_block(i->if_body);
    bool complete = false;
    while (true) {
        ASTElse *el = i->else_body;

        if (el == NULL) {
            break;
        } else if (el->kind == ELSE_BLOCK) {
            Block *tmp = get_block_return_block(&el->else_block);
            if (tmp != NULL) {
                complete = true;
                if (tmp->level > block->level) {
                    block = tmp;
                }
            }
            break;
        } else {
            i = &el->else_if;
            Block *tmp = get_block_return_block(i->if_body);
            if (tmp == NULL) {
                break;
            } else if (tmp->level > block->level) {
                block = tmp;
            }
        }
    }

    if (complete) {
        return block;
    } else {
        return NULL;
    }
}

static void free_ast_if(ASTIf *i);

static void free_ast_else(ASTElse *el) {

    if (el == NULL) {
        return;
    } else if (el->kind == ELSE_BLOCK) {
        free_ast_block(&el->else_block);
    } else {
        free_ast_if(&el->else_if);
    }

}

static void free_ast_if(ASTIf *i) {

    free_ast_expr(i->condition);
    free(i->condition);
    free_ast_block(i->if_body);
    free(i->if_body);
    free_ast_else(i->else_body);
    free(i->else_body);
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

void free_ast_file(ASTFile *f) {

    if (f == NULL) { return; }
    for (usize i = 0; i < f->items.length; ++i) {
        free_ast_item(f->items.entries[i]);
        free(f->items.entries[i]);
    }
    f->items.free(&f->items);
}

// -------------------------------------------------

void free_ast_item(ASTItem *it) {

    if (it == NULL) { return; }

    switch (it->kind) {
        case IT_CONST: {
            ASTConst *c = &it->it_const;
            free_ast_pattern(c->pattern);
            free_ast_type(c->type);
            free_ast_expr(c->expr);
            free(c->pattern);
            free(c->type);
            free(c->expr);
            free(c->var);
            break;
        }
        case IT_STATIC: {
            ASTStatic *s = &it->it_static;
            free_ast_pattern(s->pattern);
            free_ast_type(s->type);
            free_ast_expr(s->expr);
            free(s->pattern);
            free(s->type);
            free(s->expr);
            free(s->var);
            break;
        }
        case IT_FUNCTION: {
            ASTFunction *f = &it->it_function;
            free_ast_block(f->body);
            free(f->body);
            free_ast_type(f->ret_type);
            free(f->ret_type);

            for (usize i = 0; i < f->params.length; ++i) {
                ASTFnParam *param = f->params.entries[i];
                free_ast_pattern(param->pattern);
                free_ast_type(param->type);
                free_ast_expr(param->expr);
                free(param->pattern);
                free(param->type);
                free(param->expr);
                free(param);
            }
            // FnParams are freed inside free_function
            f->params.free(&f->params);
            free_function(f->fun);
            free(f->fun);
        }
    }
}

// -------------------------------------------------

Type *get_block_type(const ASTBlock *b, bool without_int) {

    if (b == NULL) {
        return NULL;
    } else if (!without_int && b->block->not_return) {
        return get_builtin_type(TYPE_NO_RETURN);
    } else {
        return b->block->must_return;
    }
}

void free_ast_block(ASTBlock *b) {

    if (b == NULL) { return; }

    free(b->label);
    free(b->block);

    for (usize i = 0; i < b->stmts.length; ++i) {
        free_ast_stmt(b->stmts.entries[i]);
        free(b->stmts.entries[i]);
    }
    b->stmts.free(&b->stmts);

    if (b->expr != NULL) { // this check is not necessary
        free_ast_expr(b->expr);
        free(b->expr);
    }
}

void llvm_set_return_block(ASTBlock *b, LLVMBasicBlockRef llvm) {

    if (b == NULL) { return; }
    b->block->llvm_return = llvm;
}

LLVMBasicBlockRef llvm_get_return_block(const ASTBlock *b) {

    if (b == NULL) {
        return NULL;
    } else {
        return b->block->llvm_return;
    }
}

void llvm_set_next_block(ASTBlock *b, LLVMBasicBlockRef llvm) {

    if (b == NULL) { return; }
    b->block->llvm_next = llvm;
}

LLVMBasicBlockRef llvm_get_next_block(const ASTBlock *b) {

    if (b == NULL) {
        return NULL;
    } else {
        return b->block->llvm_next;
    }
}

// -------------------------------------------------

Type *get_if_return_type(const ASTIf *i) {

    if (i == NULL) { return NULL; }

    bool all_no_return = true;
    bool complete = false;
    Type *no_return_type = get_builtin_type(TYPE_NO_RETURN);
    Type *type = get_block_type(i->if_body, false);
    if (type != no_return_type) {
        all_no_return = false;
    }

    while (type != NULL) {
        ASTElse *el = i->else_body;

        if (el == NULL) {
            break;
        } else if (el->kind == ELSE_BLOCK) {
            complete = true;
            type = get_block_type(&el->else_block, false);
            if (type != no_return_type) {
                all_no_return = false;
            }
            break;
        } else {
            i = &el->else_if;
            type = get_block_type(i->if_body, false);
            if (type != no_return_type) {
                all_no_return = false;
            }
        }
    }

    if (all_no_return && complete) {
        return no_return_type;
    } else if (type != NULL && complete) {
        return type;
    } else {
        return get_builtin_type(TYPE_EMPTY);
    }
}

// -------------------------------------------------

void free_ast_expr(ASTExpr *e) {

    if (e == NULL || !e->is_valid) { return; }

    switch (e->kind) {
        case EXPR_GROUPED: {
            free_ast_expr(e->e_grouped.expr);
            free(e->e_grouped.expr);
            break;
        }
        case EXPR_LOOP: {
            free_ast_block(e->e_loop.body);
            free(e->e_loop.body);
            break;
        }
        case EXPR_BLOCK: {
            free_ast_block(&e->e_block);
            break;
        }
        case EXPR_WHILE: {
            free_ast_expr(e->e_while.condition);
            free_ast_block(e->e_while.body);
            free(e->e_while.condition);
            free(e->e_while.body);
            break;
        }
        case EXPR_FN_CALL: {
            ASTFnCall *fc = &e->e_fn_call;
            for (usize i = 0; i < fc->arguments.length; ++i) {
                ASTFnArgument *arg = fc->arguments.entries[i];
                if (arg->own_expr) {
                    free_ast_expr(arg->expr);
                    free(arg->expr);
                }
                free(arg->name);
                free(arg);
            }
            fc->arguments.free(&fc->arguments);
            break;
        }
        case EXPR_BREAK: {
            free_ast_expr(e->e_break.expr);
            free(e->e_break.expr);
            free(e->e_break.label);
            break;
        }
        case EXPR_RETURN: {
            free_ast_expr(e->e_return.expr);
            free(e->e_return.expr);
            free(e->e_return.label);
            break;
        }
        case EXPR_UNARY: {
            free_ast_expr(e->e_unary.expr);
            free(e->e_unary.expr);
            break;
        }
        case EXPR_BINARY: {
            free_ast_expr(e->e_binary.left);
            free_ast_expr(e->e_binary.right);
            free(e->e_binary.left);
            free(e->e_binary.right);
            break;
        }
        case EXPR_AS: {
            free_ast_expr(e->e_as.expr);
            free_ast_type(e->e_as.type);
            free(e->e_as.type);
            free(e->e_as.expr);
            break;
        }
        case EXPR_ASSIGN: {
            free_ast_expr(e->e_assign.left);
            free_ast_expr(e->e_assign.right);
            free(e->e_assign.left);
            free(e->e_assign.right);
            break;
        }
        case EXPR_IF: {
            free_ast_if(&e->e_if);
            break;
        }
        default: {
            // nothing to do
            break;
        }
    }
}

Type *get_expr_type(const ASTExpr *e) {

    if (e == NULL || !e->is_valid) {
        return NULL;
    }

    switch (e->kind) {
        case EXPR_LITERAL: {
            return e->e_literal.type;
        }
        case EXPR_RETURN:
        case EXPR_BREAK: {
            return get_builtin_type(TYPE_NO_RETURN);
        }
        case EXPR_WHILE: {
            return get_block_type(e->e_while.body, false);
        }
        case EXPR_BLOCK: {
            return get_block_type(&e->e_block, false);
        }
        case EXPR_LOOP: {
            return get_block_type(e->e_loop.body, false);
        }
        case EXPR_VARIABLE: {
            return get_var_type(e->e_var.var);
        }
        case EXPR_FN_CALL: {
            return e->e_fn_call.fun->ret_type;
        }
        case EXPR_BINARY: {
            return e->e_binary.op->ret_type;
        }
        case EXPR_UNARY: {
            return e->e_unary.op->ret_type;
        }
        case EXPR_GROUPED: {
            if (e->e_grouped.expr == NULL) {
                return get_builtin_type(TYPE_EMPTY);
            } else {
                return get_expr_type(e->e_grouped.expr);
            }
        }
        case EXPR_AS: {
            return e->e_as.type->type;
        }
        case EXPR_ASSIGN: {
            return get_builtin_type(TYPE_EMPTY);
        }
        case EXPR_IF: {
            return get_if_return_type(&e->e_if);
        }
        default: {
            fprintf(stderr, "Cannot get expression type for this expression\n");
            exit(1);
        }
    }
}

bool is_expr_with_block(const ASTExpr *e) {

    if (e == NULL) {
        return false;
    }

    switch (e->kind) {
        case EXPR_BLOCK:
        case EXPR_WHILE:
        case EXPR_LOOP:
        case EXPR_IF: {
            return true;
        }
        case EXPR_GROUPED: {
            return is_expr_with_block(e->e_block.expr);
        }
        default: {
            return false;
        }
    }
}

bool is_expr_var(const ASTExpr *e) {

    if (e == NULL) {
        return false;
    }

    switch (e->kind) {
        case EXPR_VARIABLE: {
            return true;
        }
        case EXPR_GROUPED: {
            return is_expr_var(e->e_block.expr);
        }
        default: {
            return false;
        }
    }
}

Variable *get_expr_var(const ASTExpr *e) {

    if (e == NULL) {
        return NULL;
    }

    switch (e->kind) {
        case EXPR_VARIABLE: {
            return e->e_var.var;
        }
        case EXPR_GROUPED: {
            return get_expr_var(e->e_block.expr);
        }
        default: {
            return NULL;
        }
    }
}

Block *get_expr_return_block(const ASTExpr *e) {

    if (e == NULL || !e->is_valid) {
        return NULL;
    }

    switch (e->kind) {
        case EXPR_GROUPED: {
            return get_expr_return_block(e->e_grouped.expr);
        }
        case EXPR_FN_CALL:
        case EXPR_VARIABLE:
        case EXPR_LITERAL: {
            return NULL;
        }
        case EXPR_AS: {
            return get_expr_return_block(e->e_as.expr);
        }
        case EXPR_BINARY: {
            Block *left = get_expr_return_block(e->e_binary.left);
            if (left == NULL) {
                return get_expr_return_block(e->e_binary.right);
            } else {
                return left;
            }
        }
        case EXPR_UNARY: {
            return get_expr_return_block(e->e_unary.expr);
        }
        case EXPR_RETURN: {
            return e->e_return.block;
        }
        case EXPR_BREAK: {
            return e->e_break.block;
        }
        case EXPR_ASSIGN: {
            return get_expr_return_block(e->e_assign.right);
        }
        case EXPR_WHILE: {
            return get_block_return_block(e->e_while.body);
        }
        case EXPR_LOOP: {
            return get_block_return_block(e->e_loop.body);
        }
        case EXPR_IF: {
            return get_if_return_block(&e->e_if);
        }
        case EXPR_BLOCK: {
            return get_block_return_block(&e->e_block);
        }
        default: {
            fprintf(stderr, "CODE-ERROR: strange expression kind in get_expr_return_block\n");
            exit(1);
        }
    }
}

const Coordinates *get_expr_start(const ASTExpr *e) {

    if (e == NULL) {
        return NULL;
    }

    switch (e->kind) {
        case EXPR_LITERAL: {
            return get_token_start(&e->e_literal.original);
        }
        case EXPR_LOOP: {
            return get_token_start(&e->e_loop.loop_lex);
        }
        case EXPR_BLOCK: {
            return get_token_start(&e->e_block.open);
        }
        case EXPR_WHILE: {
            return get_token_start(&e->e_while.while_lex);
        }
        case EXPR_UNARY: {
            if (e->e_unary.is_prefix) {
                return &e->e_unary.op_id.span.start;
            } else {
                return get_expr_start(e->e_unary.expr);
            }
        }
        case EXPR_BINARY: {
            return get_expr_start(e->e_binary.left);
        }
        case EXPR_VARIABLE: {
            return get_token_start(&e->e_var.id);
        }
        case EXPR_FN_CALL: {
            return get_token_start(&e->e_fn_call.id);
        }
        case EXPR_BREAK: {
            return get_token_start(&e->e_break.break_lex);
        }
        case EXPR_RETURN: {
            return get_token_start(&e->e_return.ret_lex);
        }
        case EXPR_GROUPED: {
            return get_token_start(&e->e_grouped.open);
        }
        case EXPR_AS: {
            return get_expr_start(e->e_as.expr);
        }
        case EXPR_IF: {
            return get_token_start(&e->e_if.if_lex);
        }
        default: {
            return NULL;
        }
    }
}

const Coordinates *get_expr_end(const ASTExpr *e) {

    if (e == NULL) {
        return NULL;
    }

    switch (e->kind) {
        case EXPR_LITERAL: {
            return &e->e_literal.original.span.end;
        }
        case EXPR_LOOP: {
            return &e->e_loop.body->close.span.end;
        }
        case EXPR_BLOCK: {
            return &e->e_block.close.span.end;
        }
        case EXPR_WHILE: {
            return &e->e_while.body->close.span.end;
        }
        case EXPR_UNARY: {
            if (e->e_unary.is_prefix) {
                return get_expr_start(e->e_unary.expr);
            } else {
                return &e->e_unary.op_id.span.end;
            }
        }
        case EXPR_BINARY: {
            return get_expr_end(e->e_binary.right);
        }
        case EXPR_VARIABLE: {
            return &e->e_var.id.span.end;
        }
        case EXPR_FN_CALL: {
            return &e->e_fn_call.id.span.start;
        }
        case EXPR_BREAK: {
            if (e->e_break.expr != NULL) {
                return get_expr_end(e->e_break.expr);
            } else if (e->e_break.label != NULL) {
                return &e->e_break.label->name.span.end;
            } else {
                return &e->e_break.break_lex.span.end;
            }
        }
        case EXPR_RETURN: {
            if (e->e_return.expr != NULL) {
                return get_expr_end(e->e_return.expr);
            } else if (e->e_return.label != NULL) {
                return &e->e_return.label->name.span.end;
            } else {
                return &e->e_return.ret_lex.span.end;
            }
        }
        case EXPR_GROUPED: {
            return &e->e_grouped.close.span.end;
        }
        case EXPR_AS: {
            return get_type_end(e->e_as.type);
        }
        default: {
            return NULL;
        }
    }
}

// -------------------------------------------------

void free_ast_stmt(ASTStmt *st) {

    if (st == NULL) { return; }
    switch (st->kind) {
        case STMT_EXPR: {
            free_ast_expr(&st->expr);
            break;
        }
        case STMT_ITEM: {
            free_ast_item(&st->item);
            break;
        }
        case STMT_LOCAL: {
            ASTLocal *l = &st->local;
            free_ast_pattern(l->pattern);
            free(l->pattern);
            free_ast_type(l->type);
            free(l->type);
            free_ast_expr(l->expr);
            free(l->expr);
            free(l->var);
        }
    }
}

Block *get_stmt_return_block(const ASTStmt *s) {

    if (s == NULL) {
        return NULL;
    }

    switch (s->kind) {
        case STMT_EXPR: {
            return get_expr_return_block(&s->expr);
        }
        case STMT_LOCAL: {
            return get_expr_return_block(s->local.expr);
        }
        case STMT_ITEM: {
            return NULL;
        }
        default: {
            fprintf(stderr, "CODE-ERROR: strange statement kind in get_stmt_return_block\n");
            exit(1);
        }
    }
}

// -------------------------------------------------

const StringSlice *get_label_string(const ASTLabel *l) {

    if (l == NULL) {
        return NULL;
    }

    switch (l->name.kind) {
        case TK_NAME_ID: {
            return &l->name.id;
        }
        case TK_WHILE:
        case TK_LOOP:
        case TK_IF:
        case TK_ELSE: {
            return &l->name.keyword;
        }
        default: { // this should never happen
            return NULL;
        }
    }
}

// -------------------------------------------------

void free_ast_pattern(ASTPattern *p) {

    if (p == NULL) { return; }

    if (p->kind == PT_IDENTIFIER) {
        free(p->id.mutable);
    }
}

bool is_named_pattern(const ASTPattern *p) {

    if (p == NULL || p->kind == PT_WILDCARD) {
        return false;
    } else {
        return true;
    }
}

bool is_mutable_pattern(const ASTPattern *p) {

    if (p == NULL || p->kind == PT_WILDCARD || p->id.mutable == NULL) {
        return false;
    }

    return true;
}

const StringSlice *get_pattern_id(const ASTPattern *p) {

    if (p == NULL) { return NULL; }

    if (p->kind == PT_IDENTIFIER) {
        return get_token_slice(&p->id.identifier);
    } else {
        return NULL;
    }
}

const Coordinates *get_pattern_start(const ASTPattern *p) {

    if (p == NULL) { return NULL; }

    if (p->kind == PT_IDENTIFIER) {
        if (p->id.mutable != NULL) {
            return get_token_start(p->id.mutable);
        } else {
            return get_token_start(&p->id.identifier);
        }
    } else {
        return get_token_start(&p->underscore);
    }
}

const Coordinates *get_pattern_end(const ASTPattern *p) {

    if (p == NULL) { return NULL; }

    if (p->kind == PT_IDENTIFIER) {
        return get_token_end(&p->id.identifier);
    } else {
        return get_token_end(&p->underscore);
    }
}

const Coordinates *get_pattern_mut_start(const ASTPattern *p) {

    if (p == NULL) { return NULL; }

    if (p->kind == PT_IDENTIFIER) {
        if (p->id.mutable != NULL) {
            return get_token_start(p->id.mutable);
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

const Coordinates *get_pattern_mut_end(const ASTPattern *p) {

    if (p == NULL) { return NULL; }

    if (p->kind == PT_IDENTIFIER) {
        if (p->id.mutable != NULL) {
            return get_token_end(p->id.mutable);
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

const Coordinates *get_pattern_id_start(const ASTPattern *p) {

    if (p == NULL) { return NULL; }

    if (p->kind == PT_IDENTIFIER) {
        return get_token_start(&p->id.identifier);
    } else {
        return NULL;
    }
}

const Coordinates *get_pattern_id_end(const ASTPattern *p) {

    if (p == NULL) { return NULL; }

    if (p->kind == PT_IDENTIFIER) {
        return get_token_end(&p->id.identifier);
    } else {
        return NULL;
    }
}

// -------------------------------------------------

void free_ast_type(ASTType *t) {

    if (t == NULL) { return; }
    if (t->kind == TY_PAREN) {
        free_ast_type(t->t_paren.type);
        free(t->t_paren.type);
    }
}

bool is_inferred_type(const ASTType *t) {

    if (t == NULL) {
        return false;
    }

    switch (t->kind) {
        case TY_INFERRED: {
            return true;
        }
        case TY_EMPTY:
        case TY_IDENTIFIER: {
            return false;
        }
        case TY_PAREN: {
            return is_inferred_type(t->t_paren.type);
        }
    }
}

Type *get_type(const ASTType *t) {

    if (t == NULL) {
        return NULL;
    } else {
        return t->type;
    }
}

const Coordinates *get_type_start(const ASTType *t) {

    if (t == NULL) {
        return NULL;
    }

    switch (t->kind) {
        case TY_IDENTIFIER:
        case TY_INFERRED: {
            return &t->symbol.span.start;
        }
        case TY_PAREN: {
            return &t->t_paren.open.span.start;
        }
        case TY_EMPTY: {
            return &t->t_empty.open.span.start;
        }
    }
}

const Coordinates *get_type_end(const ASTType *t) {

    if (t == NULL) {
        return NULL;
    }

    switch (t->kind) {
        case TY_IDENTIFIER:
        case TY_INFERRED: {
            return &t->symbol.span.end;
        }
        case TY_PAREN: {
            return &t->t_paren.close.span.end;
        }
        case TY_EMPTY: {
            return &t->t_empty.close.span.end;
        }
    }
}