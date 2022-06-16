//
// Created by francesco on 03/12/21.
//

#ifndef CHORA_AST_H
#define CHORA_AST_H

#include "../lexer/token.h"
#include "../std/vector/template.h"
#include "table.h"
#include "element.h"

#include <llvm-c/Core.h>

/// -------------------------------------------------
/// TRANSLATION UNIT
/// -------------------------------------------------

typedef struct ASTItem ASTItem;
typedef ASTItem *ASTItemRef;
define_vector(ASTItemRef)

typedef struct {
    Vector(ASTItemRef) items;
} ASTFile;

// -------------------------------------------------

void free_ast_file(ASTFile *f);

/// -------------------------------------------------
/// GLOBAL ITEMS
/// -------------------------------------------------

typedef enum {
    IT_CONST,
    IT_STATIC,
    IT_FUNCTION
} ASTItemKind;

typedef struct ASTPattern ASTPattern;
typedef struct ASTType ASTType;
typedef struct ASTExpr ASTExpr;

typedef struct {
    ASTPattern *pattern;
    ASTType *type;
    ASTExpr *expr;
    Variable *var;
} ASTConst;

typedef struct {
    ASTPattern *pattern;
    ASTType *type;
    ASTExpr *expr;
    Variable *var;
} ASTStatic;

typedef struct {
    ASTPattern *pattern;
    ASTType *type;
    ASTExpr *expr;

    FnParam *param;
} ASTFnParam;

typedef ASTFnParam *ASTFnParamRef;
define_vector(ASTFnParamRef)

typedef struct ASTBlock ASTBlock;

typedef struct {
    Token fn_lex;
    Token name;

    Vector(ASTFnParamRef) params;
    ASTType *ret_type;
    ASTBlock *body;

    Function *fun;
} ASTFunction;

struct ASTItem {
    ASTItemKind kind;
    union {
        ASTConst it_const;
        ASTStatic it_static;
        ASTFunction it_function;
    };
};

// -------------------------------------------------

void free_ast_item(ASTItem *it);

/// -------------------------------------------------
/// EXPRESSIONS
/// -------------------------------------------------

/// -------------------------------------------------
/// INTERRUPT
/// -------------------------------------------------

typedef struct ASTLabel ASTLabel;

typedef struct {
    Token break_lex;
    ASTLabel *label;
    ASTExpr *expr;
    Block *block;
} ASTBreak;

typedef struct {
    Token ret_lex;
    ASTLabel *label;
    ASTExpr *expr;
    Block *block;
} ASTReturn;

/// -------------------------------------------------
/// OPERATOR EXPRESSIONS
/// -------------------------------------------------

typedef struct {
    bool is_prefix;
    Token op_id;
    ASTExpr *expr;
    Operator *op;
} ASTUnary;

typedef struct {
    Token op_id;
    ASTExpr *left;
    ASTExpr *right;
    Operator *op;
} ASTBinary;

/// -------------------------------------------------
/// VARIABLE EXPRESSIONS
/// -------------------------------------------------

typedef struct {
    Token id;
    Variable *var;
} ASTVariable;

/// -------------------------------------------------
/// LITERAL EXPRESSIONS
/// -------------------------------------------------

typedef enum {
    LIT_INT,
    LIT_FLOAT,
    LIT_BOOL,
} ASTLiteralKind;

typedef struct {
    ASTLiteralKind kind;
    Token original;
    Type *type;
    union {
        bool bool_lit;
        u64 int_lit;
        f64 float_lit;
    };
} ASTLiteral;

/// -------------------------------------------------
/// FUNCTION CALLS
/// -------------------------------------------------

typedef struct {
    Token *name;
    usize arg_number;
    // If a default parameter is used, expr points to the expression specified
    // in the function parameter declaration (we do not copy the expression, but only the pointer).
    // Therefore, when we deallocate the AST we do not have to free expr if the parameter
    // does not own the expr pointer.
    bool own_expr;
    ASTExpr *expr;
} ASTFnArgument;

typedef ASTFnArgument *ASTFnArgumentRef;
define_vector(ASTFnArgumentRef)

typedef struct {
    Token id;
    Vector(ASTFnArgumentRef) arguments;
    Function *fun;
} ASTFnCall;

/// -------------------------------------------------
/// BLOCK EXPRESSIONS
/// -------------------------------------------------

typedef struct ASTStmt ASTStmt;
typedef ASTStmt *ASTStmtRef;
define_vector(ASTStmtRef)

struct ASTBlock {
    ASTLabel *label;
    Token open;
    Token close;
    Vector(ASTStmtRef) stmts;
    ASTExpr *expr;

    Block *block;
};

// -------------------------------------------------

void free_ast_block(ASTBlock *b);

Type *get_block_type(const ASTBlock *b, bool without_int);

void llvm_set_return_block(ASTBlock *b, LLVMBasicBlockRef llvm);

LLVMBasicBlockRef llvm_get_return_block(const ASTBlock *b);

void llvm_set_next_block(ASTBlock *b, LLVMBasicBlockRef llvm);

LLVMBasicBlockRef llvm_get_next_block(const ASTBlock *b);

/// -------------------------------------------------
/// LOOP EXPRESSION
/// -------------------------------------------------

typedef struct {
    Token loop_lex;
    ASTBlock *body;
} ASTLoop;

typedef struct {
    Token while_lex;
    ASTExpr *condition;
    ASTBlock *body;
} ASTWhile;

