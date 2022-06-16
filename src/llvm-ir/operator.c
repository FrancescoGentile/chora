//
// Created by francesco on 08/12/21.
//

#include "operator.h"
#include "stmt_expr.h"

/// -------------------------------------------------
/// OPERATORS on INTEGERS
/// -------------------------------------------------

LLVMValueRef sum_su_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildAdd(cg->builder, lhs, rhs, "");
}

LLVMValueRef sub_su_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildSub(cg->builder, lhs, rhs, "");
}

LLVMValueRef mul_su_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildMul(cg->builder, lhs, rhs, "");
}

LLVMValueRef div_s_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildSDiv(cg->builder, lhs, rhs, "");
}

LLVMValueRef div_u_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildUDiv(cg->builder, lhs, rhs, "");
}

LLVMValueRef rem_s_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildSRem(cg->builder, lhs, rhs, "");
}

LLVMValueRef rem_u_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildURem(cg->builder, lhs, rhs, "");
}

LLVMValueRef and_su_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildAnd(cg->builder, lhs, rhs, "");
}

LLVMValueRef or_su_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildOr(cg->builder, lhs, rhs, "");
}

LLVMValueRef xor_su_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildXor(cg->builder, lhs, rhs, "");
}

LLVMValueRef shl_su_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildShl(cg->builder, lhs, rhs, "");
}

LLVMValueRef shr_s_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildAShr(cg->builder, lhs, rhs, "");
}

LLVMValueRef shr_u_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildLShr(cg->builder, lhs, rhs, "");
}

LLVMValueRef equal_su_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntEQ, lhs, rhs, "");
}

LLVMValueRef not_equal_su_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntNE, lhs, rhs, "");
}

LLVMValueRef less_s_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntSLT, lhs, rhs, "");
}

LLVMValueRef less_u_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntULT, lhs, rhs, "");
}

LLVMValueRef less_equal_s_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntSLE, lhs, rhs, "");
}

LLVMValueRef less_equal_u_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntULE, lhs, rhs, "");
}

LLVMValueRef greater_s_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntSGT, lhs, rhs, "");
}

LLVMValueRef greater_u_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntUGT, lhs, rhs, "");
}

LLVMValueRef greater_equal_s_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntSGE, lhs, rhs, "");
}

LLVMValueRef greater_equal_u_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntUGE, lhs, rhs, "");
}

LLVMValueRef neg_s_int(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef expr = generate_expression(cg, e->e_unary.expr);
    return LLVMBuildNeg(cg->builder, expr, "");
}

LLVMValueRef from_s_int_to_s_int(CodeGenerator *cg, ASTExpr *e) {
    Type *to = e->e_as.type->type;
    LLVMValueRef val = generate_expression(cg, e->e_as.expr);
    return LLVMBuildIntCast2(cg->builder, val, to->llvm_type, true, "");
}

LLVMValueRef from_su_int_to_su_int(CodeGenerator *cg, ASTExpr *e) {
    Type *to = e->e_as.type->type;
    LLVMValueRef val = generate_expression(cg, e->e_as.expr);
    return LLVMBuildIntCast2(cg->builder, val, to->llvm_type, false, "");
}

LLVMValueRef from_s_int_to_float(CodeGenerator *cg, ASTExpr *e) {
    Type *to = e->e_as.type->type;
    LLVMValueRef val = generate_expression(cg, e->e_as.expr);
    return LLVMBuildSIToFP(cg->builder, val, to->llvm_type, "");
}

LLVMValueRef from_u_int_to_float(CodeGenerator *cg, ASTExpr *e) {
    Type *to = e->e_as.type->type;
    LLVMValueRef val = generate_expression(cg, e->e_as.expr);
    return LLVMBuildUIToFP(cg->builder, val, to->llvm_type, "");
}

/// -------------------------------------------------
/// OPERATORS on FLOAT
/// -------------------------------------------------

LLVMValueRef sum_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFAdd(cg->builder, lhs, rhs, "");
}

LLVMValueRef sub_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFSub(cg->builder, lhs, rhs, "");
}

LLVMValueRef mul_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFMul(cg->builder, lhs, rhs, "");
}

LLVMValueRef div_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFDiv(cg->builder, lhs, rhs, "");
}

LLVMValueRef rem_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFRem(cg->builder, lhs, rhs, "");
}

LLVMValueRef equal_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFCmp(cg->builder, LLVMRealOEQ, lhs, rhs, "");
}

LLVMValueRef not_equal_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFCmp(cg->builder, LLVMRealONE, lhs, rhs, "");
}

LLVMValueRef less_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFCmp(cg->builder, LLVMRealOLT, lhs, rhs, "");
}

LLVMValueRef less_equal_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFCmp(cg->builder, LLVMRealOLE, lhs, rhs, "");
}

LLVMValueRef greater_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFCmp(cg->builder, LLVMRealOGT, lhs, rhs, "");
}

LLVMValueRef greater_equal_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildFCmp(cg->builder, LLVMRealOGE, lhs, rhs, "");
}

LLVMValueRef neg_float(CodeGenerator *cg, ASTExpr *e) {
    LLVMValueRef expr = generate_expression(cg, e->e_unary.expr);
    return LLVMBuildFNeg(cg->builder, expr, "");
}

