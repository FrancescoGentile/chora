//
// Created by francesco on 08/12/21.
//

#include "stmt_expr.h"
#include "item.h"

#include <stdio.h>

/*
 * Notes: an empty value is NULL
 * At the moment we do not support empty values, since all the provided operations don't need it.
 * In future versions, when users will be able to define custom operators also with the empty type,
 * we will implement it.
 */

/// -------------------------------------------------
/// STATEMENTS
/// -------------------------------------------------

static void generate_local(CodeGenerator *cg, ASTLocal *l) {

    LLVMValueRef llvm_expr = generate_expression(cg, l->expr);
    // if the variable is never used, var is NULL
    if (l->var != NULL) {
        LocalVar *var = &l->var->v_local;
        if (var->type == get_builtin_type(TYPE_EMPTY)) {
            var->llvm_value = NULL;
        } else if (var->mut) {
            char *var_name = to_cstring(&var->name);
            var->llvm_value = LLVMBuildAlloca(cg->builder, var->type->llvm_type, var_name);
            LLVMBuildStore(cg->builder, llvm_expr, var->llvm_value);
            free(var_name);
        } else {
            var->llvm_value = llvm_expr;
        }
    }
}

static void generate_statement(CodeGenerator *cg, ASTStmt *s) {

    switch (s->kind) {
        case STMT_LOCAL: {
            generate_local(cg, &s->local);
            break;
        }
        case STMT_ITEM: {
            generate_item(cg, &s->item, false);
            break;
        }
        case STMT_EXPR: {
            generate_expression(cg, &s->expr);
            break;
        }
        default: {
            fprintf(stderr, "ERROR-CODE (code-generation): strange statement kind\n");
            exit(1);
        }
    }
}

/// -------------------------------------------------
/// EXPRESSIONS
/// -------------------------------------------------

static LLVMValueRef generate_while(CodeGenerator *cg, ASTWhile *w) {

    LLVMBasicBlockRef entry = LLVMCreateBasicBlockInContext(cg->ctx, "while_entry");
    LLVMBasicBlockRef body = LLVMCreateBasicBlockInContext(cg->ctx, "while_body");
    LLVMBasicBlockRef after = LLVMCreateBasicBlockInContext(cg->ctx, "while_after");

    LLVMBuildBr(cg->builder, entry);
    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, entry);
    LLVMPositionBuilderAtEnd(cg->builder, entry);

    LLVMValueRef cond = generate_expression(cg, w->condition);
    LLVMBuildICmp(cg->builder, LLVMIntEQ, cond, LLVMConstInt(LLVMInt1TypeInContext(cg->ctx), 1, false), "");
    LLVMBuildCondBr(cg->builder, cond, body, after);

    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, body);
    LLVMPositionBuilderAtEnd(cg->builder, body);
    llvm_set_next_block(w->body, entry);
    llvm_set_return_block(w->body, after);
    generate_block(cg, w->body);

    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, after);
    LLVMPositionBuilderAtEnd(cg->builder, after);

    return NULL;
}

static LLVMValueRef generate_loop(CodeGenerator *cg, ASTLoop *l) {

    LLVMBasicBlockRef body = LLVMCreateBasicBlockInContext(cg->ctx, "loop_body");

    LLVMBuildBr(cg->builder, body);
    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, body);
    LLVMPositionBuilderAtEnd(cg->builder, body);

    Type *expr_type = get_block_type(l->body, false);

    LLVMValueRef ret = NULL;
    if (expr_type == get_builtin_type(TYPE_NO_RETURN)) {
        generate_block(cg, l->body);
    } else {

        LLVMBasicBlockRef after = LLVMCreateBasicBlockInContext(cg->ctx, "loop_after");
        llvm_set_next_block(l->body, body);
        llvm_set_return_block(l->body, after);

        if (expr_type != get_builtin_type(TYPE_EMPTY)) {
            LLVMPositionBuilderAtEnd(cg->builder, after);
            ret = LLVMBuildPhi(cg->builder, expr_type->llvm_type, "");
            LLVMPositionBuilderAtEnd(cg->builder, body);
        }

        generate_block(cg, l->body);
        LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, after);
        LLVMPositionBuilderAtEnd(cg->builder, after);
    }

    return ret;
}

