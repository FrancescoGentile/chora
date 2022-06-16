//
// Created by francesco on 04/12/21.
//

#include "table.h"
#include "../llvm-ir/operator.h"

#include <string.h>
#include <stdio.h>

//// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

/*
 * Infix operators: +, -, *, /
 * Prefix operators: -
 * Postfix operators: none
 */

static Type *types[TYPES_COUNT];

static const char *as_name = "as";

static void create_no_return_type(SymbolTable *st) {

    static char *type_name = "!";
    StringSlice name;
    init_string(&name, type_name, 1);

    types[TYPE_NO_RETURN] = (Type *) calloc(1, sizeof(Type));
    init_type(types[TYPE_NO_RETURN], TYPE_NO_RETURN, &name);

    add_type(st, types[TYPE_NO_RETURN]);
}

static void create_empty_type(SymbolTable *st) {

    static char *type_name = "()";
    StringSlice name;
    init_string(&name, type_name, 2);

    types[TYPE_EMPTY] = (Type *) calloc(1, sizeof(Type));
    init_type(types[TYPE_EMPTY], TYPE_EMPTY, &name);

    add_type(st, types[TYPE_EMPTY]);
}

static void create_bool_binary_ops() {

    static char *infix_ops_name[] = {"&&", "||", "==", "!="};

    usize op_count = sizeof(infix_ops_name) / sizeof(char *);
    StringSlice infix_ops[op_count];
    for (usize i = 0; i < op_count; ++i) {
        init_string(&infix_ops[i], infix_ops_name[i], strlen(infix_ops_name[i]));
    }

    for (usize i = 0; i < op_count; ++i) {
        Type *type = types[TYPE_BOOL];
        Operator *op = (Operator *) calloc(1, sizeof(Operator));
        init_operator(op, &infix_ops[i], type, type, type);
        type->operators.insert(&type->operators, &op->name, &op);

        switch (i) {
            case 0: {
                op->generate_op = and_bool;
                break;
            }
            case 1: {
                op->generate_op = or_bool;
                break;
            }
            case 2: {
                op->generate_op = eq_bool;
                break;
            }
            case 3: {
                op->generate_op = neq_bool;
                break;
            }
            default: {
                fprintf(stderr, "CODE-ERROR: strange operator in boolean operators initialization\n");
                exit(1);
            }
        }
    }
}

static void create_bool_unary_ops() {

    static char *op_name = "!";
    StringSlice op_str;
    init_string(&op_str, op_name, strlen(op_name));

    Operator *op = (Operator *) calloc(1, sizeof(Operator));
    init_operator(op, &op_str, types[TYPE_EMPTY], types[TYPE_BOOL], types[TYPE_BOOL]);
    types[TYPE_BOOL]->operators.insert(&types[TYPE_BOOL]->operators, &op->name, &op);
    op->generate_op = not_bool;
}

static void create_bool_conversions() {

    StringSlice op_str;
    init_string(&op_str, as_name, strlen(as_name));

    Type *from = types[TYPE_BOOL];

    for (usize i = TYPE_BOOL; i <= TYPE_F64; ++i) {
        Type *to = types[i];
        Operator *op = (Operator *) calloc(1, sizeof(Operator));
        init_operator(op, &op_str, from, types[TYPE_EMPTY], to);
        from->operators.insert(&from->operators, &op_str, &op);

        if (i <= TYPE_I64) {
            op->generate_op = from_su_int_to_su_int;
        } else {
            op->generate_op = from_u_int_to_float;
        }
    }
}

static void create_bool_type(SymbolTable *st) {

    static char *type_name = "bool";
    StringSlice name;
    init_string(&name, type_name, strlen(type_name));

    types[TYPE_BOOL] = (Type *) calloc(1, sizeof(Type));
    init_type(types[TYPE_BOOL], TYPE_BOOL, &name);
    add_type(st, types[TYPE_BOOL]);
}