LLVMValueRef from_float_to_float(CodeGenerator *cg, ASTExpr *e) {
    Type *to = e->e_as.type->type;
    LLVMValueRef val = generate_expression(cg, e->e_as.expr);
    return LLVMBuildFPCast(cg->builder, val, to->llvm_type, "");
}

LLVMValueRef from_float_to_s_int(CodeGenerator *cg, ASTExpr *e) {
    Type *to = e->e_as.type->type;
    LLVMValueRef val = generate_expression(cg, e->e_as.expr);
    return LLVMBuildFPToSI(cg->builder, val, to->llvm_type, "");
}

LLVMValueRef from_float_to_u_int(CodeGenerator *cg, ASTExpr *e) {
    Type *to = e->e_as.type->type;
    LLVMValueRef val = generate_expression(cg, e->e_as.expr);
    return LLVMBuildFPToUI(cg->builder, val, to->llvm_type, "");
}

/// -------------------------------------------------
/// OPERATORS on BOOLEANS
/// -------------------------------------------------

LLVMValueRef and_bool(CodeGenerator *cg, ASTExpr *e) {

    ASTBinary *b = &e->e_binary;
    LLVMValueRef llvm_true = LLVMConstInt(LLVMInt1TypeInContext(cg->ctx), true, false);
    LLVMValueRef llvm_false = LLVMConstInt(LLVMInt1TypeInContext(cg->ctx), false, false);

    // generate lhs and check if it is true. If it is true, jump to final, otherwise jump to or_else
    LLVMValueRef lhs = generate_expression(cg, b->left);
    LLVMValueRef is_lhs_false = LLVMBuildICmp(cg->builder, LLVMIntEQ, lhs, llvm_false, "");

    LLVMBasicBlockRef and_false = LLVMGetInsertBlock(cg->builder);
    LLVMBasicBlockRef and_true = LLVMCreateBasicBlockInContext(cg->ctx, "and_true");
    LLVMBasicBlockRef final = LLVMCreateBasicBlockInContext(cg->ctx, "and_final");
    LLVMBuildCondBr(cg->builder, is_lhs_false, final, and_true);

    // if lhs is false, generate rhs
    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, and_true);
    LLVMPositionBuilderAtEnd(cg->builder, and_true);
    LLVMValueRef rhs = generate_expression(cg, b->right);
    LLVMValueRef is_rhs_true = LLVMBuildICmp(cg->builder, LLVMIntEQ, rhs, llvm_true, "");
    LLVMBuildBr(cg->builder, final);
    and_true = LLVMGetInsertBlock(cg->builder);

    // select the final value
    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, final);
    LLVMPositionBuilderAtEnd(cg->builder, final);
    LLVMValueRef res = LLVMBuildPhi(cg->builder, LLVMInt1TypeInContext(cg->ctx), "");
    LLVMAddIncoming(res, &llvm_false, &and_false, 1);
    LLVMAddIncoming(res, &is_rhs_true, &and_true, 1);
    return res;
}

LLVMValueRef or_bool(CodeGenerator *cg, ASTExpr *e) {

    ASTBinary *b = &e->e_binary;
    LLVMValueRef llvm_true = LLVMConstInt(LLVMInt1TypeInContext(cg->ctx), true, false);

    // generate lhs and check if it is true. If it is true, jump to final, otherwise jump to or_else
    LLVMValueRef lhs = generate_expression(cg, b->left);
    LLVMValueRef is_lhs_true = LLVMBuildICmp(cg->builder, LLVMIntEQ, lhs, llvm_true, "");

    LLVMBasicBlockRef or_true = LLVMGetInsertBlock(cg->builder);
    LLVMBasicBlockRef or_false = LLVMCreateBasicBlockInContext(cg->ctx, "or_false");
    LLVMBasicBlockRef final = LLVMCreateBasicBlockInContext(cg->ctx, "or_final");
    LLVMBuildCondBr(cg->builder, is_lhs_true, final, or_false);

    // if lhs is false, generate rhs
    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, or_false);
    LLVMPositionBuilderAtEnd(cg->builder, or_false);
    LLVMValueRef rhs = generate_expression(cg, b->right);
    LLVMValueRef is_rhs_true = LLVMBuildICmp(cg->builder, LLVMIntEQ, rhs, llvm_true, "");
    LLVMBuildBr(cg->builder, final);
    or_false = LLVMGetInsertBlock(cg->builder);

    // select the final value
    LLVMInsertExistingBasicBlockAfterInsertBlock(cg->builder, final);
    LLVMPositionBuilderAtEnd(cg->builder, final);
    LLVMValueRef res = LLVMBuildPhi(cg->builder, LLVMInt1TypeInContext(cg->ctx), "");
    LLVMAddIncoming(res, &llvm_true, &or_true, 1);
    LLVMAddIncoming(res, &is_rhs_true, &or_false, 1);
    return res;
}

LLVMValueRef eq_bool(CodeGenerator *cg, ASTExpr *e) {

    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntEQ, lhs, rhs, "");
}

LLVMValueRef neq_bool(CodeGenerator *cg, ASTExpr *e) {

    LLVMValueRef lhs = generate_expression(cg, e->e_binary.left);
    LLVMValueRef rhs = generate_expression(cg, e->e_binary.right);
    return LLVMBuildICmp(cg->builder, LLVMIntNE, lhs, rhs, "");
}

LLVMValueRef not_bool(CodeGenerator *cg, ASTExpr *e) {

    LLVMValueRef expr = generate_expression(cg, e->e_unary.expr);
    return LLVMBuildNeg(cg->builder, expr, "");
}