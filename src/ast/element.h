//
// Created by francesco on 05/12/21.
//

#ifndef CHORA_ELEMENT_H
#define CHORA_ELEMENT_H

#include "../std/string.h"
#include "../std/vector/template.h"
#include "../std/map/map.h"
#include "../std/map/multimap.h"

#include <stdbool.h>
#include <llvm-c/Core.h>

/*
 * Basic elements that constitute any translation unit:
 * variables, functions and types
 */

typedef struct Function Function;
typedef struct Operator Operator;
typedef struct Type Type;
typedef struct Variable Variable;

/// -------------------------------------------------
/// FUNCTIONS
/// -------------------------------------------------

typedef struct ASTExpr ASTExpr;

typedef struct {
    StringSlice name;
    Type *type;
    bool is_mutable;
    ASTExpr *expr; // = NULL if the parameter has no default value

    // When we enter the function scope we add to the symbol table a variable
    // corresponding to the function parameter. We store a reference to that variable
    // so that we can set the appropriate 'var.llvm_value' during intermediate code generation.
    Variable *var; // = NULL is the parameter is irrelevant (i.e. '_')

    // This is the value referring to the formal parameter.
    // In var we store the value referring to the actual parameter.
    LLVMValueRef llvm_value;
} FnParam;

typedef FnParam *FnParamRef;
define_vector(FnParamRef)

typedef struct Function {
    StringSlice name;
    Vector(FnParamRef) params;
    Type *ret_type;

    LLVMValueRef llvm_fun;
} Function;

// -------------------------------------------------

void init_function(Function *f, const StringSlice *name, Type *ret_type);

void free_parameter(FnParam *fp);

void free_function(Function *f);

/// -------------------------------------------------
/// BLOCKS
/// -------------------------------------------------

typedef Type *TypeRef;
define_vector(TypeRef)

typedef struct {
    StringSlice name;
    // function, while, loop associated blocks may have two associated label
    // one is the name specified by the user, the other one is 'function', 'while'
    // and 'loop' respectively
    StringSlice alt_name;

    // this is set inside the symbol table
    usize level;

    // declared return type
    // e.g. a function return type, a variable type whose value is the block result
    Vector(TypeRef) *possible;

    // first encountered type
    // this type is used to check if all the type returned from a block corresponds
    // to the first one when no possible return type is specified
    Type *must_return;

    // this indicates if the control flow is always interrupted by a jump to an external block
    // if it is true, it means that this block has no return value
    bool not_return;
    // indicates if the last instruction of the block is a jump instruction
    bool last_is_jump;
    // when we must return from this block, jump to this llvm_value
    LLVMBasicBlockRef llvm_return;
    // when the block execution ends, jump to this llvm_block
    LLVMBasicBlockRef llvm_next;
} Block;

// -------------------------------------------------

void init_block(Block *b, const StringSlice *s, const StringSlice *a, Type *must_ret, Vector(TypeRef) *possible);

/// -------------------------------------------------
/// FUNCTIONS
/// -------------------------------------------------

struct CodeGenerator;

// This is a simplified version of an operator for built-in types operators.
// It must be modified when user defined types will be implemented.
struct Operator {
    StringSlice name;
    Type *first; // first parameter
    Type *second; // second parameter
    Type *ret_type;

    LLVMValueRef (*generate_op)(struct CodeGenerator *, ASTExpr *e);
};

// -------------------------------------------------

void init_operator(Operator *op, const StringSlice *n, Type *f, Type *s, Type *r);

/// -------------------------------------------------
/// TYPES
/// -------------------------------------------------

// DO NOT CHANGE THE ORDER
// the initialization of the corresponding types in the symbol table depends on this order
typedef enum {
    TYPE_NO_RETURN,
    TYPE_EMPTY,
    TYPE_BOOL,
    TYPE_U8, TYPE_U16, TYPE_U32, TYPE_U64,
    TYPE_I8, TYPE_I16, TYPE_I32, TYPE_I64,
    TYPE_F32, TYPE_F64,
    TYPES_COUNT,
} TypeKind;

typedef Operator *OperatorRef;
define_multi_map(OperatorRef)

struct Type {
    TypeKind kind;
    StringSlice name;
    MultiMap(OperatorRef) operators;

    LLVMTypeRef llvm_type;
};

// -------------------------------------------------

void init_type(Type *t, TypeKind kind, const StringSlice *name);

void free_type(Type *t);

char *type_to_cstring(const Type *t);

// -------------------------------------------------

bool is_numeric_type(const Type *t);

bool is_float_type(const Type *t);

bool is_integer_type(const Type *t);

// -------------------------------------------------

Operator *get_prefix_operators(Type *t, const StringSlice *s);

void get_infix_operators(Type *t, const StringSlice *s, Vector(OperatorRef) *ops);

Operator *get_postfix_operators(Type *t, const StringSlice *s);

Operator *get_conversion_operator(Type *from, Type *to);

/// -------------------------------------------------
/// VARIABLE
/// -------------------------------------------------

typedef enum {
    VAR_CONST,
    VAR_STATIC,
    VAR_LOCAL,
    VAR_FN_PARAM
} VarKind;

typedef struct {
    StringSlice name;
    Type *type;
    LLVMValueRef llvm_value;
} ConstVar;

typedef struct {
    StringSlice name;
    Type *type;
    bool mut;
    LLVMValueRef llvm_value;
} StaticVar;

typedef struct {
    StringSlice name;
    Type *type;
    bool mut;
    LLVMValueRef llvm_value;
} LocalVar;

typedef struct {
    StringSlice name;
    Type *type;
    bool mut;
    usize param_pos;
    LLVMValueRef llvm_value;
} FnParamVar;

struct Variable {
    VarKind kind;
    union {
        ConstVar v_const;
        StaticVar v_static;
        LocalVar v_local;
        FnParamVar v_param;
    };
};

// -------------------------------------------------

void init_const_variable(Variable *v, const StringSlice *n, Type *t);

void init_static_variable(Variable *v, const StringSlice *n, Type *t, bool mut);

void init_local_variable(Variable *v, const StringSlice *n, Type *t, bool mut);

void init_param_variable(Variable *v, const StringSlice *n, Type *t, bool mut, usize pos);

// -------------------------------------------------

const StringSlice *get_var_name(const Variable *v);

Type *get_var_type(const Variable *v);

bool is_var_mutable(const Variable *v);

LLVMValueRef llvm_get_var_value(const Variable *v);

#endif //CHORA_ELEMENT_H