static void create_int_binary_ops() {

    static char *op_name[] = {"+", "-", "*", "/", "%", // return same type
                              "&", "|", "^", "<<", ">>", // return same type
                              "==", "!=", "<", "<=", ">", ">="}; // return bool

    usize ops_count = sizeof(op_name) / sizeof(char *);
    StringSlice ops[ops_count];
    for (usize i = 0; i < ops_count; ++i) {
        init_string(&ops[i], op_name[i], strlen(op_name[i]));
    }

    for (usize i = TYPE_U8; i <= TYPE_I64; ++i) {
        Type *type = types[i];

        for (usize j = 0; j < ops_count; ++j) {
            Type *ret_type = (j < 10 ? type : types[TYPE_BOOL]);
            Operator *op = (Operator *) calloc(1, sizeof(Operator));
            init_operator(op, &ops[j], type, type, ret_type);
            type->operators.insert(&type->operators, &ops[j], &op);

            switch (j) {
                case 0: {
                    op->generate_op = sum_su_int;
                    break;
                }
                case 1: {
                    op->generate_op = sub_su_int;
                    break;
                }
                case 2: {
                    op->generate_op = mul_su_int;
                    break;
                }
                case 3: {
                    if (i <= TYPE_U64) {
                        op->generate_op = div_u_int;
                    } else {
                        op->generate_op = div_s_int;
                    }
                    break;
                }
                case 4: {
                    if (i <= TYPE_U64) {
                        op->generate_op = rem_u_int;
                    } else {
                        op->generate_op = rem_s_int;
                    }
                    break;
                }
                case 5: {
                    op->generate_op = and_su_int;
                    break;
                }
                case 6: {
                    op->generate_op = or_su_int;
                    break;
                }
                case 7: {
                    op->generate_op = xor_su_int;
                    break;
                }
                case 8: {
                    op->generate_op = shl_su_int;
                    break;
                }
                case 9: {
                    if (i <= TYPE_U64) {
                        op->generate_op = shr_u_int;
                    } else {
                        op->generate_op = shr_s_int;
                    }
                    break;
                }
                case 10: {
                    op->generate_op = equal_su_int;
                    break;
                }
                case 11: {
                    op->generate_op = not_equal_su_int;
                }
                case 12: {
                    if (i <= TYPE_U64) {
                        op->generate_op = less_u_int;
                    } else {
                        op->generate_op = less_s_int;
                    }
                    break;
                }
                case 13: {
                    if (i <= TYPE_U64) {
                        op->generate_op = less_equal_u_int;
                    } else {
                        op->generate_op = less_equal_s_int;
                    }
                    break;
                }
                case 14: {
                    if (i <= TYPE_U64) {
                        op->generate_op = greater_u_int;
                    } else {
                        op->generate_op = greater_s_int;
                    }
                    break;
                }
                case 15: {
                    if (i <= TYPE_U64) {
                        op->generate_op = greater_equal_u_int;
                    } else {
                        op->generate_op = greater_equal_s_int;
                    }
                    break;
                }
                default: {
                    fprintf(stderr, "CODE-ERROR: strange operator in integer binary operators initialization\n");
                    exit(1);
                }
            }
        }
    }
}

static void create_int_unary_ops() {

    static char *op_name = "-";
    StringSlice op_str;
    init_string(&op_str, op_name, strlen(op_name));

    for (usize i = TYPE_I8; i <= TYPE_I64; ++i) {
        Type *type = types[i];
        Operator *op = (Operator *) calloc(1, sizeof(Operator));
        init_operator(op, &op_str, types[TYPE_EMPTY], type, type);
        op->generate_op = neg_s_int;

        type->operators.insert(&type->operators, &op_str, &op);
    }
}

