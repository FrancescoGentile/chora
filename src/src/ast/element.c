//
// Created by francesco on 05/12/21.
//

#include "element.h"

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

void init_function(Function *f, const StringSlice *name, Type *ret_type) {
    if (f != NULL) {
        f->name = *name;
        f->params = new_vector(FnParamRef);
        f->ret_type = ret_type;
    }
}

void free_parameter(FnParam *fp) {

    if (fp == NULL) { return; }

    free(fp->var);
}

void free_function(Function *f) {

    if (f == NULL) { return; }
    for (usize i = 0; i < f->params.length; ++i) {
        free_parameter(f->params.entries[i]);
        free(f->params.entries[i]);
    }
    f->params.free(&f->params);
}

// -------------------------------------------------

void init_block(Block *b, const StringSlice *s, const StringSlice *a, Type *must_ret, Vector(TypeRef) *possible) {
    if (b != NULL) {
        if (s != NULL) { b->name = *s; }
        else { init_string(&b->name, NULL, 0); }

        if (a != NULL) { b->alt_name = *a; }
        else { init_string(&b->alt_name, NULL, 0); }

        b->possible = possible;
        b->must_return = must_ret;
        b->not_return = false;
        b->last_is_jump = false;
        b->llvm_next = NULL;
        b->llvm_return = NULL;
    }
}

const StringSlice *get_block_name(const Block *b) {

    if (b == NULL) { return NULL; }

    if (b->name.len > 0) {
        return &b->name;
    } else {
        return &b->alt_name;
    }
}

// -------------------------------------------------

void init_operator(Operator *op, const StringSlice *n, Type *f, Type *s, Type *r) {
    if (op != NULL) {
        op->name = *n;
        op->first = f;
        op->second = s;
        op->ret_type = r;
    }
}

// -------------------------------------------------

void init_const_variable(Variable *v, const StringSlice *n, Type *t) {
    if (v != NULL) {
        v->kind = VAR_CONST;
        v->v_const.name = *n;
        v->v_const.type = t;
    }
}

void init_static_variable(Variable *v, const StringSlice *n, Type *t, bool mut) {
    if (v != NULL) {
        v->kind = VAR_STATIC;
        v->v_static.name = *n;
        v->v_static.type = t;
        v->v_static.mut = mut;
    }
}

void init_local_variable(Variable *v, const StringSlice *n, Type *t, bool mut) {
    if (v != NULL) {
        v->kind = VAR_LOCAL;
        v->v_local.name = *n;
        v->v_local.type = t;
        v->v_local.mut = mut;
    }
}

void init_param_variable(Variable *v, const StringSlice *n, Type *t, bool mut, usize pos) {
    if (v != NULL) {
        v->kind = VAR_FN_PARAM;
        v->v_param.name = *n;
        v->v_param.type = t;
        v->v_param.mut = mut;
        v->v_param.param_pos = pos;
    }
}

// -------------------------------------------------

const StringSlice *get_var_name(const Variable *v) {

    if (v == NULL) { return NULL; }

    switch (v->kind) {
        case VAR_CONST: {
            return &v->v_const.name;
        }
        case VAR_STATIC: {
            return &v->v_static.name;
        }
        case VAR_LOCAL: {
            return &v->v_local.name;
        }
        case VAR_FN_PARAM: {
            return &v->v_param.name;
        }
    }
}

Type *get_var_type(const Variable *v) {

    if (v == NULL) { return NULL; }

    switch (v->kind) {
        case VAR_CONST: {
            return v->v_const.type;
        }
        case VAR_STATIC: {
            return v->v_static.type;
        }
        case VAR_LOCAL: {
            return v->v_local.type;
        }
        case VAR_FN_PARAM: {
            return v->v_param.type;
        }
    }
}

bool is_var_mutable(const Variable *v) {

    if (v == NULL) { return false; }

    switch (v->kind) {
        case VAR_CONST: {
            return false;
        }
        case VAR_STATIC: {
            return v->v_static.mut;
        }
        case VAR_LOCAL: {
            return v->v_local.mut;
        }
        case VAR_FN_PARAM: {
            return v->v_param.mut;
        }
    }
}

