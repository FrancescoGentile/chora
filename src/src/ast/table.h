//
// Created by francesco on 04/12/21.
//

#ifndef CHORA_TABLE_H
#define CHORA_TABLE_H

#include "../std/string.h"
#include "../std/vector/template.h"
#include "../std/map/multimap.h"
#include "../std/types.h"
#include "element.h"

#include <stdbool.h>
#include <llvm-c/Core.h>

typedef Variable *VariableRef;
typedef Type *TypeRef;
typedef Function *FunctionRef;

define_multi_map(FunctionRef)

define_multi_map(VariableRef)

define_map(TypeRef)

typedef struct Scope Scope;

struct Scope {
    MultiMap(VariableRef) variables;
    MultiMap(FunctionRef) functions;
    Map(TypeRef) types;
    Block *block;

    Scope *parent;
    usize level;
};

typedef struct {
    Scope *current;
} SymbolTable;

// -------------------------------------------------

void init_symbol_table(SymbolTable *st);

void free_symbol_table(SymbolTable *st);

// -------------------------------------------------

usize get_scope_level(const SymbolTable *st);

void enter_scope(SymbolTable *st);

bool leave_scope(SymbolTable *st);

// -------------------------------------------------

bool add_function(SymbolTable *st, Function *f_ins);

bool add_type(SymbolTable *st, Type *t);

bool add_variable(SymbolTable *st, Variable *v, bool redef);

bool add_block(SymbolTable *st, Block *b);

// -------------------------------------------------

Type *search_type(const SymbolTable *st, const StringSlice *name);

Type *get_builtin_type(TypeKind tk);

// -------------------------------------------------

Variable **get_variable(const SymbolTable *st, const StringSlice *name);

Vector(FunctionRef) *get_functions(const SymbolTable *st, const StringSlice *name);

Block *get_internal_block(const SymbolTable *st, const StringSlice *name);

Block *get_external_block(const SymbolTable *st, const StringSlice *name);


#endif //CHORA_TABLE_H