static void create_int_conversions() {

    StringSlice op_str;
    init_string(&op_str, as_name, strlen(as_name));

    for (usize i = TYPE_U8; i <= TYPE_I64; ++i) {
        Type *from = types[i];
        for (usize j = TYPE_BOOL; j <= TYPE_F64; ++j) {
            Type *to = types[j];

            Operator *op = (Operator *) calloc(1, sizeof(Operator));
            init_operator(op, &op_str, from, types[TYPE_EMPTY], to);
            from->operators.insert(&from->operators, &op_str, &op);

            switch (i) {
                case TYPE_U8:
                case TYPE_U16:
                case TYPE_U32:
                case TYPE_U64: {
                    if (j <= TYPE_I64) {
                        op->generate_op = from_su_int_to_su_int;
                    } else {
                        op->generate_op = from_u_int_to_float;
                    }
                    break;
                }
                case TYPE_I8:
                case TYPE_I16:
                case TYPE_I32:
                case TYPE_I64: {
                    if (j <= TYPE_U64) {
                        op->generate_op = from_su_int_to_su_int;
                    } else if (j <= TYPE_I64) {
                        op->generate_op = from_s_int_to_s_int;
                    } else {
                        op->generate_op = from_s_int_to_float;
                    }
                    break;
                }
                default: {
                    fprintf(stderr,
                            "CODE-ERROR (types-initialization): strange type when initializing integer conversions\n");
                    exit(1);
                }
            }
        }
    }
}

static void create_int_types(SymbolTable *st) {

    static char *type_name[] = {"u8", "u16", "u32", "u64",
                                "i8", "i16", "i32", "i64"};


    usize types_count = sizeof(type_name) / sizeof(char *);
    StringSlice names[types_count];
    for (usize i = 0; i < types_count; ++i) {
        init_string(&names[i], type_name[i], strlen(type_name[i]));
    }

    for (usize i = TYPE_U8, j = 0; i <= TYPE_I64; ++i, ++j) {
        types[i] = (Type *) calloc(1, sizeof(Type));
        init_type(types[i], i, &names[j]);
        add_type(st, types[i]);
    }
}

static void create_float_binary_ops() {

    static char *op_name[] = {"+", "-", "*", "/", "%", // return same type
                              "==", "!=", "<", "<=", ">", ">="}; // return bool

    usize ops_count = sizeof(op_name) / sizeof(char *);
    StringSlice ops[ops_count];
    for (usize i = 0; i < ops_count; ++i) {
        init_string(&ops[i], op_name[i], strlen(op_name[i]));
    }

    for (usize i = TYPE_F32; i <= TYPE_F64; ++i) {
        Type *type = types[i];

        for (usize j = 0; j < ops_count; ++j) {
            Type *ret_type = (j < 5 ? type : types[TYPE_BOOL]);
            Operator *op = (Operator *) calloc(1, sizeof(Operator));
            init_operator(op, &ops[j], type, type, ret_type);
            type->operators.insert(&type->operators, &ops[j], &op);

            switch (j) {
                case 0: {
                    op->generate_op = sum_float;
                    break;
                }
                case 1: {
                    op->generate_op = sub_float;
                    break;
                }
                case 2: {
                    op->generate_op = mul_float;
                    break;
                }
                case 3: {
                    op->generate_op = div_float;
                    break;
                }
                case 4: {
                    op->generate_op = rem_float;
                    break;
                }
                case 5: {
                    op->generate_op = equal_float;
                    break;
                }
                case 6: {
                    op->generate_op = not_equal_float;
                    break;
                }
                case 7: {
                    op->generate_op = less_float;
                    break;
                }
                case 8: {
                    op->generate_op = less_equal_float;
                    break;
                }
                case 9: {
                    op->generate_op = greater_float;
                    break;
                }
                case 10: {
                    op->generate_op = greater_equal_float;
                    break;
                }
                default: {
                    fprintf(stderr, "CODE-ERROR: strange operator in float binary operators initialization\n");
                    exit(1);
                }
            }
        }
    }
}

static void create_float_unary_ops() {

    static char *op_name = "-";
    StringSlice op_str;
    init_string(&op_str, op_name, strlen(op_name));

    for (usize i = TYPE_F32; i <= TYPE_F64; ++i) {
        Type *type = types[i];
        Operator *op = (Operator *) calloc(1, sizeof(Operator));
        init_operator(op, &op_str, types[TYPE_EMPTY], type, type);
        op->generate_op = neg_float;

        type->operators.insert(&type->operators, &op_str, &op);
    }
}

