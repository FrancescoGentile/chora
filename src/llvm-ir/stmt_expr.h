//
// Created by francesco on 08/12/21.
//

#ifndef CHORA_LLVM_IR_STMT_EXPR_H
#define CHORA_LLVM_IR_STMT_EXPR_H

#include "codegen.h"

LLVMValueRef generate_expression(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef generate_block(CodeGenerator *cg, ASTBlock *b);

#endif //CHORA_LLVM_IR_STMT_EXPR_H
