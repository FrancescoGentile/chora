//
// Created by francesco on 11/12/21.
//

#ifndef CHORA_LLVM_IR_ITEM_H
#define CHORA_LLVM_IR_ITEM_H

#include "codegen.h"

void generate_item(CodeGenerator *cg, ASTItem *item, bool global);

#endif //CHORA_LLVM_IR_ITEM_H
