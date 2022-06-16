//
// Created by francesco on 04/12/21.
//

#include "codegen.h"
#include "item.h"

#include <stdio.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/TargetMachine.h>

/// -------------------------------------------------
/// PRIVATE API
/// -------------------------------------------------

static void generate_types(CodeGenerator *cg) {

    for (usize i = TYPE_NO_RETURN; i < TYPES_COUNT; ++i) {
        Type *type = get_builtin_type(i);
        switch (type->kind) {
            case TYPE_NO_RETURN: {
                type->llvm_type = NULL;
                break;
            }
            case TYPE_EMPTY: {
                type->llvm_type = LLVMVoidTypeInContext(cg->ctx);
                break;
            }
            case TYPE_BOOL: {
                type->llvm_type = LLVMInt1TypeInContext(cg->ctx);
                break;
            }
            case TYPE_U8:
            case TYPE_I8: {
                type->llvm_type = LLVMInt8TypeInContext(cg->ctx);
                break;
            }
            case TYPE_U16:
            case TYPE_I16: {
                type->llvm_type = LLVMInt16TypeInContext(cg->ctx);
                break;
            }
            case TYPE_U32:
            case TYPE_I32: {
                type->llvm_type = LLVMInt32TypeInContext(cg->ctx);
                break;
            }
            case TYPE_U64:
            case TYPE_I64: {
                type->llvm_type = LLVMInt64TypeInContext(cg->ctx);
                break;
            }
            case TYPE_F32: {
                type->llvm_type = LLVMFloatTypeInContext(cg->ctx);
                break;
            }
            case TYPE_F64: {
                type->llvm_type = LLVMDoubleTypeInContext(cg->ctx);
                break;
            }
            default: {
                fprintf(stderr, "CODE-ERROR (code-generation): trying to create an unknown type\n");
                exit(1);
            }
        }
    }
}

static void create_ctor(CodeGenerator *cg) {

    LLVMTypeRef *ctor_struct_types = (LLVMTypeRef *) calloc(3, sizeof(LLVMTypeRef));
    ctor_struct_types[0] = LLVMInt32TypeInContext(cg->ctx);
    ctor_struct_types[1] = LLVMTypeOf(cg->ctor_fun);
    ctor_struct_types[2] = LLVMPointerType(LLVMInt8TypeInContext(cg->ctx), 0);

    LLVMTypeRef ctor_struct = LLVMStructCreateNamed(cg->ctx, "ctor_struct");
    LLVMStructSetBody(ctor_struct, ctor_struct_types, 3, 0);

    LLVMTypeRef global_ctor_type = LLVMArrayType(ctor_struct, 1);
    LLVMValueRef global_ctor = LLVMAddGlobal(cg->mod, global_ctor_type, "llvm.global_ctors");
    LLVMSetLinkage(global_ctor, LLVMAppendingLinkage);

    LLVMValueRef *values = (LLVMValueRef *) calloc(3, sizeof(LLVMValueRef));
    values[0] = LLVMConstInt(ctor_struct_types[0], 65536, false);
    values[1] = cg->ctor_fun;
    values[2] = LLVMConstNull(ctor_struct_types[2]);

    LLVMValueRef *value = (LLVMValueRef *) calloc(1, sizeof(LLVMValueRef));
    *value = LLVMConstNamedStruct(ctor_struct, values, 3);

    LLVMValueRef init = LLVMConstArray(ctor_struct, value, 1);
    LLVMSetInitializer(global_ctor, init);

    free(ctor_struct_types);
    free(values);
    free(value);
}

static bool check(CodeGenerator *cg) {

    char *error;
    bool invalid = LLVMVerifyModule(cg->mod, LLVMReturnStatusAction, &error);
    //fprintf(stdout, "%s\n", LLVMPrintModuleToString(cg->mod));
    LLVMDisposeMessage(error);

    return !invalid;
}

static void perform_optimizations(CodeGenerator *cg) {

    LLVMPassManagerRef manager = LLVMCreatePassManager();
    LLVMRunPassManager(manager, cg->mod);
    LLVMDisposePassManager(manager);
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

void init_generator(CodeGenerator *cg, const char *filename) {

    cg->ctx = LLVMContextCreate();
    cg->mod = LLVMModuleCreateWithNameInContext(filename, cg->ctx);
    cg->builder = LLVMCreateBuilderInContext(cg->ctx);
    LLVMSetTarget(cg->mod, LLVMGetDefaultTargetTriple());
}

void free_generator(CodeGenerator *cg) {

    if (cg == NULL) { return; }
    LLVMDisposeBuilder(cg->builder);
    LLVMDisposeModule(cg->mod);
    LLVMContextDispose(cg->ctx);
}

// -------------------------------------------------

bool generate_ir(CodeGenerator *cg, const ASTFile *f) {

    generate_types(cg);

    // create a function to initialize global static variables
    LLVMTypeRef ctor_type = LLVMFunctionType(LLVMVoidTypeInContext(cg->ctx), NULL, 0, false);
    cg->ctor_fun = LLVMAddFunction(cg->mod, "ctor_fun", ctor_type);
    LLVMAppendBasicBlockInContext(cg->ctx, cg->ctor_fun, "entry");
    cg->global_static = false;

    for (usize i = 0; i < f->items.length; ++i) {
        generate_item(cg, f->items.entries[i], true);
    }

    if (!cg->global_static) {
        LLVMDeleteFunction(cg->ctor_fun);
    } else {
        LLVMBasicBlockRef b = LLVMGetLastBasicBlock(cg->ctor_fun);
        LLVMPositionBuilderAtEnd(cg->builder, b);
        LLVMBuildRetVoid(cg->builder);

        create_ctor(cg);
    }

    bool valid = check(cg);
    if (valid) {
        perform_optimizations(cg);
    }

    return valid;
}

void print_ir(CodeGenerator *cg, FILE *out) {

    char *code = LLVMPrintModuleToString(cg->mod);
    fputs(code, out);
    LLVMDisposeMessage(code);
}

