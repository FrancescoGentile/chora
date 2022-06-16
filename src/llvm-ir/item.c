//
// Created by francesco on 11/12/21.
//

#include "item.h"
#include "stmt_expr.h"

#include <stdio.h>
#include <string.h>

/// -------------------------------------------------
/// PRIVATE FUNCTIONS
/// -------------------------------------------------

static void initialize_parameters(CodeGenerator *cg, ASTFunction *f) {

    Type *empty_type = get_builtin_type(TYPE_EMPTY);
    Function *fun = f->fun;

    for (usize i = 0; i < fun->params.length; ++i) {
        FnParam *param = fun->params.entries[i];
        param->llvm_value = LLVMGetParam(fun->llvm_fun, i);
        FnParamVar *var = &param->var->v_param;

        if (param->type == empty_type) {
            var->llvm_value = NULL;
            continue;
        }

        if (var == NULL) { // parameter is '_'
            LLVMSetValueName2(param->llvm_value, "_", 1);
        } else {
            LLVMSetValueName2(param->llvm_value, param->name.data, param->name.len);

            if (param->is_mutable) { // if the param is mutable, we allocate it on the stack
                char *param_name = to_cstring(&param->name);
                LLVMValueRef mut_value = LLVMBuildAlloca(cg->builder, param->type->llvm_type, param_name);
                var->llvm_value = mut_value;
                LLVMBuildStore(cg->builder, param->llvm_value, mut_value);
                free(param_name);
            } else {
                var->llvm_value = param->llvm_value;
            }
        }
    }
}

static void create_function(CodeGenerator *cg, ASTFunction *f) {

    Function *fun = f->fun;
    Type *ret_type = fun->ret_type;

    Type *empty_type = get_builtin_type(TYPE_EMPTY);

    LLVMTypeRef *params = (LLVMTypeRef *) calloc(fun->params.length, sizeof(LLVMTypeRef));
    usize params_number = 0;
    // we don't create empty type parameters
    for (usize i = 0; i < fun->params.length; ++i) {
        FnParam *param = fun->params.entries[i];
        if (param->type != empty_type) {
            params[i] = param->type->llvm_type;
            ++params_number;
        }
    }

    LLVMTypeRef fun_type = LLVMFunctionType(ret_type->llvm_type, params, params_number, false);

    char *fun_name = to_cstring(&fun->name);
    LLVMValueRef llvm_fun = LLVMAddFunction(cg->mod, fun_name, fun_type);
    fun->llvm_fun = llvm_fun;

    free(fun_name);
    free(params);
}

static void generate_function(CodeGenerator *cg, ASTFunction *f) {

    create_function(cg, f);

    Function *fun = f->fun;
    Type *ret_type = fun->ret_type;

    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(cg->ctx, fun->llvm_fun, "fn_entry");
    LLVMBasicBlockRef exit = LLVMCreateBasicBlockInContext(cg->ctx, "fn_exit");
    LLVMPositionBuilderAtEnd(cg->builder, entry);

    initialize_parameters(cg, f);

    LLVMValueRef ret = NULL;
    if (ret_type != get_builtin_type(TYPE_EMPTY)) {
        LLVMPositionBuilderAtEnd(cg->builder, exit);
        ret = LLVMBuildPhi(cg->builder, ret_type->llvm_type, "");
        LLVMPositionBuilderAtEnd(cg->builder, entry);
    }

    llvm_set_next_block(f->body, exit);
    llvm_set_return_block(f->body, exit);
    generate_block(cg, f->body);

    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, exit);
    LLVMPositionBuilderAtEnd(cg->builder, exit);

    if (ret != NULL) {
        LLVMBuildRet(cg->builder, ret);
    } else {
        LLVMBuildRetVoid(cg->builder);
    }
}

static void generate_const(CodeGenerator *cg, ASTConst *c) {

    if (c->var == NULL) { // the variable is never used
        return;
    }

    ConstVar *var = &c->var->v_const;
    LLVMValueRef val = generate_expression(cg, c->expr);
    if (var->type != get_builtin_type(TYPE_EMPTY)) {
        var->llvm_value = val;
    } else {
        var->llvm_value = NULL;
    }
}