/// -------------------------------------------------
/// IF - ELSE EXPRESSION
/// -------------------------------------------------

typedef struct ASTElse ASTElse;

typedef struct {
    Token if_lex;
    ASTExpr *condition;
    ASTBlock *if_body;
    ASTElse *else_body;
} ASTIf;

typedef enum {
    ELSE_IF,
    ELSE_BLOCK
} ASTElseKind;

struct ASTElse {
    ASTElseKind kind;
    union {
        ASTIf else_if;
        ASTBlock else_block;
    };
};

Type *get_if_return_type(const ASTIf *i);

// -------------------------------------------------

typedef struct {
    Token open;
    ASTExpr *expr;
    Token close;
} ASTGrouped;

typedef struct {
    ASTExpr *expr;
    ASTType *type;
    Operator *op;
} ASTAs;

typedef struct {
    ASTExpr *left;
    ASTExpr *right;
    Token eq;
} ASTAssign;

/// -------------------------------------------------
/// EXPRESSION
/// -------------------------------------------------

typedef enum {
    //EXPR_CONTINUE,
    EXPR_BREAK,
    EXPR_RETURN,
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_FN_CALL,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_LOOP,
    EXPR_WHILE,
    EXPR_IF,
    EXPR_BLOCK,
    EXPR_GROUPED,
    EXPR_AS,
    EXPR_ASSIGN,
} ASTExprKind;

struct ASTExpr {
    ASTExprKind kind;
    // when freeing the AST, we need to know if the expression is valid, since the pointers in the union structs
    // may not be NULL initialized (thus, we may free pointers to wrong memory locations).
    bool is_valid;
    LLVMValueRef llvm_value;
    union {
        //ASTContinue e_continue;
        ASTBreak e_break;
        ASTReturn e_return;
        ASTLiteral e_literal;
        ASTVariable e_var;
        ASTUnary e_unary;
        ASTBinary e_binary;
        ASTFnCall e_fn_call;
        ASTLoop e_loop;
        ASTWhile e_while;
        ASTIf e_if;
        ASTBlock e_block;
        ASTGrouped e_grouped;
        ASTAs e_as;
        ASTAssign e_assign;
    };
};

// -------------------------------------------------

void free_ast_expr(ASTExpr *e);

Type *get_expr_type(const ASTExpr *e);

bool is_expr_with_block(const ASTExpr *e);

bool is_expr_var(const ASTExpr *e);

Variable *get_expr_var(const ASTExpr *e);

/// if the expression returns from a block, this functions returns that block, otherwise NULL
Block *get_expr_return_block(const ASTExpr *e);

const Coordinates *get_expr_start(const ASTExpr *e);

const Coordinates *get_expr_end(const ASTExpr *e);

/// -------------------------------------------------
/// STATEMENTS
/// -------------------------------------------------

typedef enum {
    STMT_ITEM,
    STMT_LOCAL,
    STMT_EXPR
} ASTStmtKind;

typedef struct {
    ASTPattern *pattern;
    ASTType *type;
    ASTExpr *expr;

    Variable *var;
} ASTLocal;

struct ASTStmt {
    ASTStmtKind kind;
    union {
        ASTItem item;
        ASTLocal local;
        ASTExpr expr;
    };
};

// -------------------------------------------------

void free_ast_stmt(ASTStmt *st);

Block *get_stmt_return_block(const ASTStmt *);

/// -------------------------------------------------
/// LABEL
/// -------------------------------------------------

struct ASTLabel {
    Token name;
};

// -------------------------------------------------

const StringSlice *get_label_string(const ASTLabel *l);

/// -------------------------------------------------
/// PATTERN
/// -------------------------------------------------

typedef enum {
    PT_IDENTIFIER,
    PT_WILDCARD
} ASTPatternKind;

typedef struct {
    Token *mutable;
    Token identifier;
} ASTIdPattern;

struct ASTPattern {
    ASTPatternKind kind;
    union {
        ASTIdPattern id;
        Token underscore;
    };
};

// -------------------------------------------------

void free_ast_pattern(ASTPattern *p);

bool is_named_pattern(const ASTPattern *p);

bool is_mutable_pattern(const ASTPattern *p);

const StringSlice *get_pattern_id(const ASTPattern *p);

const Coordinates *get_pattern_start(const ASTPattern *p);

const Coordinates *get_pattern_end(const ASTPattern *p);

const Coordinates *get_pattern_mut_start(const ASTPattern *p);

const Coordinates *get_pattern_mut_end(const ASTPattern *p);

const Coordinates *get_pattern_id_start(const ASTPattern *p);

const Coordinates *get_pattern_id_end(const ASTPattern *p);

/// -------------------------------------------------
/// TYPE
/// -------------------------------------------------

typedef enum {
    TY_EMPTY,
    TY_INFERRED,
    TY_IDENTIFIER,
    TY_PAREN,
} ASTTypeKind;

typedef struct {
    Token open;
    ASTType *type;
    Token close;
} ASTParenType;

typedef struct {
    Token open;
    Token close;
} ASTEmptyType;

struct ASTType {
    ASTTypeKind kind;
    Type *type;
    union {
        ASTParenType t_paren;
        ASTEmptyType t_empty;
        Token symbol;
    };
};

// -------------------------------------------------

void free_ast_type(ASTType *t);

bool is_inferred_type(const ASTType *t);

Type *get_type(const ASTType *t);

const Coordinates *get_type_start(const ASTType *t);

const Coordinates *get_type_end(const ASTType *t);

#endif //CHORA_AST_H
