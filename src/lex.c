#include "lex.h"
#include "fiter.h"
#include "diagnostics.h"
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include "../cutil/str.h"
#include "../cutil/vec.h"
#include "../cutil/rpmalloc.h"
#include <string.h>

int lex(fiter* fiter, vec_token* tokens) {
    int ret = 0;
    char c;
    assert(fiter);
    assert(fiter->file);

    while (1) {
        token tk;
        if (!(c = fiter_next(fiter))) {
            break;
        }
top:
        tk.fpos = fiter->fpos;
        if (isspace(c)) {
        } else if (isalpha(c)) {
            str s = STR_INIT;
            if (str_push(&s, c)) {
                str_destroy(&s);
                return -1;
            }
            while ((c = fiter_next(fiter)) && (isalnum(c) || c == '_')) {
                if (str_push(&s, c)) {
                    str_destroy(&s);
                    return -1;
                }
            }
            if (strcmp(str_cbegin(&s), "fun") == 0) {
                tk.type = token_fun;
                str_destroy(&s);
            } else if (strcmp(str_cbegin(&s), "struct") == 0) {
                tk.type = token_struct;
                str_destroy(&s);
            } else if (strcmp(str_cbegin(&s), "return") == 0) {
                tk.type = token_return;
                str_destroy(&s);
            } else {
                tk.type = token_word;
                tk.data.s = s;
            }
            if (vec_push(tokens, sizeof(tk), &tk)) {
                /* destroying the str is safe to do twice */
                str_destroy(&s);
                return -1;
            }
            goto top;
        } else if (c == '{') {
            tk.type = token_open_curly;
            if (vec_push(tokens, sizeof(tk), &tk)) {
                return -1;
            }
        } else if (c == '}') {
            tk.type = token_close_curly;
            if (vec_push(tokens, sizeof(tk), &tk)) {
                return -1;
            }
        } else if (c == '(') {
            tk.type = token_open_paren;
            if (vec_push(tokens, sizeof(tk), &tk)) {
                return -1;
            }
        } else if (c == ')') {
            tk.type = token_close_paren;
            if (vec_push(tokens, sizeof(tk), &tk)) {
                return -1;
            }
        } else if (c == ';') {
            tk.type = token_semicolon;
            if (vec_push(tokens, sizeof(tk), &tk)) {
                return -1;
            }
        } else if (c == '+') {
            tk.type = token_plus;
            if (vec_push(tokens, sizeof(tk), &tk)) {
                return -1;
            }
        } else if (c == '-') {
            if ((c = fiter_next(fiter)) == '>') {
                tk.type = token_right_arrow;
                if (vec_push(tokens, sizeof(tk), &tk)) {
                    return -1;
                }
            } else {
                tk.type = token_minus;
                if (vec_push(tokens, sizeof(tk), &tk)) {
                    return -1;
                }
                goto top;
            }
        } else if (c == ',') {
            tk.type = token_comma;
            if (vec_push(tokens, sizeof(tk), &tk)) {
                return -1;
            }
        } else if (c == ':') {
            if ((c = fiter_next(fiter)) == ':') {
                tk.type = token_namespace;
                if (vec_push(tokens, sizeof(tk), &tk)) {
                    return -1;
                }
            } else {
                tk.type = token_colon;
                if (vec_push(tokens, sizeof(tk), &tk)) {
                    return -1;
                }
                goto top;
            }
        } else if (c == '=') {
            tk.type = token_assign;
            if (vec_push(tokens, sizeof(tk), &tk)) {
                return -1;
            }
        } else {
            print_error_pos(&fiter->fpos,
                            "Lexing error on seeing %c", (int)c);
            ret = -1;
        }
    }

    return ret;
}

void destroy_tokens(vec_token* tokens) {
    token* last;
    token* token;
    assert(tokens);
    assert(tokens->tokens);
    last = tokens->tokens + tokens->len;
    for (token = tokens->tokens; token != last; ++token) {
        if (token->type == token_word) {
            str_destroy(&token->data.s);
        }
    }
    rpfree(tokens->tokens);
}