static void generate_global_static(CodeGenerator *cg, ASTStatic *s) {

    cg->global_static = true;
    LLVMBasicBlockRef block = LLVMGetLastBasicBlock(cg->ctor_fun);
    LLVMPositionBuilderAtEnd(cg->builder, block);
    LLVMValueRef expr_value = generate_expression(cg, s->expr);

    if (s->var != NULL) {
        Type *type = get_var_type(s->var);
        char *name = to_cstring(get_pattern_id(s->pattern));
        LLVMValueRef global = LLVMAddGlobal(cg->mod, type->llvm_type, name);
        s->var->v_static.llvm_value = global;
        free(name);

        LLVMSetLinkage(global, LLVMInternalLinkage);
        LLVMSetInitializer(global, LLVMConstNull(s->type->type->llvm_type));
        LLVMBuildStore(cg->builder, expr_value, global);
    }
}

static void generate_local_static(CodeGenerator *cg, ASTStatic *s) {

    char *name = to_cstring(get_pattern_id(s->pattern));
    char *guard_name = (char *) calloc(strlen(name) + 7, sizeof(char));
    strcpy(guard_name, name);
    strcat(guard_name, "_guard");

    // create global static variable guard
    // the guard is used to check if the variable was already initialised
    LLVMTypeRef guard_type = LLVMInt1TypeInContext(cg->ctx);
    LLVMValueRef guard = LLVMAddGlobal(cg->mod, guard_type, guard_name);
    LLVMSetLinkage(guard, LLVMInternalLinkage);
    LLVMSetInitializer(guard, LLVMConstNull(guard_type));

    // verify if the guard is set to false or true
    // if guard = 0, then go to init_block where the variable is initialised and guards is set to 1
    // otherwise, go to cont_flow where we continue the normal execution flow
    LLVMValueRef guard_load = LLVMBuildLoad2(cg->builder, guard_type, guard, "");
    LLVMValueRef cmp = LLVMBuildICmp(cg->builder, LLVMIntEQ, guard_load, LLVMConstNull(guard_type), "");
    LLVMBasicBlockRef init_block = LLVMCreateBasicBlockInContext(cg->ctx, "");
    LLVMBasicBlockRef cont_flow = LLVMCreateBasicBlockInContext(cg->ctx, "");
    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, init_block);
    LLVMBuildCondBr(cg->builder, cmp, init_block, cont_flow);

    // initialise the variable and set guard to 1
    LLVMPositionBuilderAtEnd(cg->builder, init_block);
    LLVMValueRef expr_value = generate_expression(cg, s->expr);
    if (s->var != NULL) { // create static global variable
        Type *type = get_var_type(s->var);

        LLVMValueRef global = LLVMAddGlobal(cg->mod, type->llvm_type, name);
        s->var->v_static.llvm_value = global;

        LLVMSetLinkage(global, LLVMInternalLinkage);
        LLVMSetInitializer(global, LLVMConstNull(type->llvm_type));
        LLVMBuildStore(cg->builder, expr_value, global);
    }

    LLVMBuildStore(cg->builder, LLVMConstInt(guard_type, 1, 0), guard);
    LLVMBuildBr(cg->builder, cont_flow);

    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, cont_flow);
    LLVMPositionBuilderAtEnd(cg->builder, cont_flow);

    free(name);
    free(guard_name);
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

void generate_item(CodeGenerator *cg, ASTItem *item, bool global) {

    switch (item->kind) {
        case IT_CONST: {
            generate_const(cg, &item->it_const);
            break;
        }
        case IT_STATIC: {
            if (global) { generate_global_static(cg, &item->it_static); }
            else { generate_local_static(cg, &item->it_static); }
            break;
        }
        case IT_FUNCTION: {
            generate_function(cg, &item->it_function);
            break;
        }
        default: {
            // this should never happen
            fprintf(stderr, "CODE-ERROR (code-generation): strange item type\n");
            exit(1);
        }
    }
}