static LLVMValueRef generate_if(CodeGenerator *cg, ASTIf *i) {

    Type *ret_type = get_if_return_type(i);
    LLVMValueRef ret = NULL;

    LLVMBasicBlockRef after;

    if (ret_type == get_builtin_type(TYPE_NO_RETURN)) {
        after = NULL;
    } else {
        after = LLVMCreateBasicBlockInContext(cg->ctx, "if_after");

        if (ret_type != get_builtin_type(TYPE_EMPTY)) {
            LLVMBasicBlockRef current = LLVMGetInsertBlock(cg->builder);
            LLVMPositionBuilderAtEnd(cg->builder, after);
            ret = LLVMBuildPhi(cg->builder, ret_type->llvm_type, "");
            LLVMPositionBuilderAtEnd(cg->builder, current);
        }
    }

    LLVMBasicBlockRef if_entry = LLVMGetInsertBlock(cg->builder);
    while (true) {
        LLVMValueRef cond = generate_expression(cg, i->condition);
        LLVMBasicBlockRef then_block = LLVMCreateBasicBlockInContext(cg->ctx, "then");

        if (i->else_body != NULL) {
            char *name = (i->else_body->kind == ELSE_BLOCK ? "else" : "else_if");
            LLVMBasicBlockRef else_block = LLVMCreateBasicBlockInContext(cg->ctx, name);
            LLVMBuildCondBr(cg->builder, cond, then_block, else_block);

            LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, then_block);
            LLVMPositionBuilderAtEnd(cg->builder, then_block);
            llvm_set_return_block(i->if_body, after);
            llvm_set_next_block(i->if_body, after);
            generate_block(cg, i->if_body);

            if (i->else_body->kind == ELSE_BLOCK) {
                ASTElse *el = i->else_body;
                LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, else_block);
                LLVMPositionBuilderAtEnd(cg->builder, else_block);
                llvm_set_return_block(&el->else_block, after);
                llvm_set_next_block(&el->else_block, after);
                generate_block(cg, &el->else_block);
                break;

            } else {
                if_entry = else_block;
                LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, if_entry);
                LLVMPositionBuilderAtEnd(cg->builder, if_entry);
                i = &i->else_body->else_if;
            }

        } else {
            LLVMBuildCondBr(cg->builder, cond, then_block, after);
            LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, then_block);
            LLVMPositionBuilderAtEnd(cg->builder, then_block);
            llvm_set_return_block(i->if_body, after);
            llvm_set_next_block(i->if_body, after);
            generate_block(cg, i->if_body);
            break;
        }
    }

    if (after != NULL) {
        LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, after);
        LLVMPositionBuilderAtEnd(cg->builder, after);
    }
    return ret;
}

static LLVMValueRef generate_fn_call(CodeGenerator *cg, ASTFnCall *f) {

    usize args_count = 0;
    LLVMValueRef *llvm_args = (LLVMValueRef *) calloc(f->arguments.length, sizeof(LLVMValueRef));

    for (usize i = 0; i < f->arguments.length; ++i) {
        LLVMValueRef val = generate_expression(cg, f->arguments.entries[i]->expr);
        if (val != NULL) {
            llvm_args[i] = val;
            ++args_count;
        }
    }

    // LLVMBuildCall is deprecated, we should use LVVMBuildCall2.
    // The problem is that if we pass an empty string as a name (when the return type is void),
    // LLVMBuildCall2 gives a name to the return value (causing an error at compile time).
    // Thus, for the moment, we stick to LLVMBuildCall.
    LLVMValueRef ret = LLVMBuildCall(cg->builder, f->fun->llvm_fun, llvm_args, args_count, "");

    free(llvm_args);
    return ret;
}

