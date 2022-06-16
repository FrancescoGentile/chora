//
// Created by francesco on 08/12/21.
//

#ifndef CHORA_OPERATOR_H
#define CHORA_OPERATOR_H

#include "codegen.h"

/// -------------------------------------------------
/// OPERATORS on INTEGERS
/// -------------------------------------------------

/// binary
LLVMValueRef sum_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef sub_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef mul_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef div_s_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef div_u_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef rem_s_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef rem_u_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef and_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef or_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef xor_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef shl_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef shr_s_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef shr_u_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef equal_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef not_equal_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef less_s_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef less_u_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef less_equal_s_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef less_equal_u_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef greater_s_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef greater_u_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef greater_equal_s_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef greater_equal_u_int(CodeGenerator *cg, ASTExpr *e);

/// unary
LLVMValueRef neg_s_int(CodeGenerator *cg, ASTExpr *e);

/// conversion
// if both the integers are signed
LLVMValueRef from_s_int_to_s_int(CodeGenerator *cg, ASTExpr *e);

// if at least one integer is unsigned (it includes boolean type)
LLVMValueRef from_su_int_to_su_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef from_s_int_to_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef from_u_int_to_float(CodeGenerator *cg, ASTExpr *e);

/// -------------------------------------------------
/// OPERATORS on FLOATS
/// -------------------------------------------------

/// binary
LLVMValueRef sum_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef sub_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef mul_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef div_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef rem_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef equal_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef not_equal_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef less_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef less_equal_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef greater_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef greater_equal_float(CodeGenerator *cg, ASTExpr *e);

/// unary
LLVMValueRef neg_float(CodeGenerator *cg, ASTExpr *e);

/// conversion
LLVMValueRef from_float_to_float(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef from_float_to_s_int(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef from_float_to_u_int(CodeGenerator *cg, ASTExpr *e);

/// -------------------------------------------------
/// OPERATORS on BOOLEANS
/// -------------------------------------------------

/// binary
LLVMValueRef and_bool(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef or_bool(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef eq_bool(CodeGenerator *cg, ASTExpr *e);

LLVMValueRef neq_bool(CodeGenerator *cg, ASTExpr *e);

/// unary
LLVMValueRef not_bool(CodeGenerator *cg, ASTExpr *e);

/// conversion: see integers (since boolean type is implemented as Int)

#endif //CHORA_OPERATOR_H
