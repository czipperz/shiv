#pragma once

#ifndef HEADER_GUARD_PARSE_H
#define HEADER_GUARD_PARSE_H

#include <stddef.h>
#include "../cutil/str.h"

#ifdef __cplusplus
extern "C" {
#endif

struct expression {
    enum expression_type {
        expression_name,

        /* from widest to tightest: */
        expression_comma = 1000,
        expression_assign,
        expression_minus,
        expression_plus,
    } type;
    union {
        struct {
            struct expression* first;
            struct expression* second;
        } binary;
        str name;
    } data;
};
typedef enum expression_type expression_type;
typedef struct expression expression;

struct type_expression {
    enum {
        type_pointer = 2,
        type_const_pointer = 3,
        type_name = 4,
        type_const_name = 5,
        /* type_fun_ptr = 6, */
        /* type_const_fun_ptr = 7 */
    } type;
    union {
        struct type_expression* next_type;
        str name;
    } data;
};
typedef struct type_expression type_expression;

struct statements {
    struct statement* stmts;
    size_t len, cap;
};
typedef struct statements statements;

struct vec_var_decl {
    struct var_decl* vars;
    size_t len, cap;
};
typedef struct vec_var_decl vec_var_decl;

struct defining_type_expression {
    enum {
        dtype_pointer = 2,
        dtype_const_pointer = 3,
        dtype_name = 4,
        dtype_const_name = 5,
        /* dtype_fun_ptr = 6, */
        /* dtype_const_fun_ptr = 7, */
        dtype_fun_def = 8,
        /* dtype_struct_def = 9, */
    } type;
    union {
        struct type_expression* next_type;
        str name;
        struct {
            str name;
            vec_var_decl params;
            type_expression return_type;
            statements stmts;
        } fun_def;
        /* struct { */
        /* } struct_def; */
    } data;
};
typedef struct defining_type_expression defining_type_expression;

struct var_decl {
    str name;
    defining_type_expression type;
};
typedef struct var_decl var_decl;

struct statement {
    enum {
        statement_if,
        statement_expression,
        statement_var_decl,
    } type;
    union {
        struct {
            expression cond;
            statements truebranch;
            statements falsebranch;
        } s_if;
        expression s_expression;
        var_decl s_var_decl;
    } data;
};
typedef struct statement statement;

void destroy_var_decls(vec_var_decl*);

struct vec_token;
int parse(const struct vec_token*, vec_var_decl*);

#ifdef __cplusplus
}
#endif

#endif