static LLVMValueRef generate_literal(CodeGenerator *cg, ASTLiteral *l) {

    switch (l->type->kind) {
        case TYPE_EMPTY: {
            fprintf(stderr, "CODE-ERROR (code-generation): cannot generate an empty literal value");
            exit(1);
        }
        case TYPE_BOOL: {
            return LLVMConstInt(l->type->llvm_type, l->bool_lit, false);
        }
        case TYPE_U8:
        case TYPE_U16:
        case TYPE_U32:
        case TYPE_U64: {
            return LLVMConstInt(l->type->llvm_type, l->int_lit, false);
        }
        case TYPE_I8:
        case TYPE_I16:
        case TYPE_I32:
        case TYPE_I64: {
            return LLVMConstInt(l->type->llvm_type, l->int_lit, true);
        }
        case TYPE_F32:
        case TYPE_F64: {
            return LLVMConstReal(l->type->llvm_type, (double) l->float_lit);
        }
        default: {
            fprintf(stderr,
                    "CODE-ERROR (code-generation): unknown type during the creation of a literal value\n");
            exit(1);
        }
    }
}

static LLVMValueRef generate_variable(CodeGenerator *cg, ASTVariable *v) {

    if (v == NULL) { return NULL; }

    Variable *var = v->var;
    switch (var->kind) {
        case VAR_CONST: {
            return var->v_const.llvm_value;
        }

        case VAR_STATIC: {
            if (var->v_static.llvm_value == NULL) {
                return NULL;
            } else {
                return LLVMBuildLoad2(cg->builder, var->v_static.type->llvm_type,
                                      var->v_static.llvm_value, "");
            }
        }
        case VAR_LOCAL: {
            if (var->v_local.llvm_value == NULL) {
                return NULL;
            } else if (var->v_local.mut) {
                return LLVMBuildLoad2(cg->builder, var->v_local.type->llvm_type,
                                      var->v_local.llvm_value, "");
            } else {
                return var->v_local.llvm_value;
            }
        }
        case VAR_FN_PARAM: {
            if (var->v_param.llvm_value == NULL) {
                return NULL;
            } else if (var->v_param.mut) {
                return LLVMBuildLoad2(cg->builder, var->v_param.type->llvm_type,
                                      var->v_param.llvm_value, "");
            } else {
                return var->v_param.llvm_value;
            }
        }
    }
}

static LLVMValueRef generate_return(CodeGenerator *cg, ASTReturn *r) {

    LLVMBasicBlockRef dest_block = r->block->llvm_return;
    if (dest_block == NULL) {
        fprintf(stderr, "CODE-ERROR (code-generation): return block is NULL");
        exit(1);
    } else if (r->expr == NULL) {
        LLVMBuildBr(cg->builder, dest_block);
    } else if (get_expr_type(r->expr) == get_builtin_type(TYPE_EMPTY)) {
        generate_expression(cg, r->expr);
        LLVMBuildBr(cg->builder, dest_block);
    } else {
        LLVMValueRef val = generate_expression(cg, r->expr);
        LLVMBasicBlockRef current = LLVMGetInsertBlock(cg->builder);
        LLVMValueRef phi = LLVMGetFirstInstruction(dest_block);
        LLVMAddIncoming(phi, &val, &current, 1);
        LLVMBuildBr(cg->builder, dest_block);
    }

    return NULL;
}

static LLVMValueRef generate_break(CodeGenerator *cg, ASTBreak *b) {

    LLVMBasicBlockRef dest_block = b->block->llvm_return;
    if (dest_block == NULL) {
        fprintf(stderr, "CODE-ERROR (code-generation): break block is NULL");
        exit(1);
    } else if (b->expr == NULL) {
        LLVMBuildBr(cg->builder, dest_block);
    } else if (get_expr_type(b->expr) == get_builtin_type(TYPE_EMPTY)) {
        generate_expression(cg, b->expr);
        LLVMBuildBr(cg->builder, dest_block);
    } else {
        LLVMValueRef val = generate_expression(cg, b->expr);
        LLVMBasicBlockRef current = LLVMGetInsertBlock(cg->builder);
        LLVMValueRef phi = LLVMGetFirstInstruction(dest_block);
        LLVMAddIncoming(phi, &val, &current, 1);
        LLVMBuildBr(cg->builder, dest_block);
    }

    return NULL;
}