static void create_float_conversions() {

    StringSlice op_str;
    init_string(&op_str, as_name, strlen(as_name));

    for (usize i = TYPE_F32; i <= TYPE_F64; ++i) {
        Type *from = types[i];

        for (usize j = TYPE_BOOL; j <= TYPE_F64; ++j) {
            Type *to = types[j];

            Operator *op = (Operator *) calloc(1, sizeof(Operator));
            init_operator(op, &op_str, from, types[TYPE_EMPTY], to);
            from->operators.insert(&from->operators, &op_str, &op);

            if (j <= TYPE_U64) {
                op->generate_op = from_float_to_u_int;
            } else if (j <= TYPE_I64) {
                op->generate_op = from_float_to_s_int;
            } else {
                op->generate_op = from_float_to_float;
            }
        }
    }
}

static void create_float_types(SymbolTable *st) {

    static char *type_name[] = {"f32", "f64"};

    usize types_count = sizeof(type_name) / sizeof(char *);
    StringSlice names[types_count];
    for (usize i = 0; i < types_count; ++i) {
        init_string(&names[i], type_name[i], strlen(type_name[i]));
    }

    for (usize i = TYPE_F32, j = 0; i <= TYPE_F64; ++i, ++j) {
        types[i] = (Type *) calloc(1, sizeof(Type));
        init_type(types[i], i, &names[j]);
        add_type(st, types[i]);
    }
}

/// -------------------------------------------------
/// PRIVATE FUNCTIONS
/// -------------------------------------------------

static void init_scope(Scope *s, Scope *p) {
    s->functions = new_multi_map(FunctionRef);
    s->types = new_map(TypeRef);
    s->variables = new_multi_map(VariableRef);
    s->block = NULL;

    s->parent = p;
    s->level = (p != NULL ? p->level + 1 : 0);
}

