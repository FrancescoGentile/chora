//
// Created by francesco on 04/12/21.
//

#ifndef CHORA_CODEGEN_H
#define CHORA_CODEGEN_H

#include "../ast/ast.h"

#include <llvm-c/Core.h>
#include <stdio.h>

typedef struct CodeGenerator {
    LLVMContextRef ctx;
    LLVMModuleRef mod;
    LLVMBuilderRef builder;

    LLVMValueRef ctor_fun;
    bool global_static;
} CodeGenerator;

// -------------------------------------------------

void init_generator(CodeGenerator *cg, const char *filename);

void free_generator(CodeGenerator *cg);

// -------------------------------------------------

bool generate_ir(CodeGenerator *cg, const ASTFile *f);

void print_ir(CodeGenerator *cg, FILE *out);

#endif //CHORA_CODEGEN_H