static LLVMValueRef generate_expr_block(CodeGenerator *cg, ASTBlock *b) {

    LLVMValueRef ret = NULL;
    Type *type = get_block_type(b, false);
    if (type == get_builtin_type(TYPE_NO_RETURN)) {
        generate_block(cg, b);
    } else {
        LLVMBasicBlockRef after = LLVMCreateBasicBlockInContext(cg->ctx, "loop_after");
        llvm_set_next_block(b, after);
        llvm_set_return_block(b, after);

        if (type != get_builtin_type(TYPE_EMPTY)) {
            LLVMPositionBuilderAtEnd(cg->builder, after);
            ret = LLVMBuildPhi(cg->builder, type->llvm_type, "");
            LLVMPositionBuilderAtEnd(cg->builder, after);
        }

        generate_block(cg, b);
        LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, after);
        LLVMPositionBuilderAtEnd(cg->builder, after);
    }

    return ret;
}

static LLVMValueRef generate_assign(CodeGenerator *cg, ASTAssign *a) {

    LLVMValueRef val = generate_expression(cg, a->right);
    if (val != NULL) {
        Variable *v = get_expr_var(a->left);
        LLVMValueRef llvm_var = llvm_get_var_value(v);
        LLVMBuildStore(cg->builder, val, llvm_var);
    }

    return NULL;
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

LLVMValueRef generate_expression(CodeGenerator *cg, ASTExpr *e) {

    if (e == NULL) { return NULL; }

    switch (e->kind) {
        case EXPR_LITERAL: {
            return generate_literal(cg, &e->e_literal);
        }
        case EXPR_VARIABLE: {
            return generate_variable(cg, &e->e_var);
        }
        case EXPR_BINARY: {
            return e->e_binary.op->generate_op(cg, e);
        }
        case EXPR_UNARY: {
            return e->e_unary.op->generate_op(cg, e);
        }
        case EXPR_AS: {
            return e->e_as.op->generate_op(cg, e);
        }
        case EXPR_RETURN: {
            return generate_return(cg, &e->e_return);
        }
        case EXPR_BREAK: {
            return generate_break(cg, &e->e_break);
        }
        case EXPR_FN_CALL: {
            return generate_fn_call(cg, &e->e_fn_call);
        }
        case EXPR_BLOCK: {
            return generate_expr_block(cg, &e->e_block);
        }
        case EXPR_WHILE: {
            return generate_while(cg, &e->e_while);
        }
        case EXPR_LOOP: {
            return generate_loop(cg, &e->e_loop);
        }
        case EXPR_IF: {
            return generate_if(cg, &e->e_if);
        }
        case EXPR_GROUPED: {
            return generate_expression(cg, e->e_grouped.expr);
        }
        case EXPR_ASSIGN: {
            return generate_assign(cg, &e->e_assign);
        }
        default: {
            fprintf(stderr, "CODE-ERROR (code-generation): unknown expression kind\n");
            exit(1);
        }
    }
}

LLVMValueRef generate_block(CodeGenerator *cg, ASTBlock *b) {

    for (usize i = 0; i < b->stmts.length; ++i) {
        generate_statement(cg, b->stmts.entries[i]);
    }

    LLVMValueRef ret;
    if (b->expr == NULL || get_expr_type(b->expr) == get_builtin_type(TYPE_EMPTY)) {
        ret = NULL;
    } else {
        LLVMValueRef val = generate_expression(cg, b->expr);
        LLVMBasicBlockRef current = LLVMGetInsertBlock(cg->builder);
        ret = LLVMGetFirstInstruction(llvm_get_return_block(b));
        LLVMAddIncoming(ret, &val, &current, 1);
    }

    LLVMBasicBlockRef next = llvm_get_next_block(b);
    if (next != NULL && !b->block->last_is_jump) {
        LLVMBuildBr(cg->builder, next);
    }

    return ret;
}