static void free_scope(Scope *s) {
    s->functions.free(&s->functions);
    s->types.free(&s->types);
    s->variables.free(&s->variables);
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

void init_symbol_table(SymbolTable *st) {

    st->current = (Scope *) calloc(1, sizeof(Scope));
    init_scope(st->current, NULL);

    // create types
    create_no_return_type(st);
    create_empty_type(st);
    create_bool_type(st);
    create_int_types(st);
    create_float_types(st);

    // create operators for types
    create_bool_binary_ops();
    create_bool_unary_ops();
    create_bool_conversions();

    create_int_binary_ops();
    create_int_unary_ops();
    create_int_conversions();

    create_float_binary_ops();
    create_float_unary_ops();
    create_float_conversions();
}

void free_symbol_table(SymbolTable *st) {

    if (st == NULL || st->current == NULL) { return; }

    while (st->current != NULL) {
        Scope *p = st->current->parent;
        free_scope(st->current);
        free(st->current);
        st->current = p;
    }

    for (usize i = 0; i < TYPES_COUNT; ++i) {
        free_type(types[i]);
        free(types[i]);
    }
}

usize get_scope_level(const SymbolTable *st) {

    if (st->current == NULL) {
        return 0;
    } else {
        return st->current->level;
    }
}

void enter_scope(SymbolTable *st) {

    Scope *p = st->current;
    st->current = (Scope *) calloc(1, sizeof(Scope));
    init_scope(st->current, p);
}

bool leave_scope(SymbolTable *st) {

    if (st == NULL || st->current == NULL || st->current->level == 0) {
        return false;
    }

    Scope *p = st->current->parent;
    free_scope(st->current);
    free(st->current);
    st->current = p;

    return true;
}

// -------------------------------------------------

bool add_function(SymbolTable *st, Function *f_ins) {

    if (st->current == NULL) { return false; }

    Scope *global = st->current;
    while (global->level > 0) {
        global = global->parent;
    }

    Vector(FunctionRef) *functions = global->functions.get(&global->functions, &f_ins->name);
    if (functions == NULL || functions->length == 0) {
        // there is no function with the same name, so we can add it
        global->functions.insert(&global->functions, &f_ins->name, &f_ins);
        return true;
    } else {

        bool can_insert = true;
        for (usize i = 0; i < functions->length; ++i) {
            Function *fun = functions->entries[i];
            bool is_different = true;
            // if both the functions have NULL types, it means that we could not correctly parse their type,
            // so we assume that thy are different
            if (fun->ret_type == f_ins->ret_type && (fun->ret_type != NULL || f_ins->ret_type != NULL)
                && fun->params.length == f_ins->params.length) {

                is_different = false;
                for (usize j = 0; j < fun->params.length; ++j) {
                    FnParam *f_param = f_ins->params.entries[j];
                    FnParam *fun_param = fun->params.entries[j];
                    if (f_param->type != fun_param->type || (f_param->type == NULL && fun_param->type == NULL)) {
                        // at least one parameter is different, so the two functions types are different
                        // if both parameter types are set to NULL, it means that their type is wrong,
                        // so we assume that their types are different
                        is_different = true;
                        break;
                    }
                }
            }

            if (!is_different) {
                can_insert = false;
                break;
            }
        }

        if (can_insert) {
            global->functions.insert(&global->functions, &f_ins->name, &f_ins);
        }
        return can_insert;
    }
}

bool add_block(SymbolTable *st, Block *b) {

    if (st->current == NULL) {
        return false;
    } else {
        st->current->block = b;
        b->level = st->current->level;
        return true;
    }
}

bool add_variable(SymbolTable *st, Variable *v, bool redef) {

    if (st->current == NULL) {
        return false;
    } else if (redef){
        st->current->variables.insert(&st->current->variables, get_var_name(v), &v);
        return true;
    } else {
        Vector(VariableRef) *vars = st->current->variables.get(&st->current->variables, get_var_name(v));
        if (vars == NULL || vars->length == 0) {
            st->current->variables.insert(&st->current->variables, get_var_name(v), &v);
            return true;
        } else {
            return false;
        }
    }
}

bool add_type(SymbolTable *st, Type *type) {

    if (st->current == NULL) {
        return false;
    } else {
        return st->current->types.insert(&st->current->types, &type->name, &type);
    }
}

// -------------------------------------------------

Type *search_type(const SymbolTable *st, const StringSlice *name) {

    Scope *cur = st->current;
    while (cur != NULL) {
        Type **t = cur->types.get(&cur->types, name);
        if (t != NULL) {
            return *t;
        } else {
            cur = cur->parent;
        }
    }

    return NULL;
}

Type *get_builtin_type(TypeKind tk) {
    if (tk >= TYPE_NO_RETURN && tk < TYPES_COUNT) {
        return types[tk];
    } else {
        return NULL;
    }
}

Variable **get_variable(const SymbolTable *st, const StringSlice *name) {

    Scope *cur = st->current;
    while (cur != NULL) {
        Vector(VariableRef) *vars = cur->variables.get(&cur->variables, name);
        if (vars != NULL && vars->length > 0) {
            return vars->top(vars);
        } else {
            cur = cur->parent;
        }
    }

    return NULL;
}

Vector(FunctionRef) *get_functions(const SymbolTable *st, const StringSlice *name) {

    Vector(FunctionRef) *fun = NULL;
    Scope *cur = st->current;
    while (cur != NULL && cur->level > 0) {
        cur = cur->parent;
    }

    if (cur != NULL) {
        fun = cur->functions.get(&cur->functions, name);
    }

    return fun;
}

Block *get_internal_block(const SymbolTable *st, const StringSlice *name) {

    Scope *cur = st->current;
    while (cur != NULL) {
        Block *b = cur->block;
        if (b == NULL) {
            cur = cur->parent;
        } else if (name == NULL || str_equals(&b->name, name) || str_equals(&b->alt_name, name)) {
            return b;
        } else {
            cur = cur->parent;
        }
    }

    return NULL;
}

Block *get_external_block(const SymbolTable *st, const StringSlice *name) {

    Block *block = NULL;
    Scope *cur = st->current;
    while (cur != NULL) {
        Block *b = cur->block;

        if (b != NULL && (name == NULL || str_equals(&b->name, name) || str_equals(&b->alt_name, name))) {
            block = b;
        }
        cur = cur->parent;
    }
    return block;
}