#ifdef TEST_MODE
#define LEX_TEST(name)                                               \
    do {                                                             \
        vec_token tokens = VEC_INIT;                                 \
        size_t i;                                                    \
        fiter iter;                                                  \
        iter.file = (FILE*)name##_file;                              \
        iter.index = 0;                                              \
        iter.fpos.fname = #name;                                     \
        iter.fpos.line = 1;                                          \
        iter.fpos.column = 1;                                        \
        ASSERT(lex(&iter, &tokens) == 0, cleanup);                   \
        for (i = 0; i != tokens.len; ++i) {                          \
            const token* tk = &tokens.tokens[i];                     \
            const token* expected = &name##_tokens[i];               \
            ASSERT(tk->type == expected->type, cleanup);             \
            if (tk->type == token_word) {                            \
                ASSERT(strcmp(str_cbegin(&tk->data.s),               \
                              str_cbegin(&expected->data.s)) == 0,   \
                       cleanup);                                     \
            }                                                        \
        }                                                            \
    cleanup:                                                         \
        destroy_tokens(&tokens);                                     \
    } while (0)

#include "../cutil/test.h"

static const char* test_lex_1_file[] = {"a::", 0};
static token test_lex_1_tokens[] = {
    {token_word, STR_INIT}, {token_namespace}
};
TEST(test_lex_1) {
    ASSERT(str_push_s(&test_lex_1_tokens[0].data.s, "a") == 0, stop);
    LEX_TEST(test_lex_1);
stop:
    str_destroy(&test_lex_1_tokens[0].data.s);
}
END_TEST

static const char* test_lex_2_file[] =
    {"add2 := fun (a : std::i32, b : std::i32) -> std::i32 {",
     "return a + b; }", 0};
static token test_lex_2_tokens[] =
    {/* add2 := fun ( */ {token_word, STR_INIT},
     {token_colon},
     {token_assign},
     {token_fun},
     {token_open_paren},
     /* a : std::int32 */ {token_word, STR_INIT},
     {token_colon},
     {token_word, STR_INIT},
     {token_namespace},
     {token_word, STR_INIT},
     {token_comma},
     /* b : std::int32 */ {token_word, STR_INIT},
     {token_colon},
     {token_word, STR_INIT},
     {token_namespace},
     {token_word, STR_INIT},
     /* ) -> */ {token_close_paren},
     {token_right_arrow},
     /* std::int32 */ {token_word, STR_INIT},
     {token_namespace},
     {token_word, STR_INIT},
     /* { */ {token_open_curly},
     {token_return},
     {token_word, STR_INIT},
     {token_plus},
     {token_word, STR_INIT},
     {token_semicolon},
     /* } */ {token_close_curly}};
TEST(test_lex_2) {
    ASSERT(str_push_s(&test_lex_2_tokens[0].data.s, "add2") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[5].data.s, "a") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[7].data.s, "std") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[9].data.s, "i32") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[11].data.s, "b") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[13].data.s, "std") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[15].data.s, "i32") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[18].data.s, "std") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[20].data.s, "i32") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[23].data.s, "a") == 0, stop);
    ASSERT(str_push_s(&test_lex_2_tokens[25].data.s, "b") == 0, stop);
    LEX_TEST(test_lex_2);
stop:
    str_destroy(&test_lex_2_tokens[0].data.s);
    str_destroy(&test_lex_2_tokens[5].data.s);
    str_destroy(&test_lex_2_tokens[7].data.s);
    str_destroy(&test_lex_2_tokens[9].data.s);
    str_destroy(&test_lex_2_tokens[11].data.s);
    str_destroy(&test_lex_2_tokens[13].data.s);
    str_destroy(&test_lex_2_tokens[15].data.s);
    str_destroy(&test_lex_2_tokens[18].data.s);
    str_destroy(&test_lex_2_tokens[20].data.s);
    str_destroy(&test_lex_2_tokens[23].data.s);
    str_destroy(&test_lex_2_tokens[25].data.s);
}
END_TEST

void test_lex(void) {
    RUN(test_lex_1);
    RUN(test_lex_2);
}

#endif
