#pragma once

#ifndef HEADER_GUARD_LEX_H
#define HEADER_GUARD_LEX_H

#include "../cutil/str.h"
#include "fposition.h"

#ifdef __cplusplus
extern "C" {
#endif

struct token {
    enum token_type {
        token_close_curly,
        token_close_paren,
        token_colon,
        token_fun,
        token_namespace,
        token_open_curly,
        token_open_paren,
        token_return,
        token_right_arrow,
        token_semicolon,
        token_struct,
        token_word,

        /* from widest to tightest: */
        token_comma = 1000,
        token_assign,
        /* a + b - c = a + (b - c) */
        token_minus,
        token_plus,
    } type;
    union {
        str s;
    } data;
    fposition fpos;
};
typedef struct token token;
typedef enum token_type token_type;

struct vec_token {
    token* tokens;
    size_t len, cap;
};
typedef struct vec_token vec_token;

void destroy_tokens(vec_token* tokens);

struct fiter;
int lex(struct fiter*, vec_token* tokens);

#ifdef __cplusplus
}
#endif

#endif