LLVMValueRef llvm_get_var_value(const Variable *v) {

    if (v == NULL) { return NULL; }

    switch (v->kind) {
        case VAR_CONST: {
            return v->v_const.llvm_value;
        }
        case VAR_STATIC: {
            return v->v_static.llvm_value;
        }
        case VAR_LOCAL: {
            return v->v_local.llvm_value;
        }
        case VAR_FN_PARAM: {
            return v->v_param.llvm_value;
        }
    }
}

/// -------------------------------------------------
/// TYPE
/// -------------------------------------------------

void init_type(Type *t, TypeKind kind, const StringSlice *name) {
    t->kind = kind;
    t->name = *name;
    t->operators = new_multi_map(OperatorRef);
}

void free_type(Type *t) {

    if (t == NULL) { return; }

    for (usize i = 0; i < t->operators.capacity; ++i) {
        MultiEntry(OperatorRef) *entry = &t->operators.entries[i];
        if (entry->kind == PRESENT) {
            Vector(OperatorRef) *ops = &entry->values;
            for (usize j = 0; j < ops->length; ++j) {
                free(ops->entries[j]);
            }
        }
    }
    t->operators.free(&t->operators);
}

char *type_to_cstring(const Type *t) {
    if (t == NULL) {
        return to_cstring(NULL);
    } else {
        return to_cstring(&t->name);
    }
}

// -------------------------------------------------

bool is_numeric_type(const Type *t) {
    if (t == NULL) {
        return false;
    }
    switch (t->kind) {
        case TYPE_U8:
        case TYPE_U16:
        case TYPE_U32:
        case TYPE_U64:
        case TYPE_I8:
        case TYPE_I16:
        case TYPE_I32:
        case TYPE_I64:
        case TYPE_F32:
        case TYPE_F64: {
            return true;
        }
        default: {
            return false;
        }
    }
}

bool is_float_type(const Type *t) {

    if (t == NULL || t->kind != TYPE_F32 && t->kind != TYPE_F64) {
        return false;
    } else if (t->kind == TYPE_F32 || t->kind == TYPE_F64) {
        return true;
    }
}

bool is_integer_type(const Type *t) {
    if (is_numeric_type(t) && !is_float_type(t)) {
        return true;
    } else {
        return false;
    }
}

// -------------------------------------------------

Operator *get_prefix_operators(Type *t, const StringSlice *s) {

    Vector(OperatorRef) *res = t->operators.get(&t->operators, s);
    if (res == NULL) {
        return NULL;
    } else {
        for (usize i = 0; i < res->length; ++i) {
            Operator *op = res->entries[i];
            if (op->first->kind == TYPE_EMPTY) {
                return op;
            }
        }
        return NULL;
    }
}

void get_infix_operators(Type *t, const StringSlice *s, Vector(OperatorRef) *ops) {

    Vector(OperatorRef) *res = t->operators.get(&t->operators, s);
    if (res != NULL) {
        for (usize i = 0; i < res->length; ++i) {
            Operator *op = res->entries[i];
            if (op->first->kind != TYPE_EMPTY && op->second->kind != TYPE_EMPTY) {
                ops->push(ops, &op);
            }
        }
    }
}

Operator *get_postfix_operators(Type *t, const StringSlice *s) {

    Vector(OperatorRef) *res = t->operators.get(&t->operators, s);
    if (res == NULL) {
        return NULL;
    } else {
        for (usize i = 0; i < res->length; ++i) {
            Operator *op = res->entries[i];
            if (op->second->kind == TYPE_EMPTY) {
                return op;
            }
        }
        return NULL;
    }
}

Operator *get_conversion_operator(Type *from, Type *to) {

    StringSlice as;
    init_string(&as, "as", 2);

    Vector(OperatorRef) *ops = from->operators.get(&from->operators, &as);
    if (ops == NULL) {
        return NULL;
    } else {
        for (usize i = 0; i < ops->length; ++i) {
            Operator *op = ops->entries[i];
            if (op->ret_type == to) {
                return op;
            }
        }
        return NULL;
    }
}