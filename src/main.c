#include "../cutil/rpmalloc.h"
#include <stdio.h>

#ifndef TEST_MODE

#include <string.h>
#include "../cutil/vec.h"
#include "../cutil/str.h"
#include "../cutil/stack_trace.h"
#include "arguments.h"
#include "diagnostics.h"
#include "fiter.h"
#include "lex.h"
#include "parse.h"

static void dump_tokens(vec_token *tokens) {
    token* token;
    for (token = tokens->tokens; token != tokens->tokens + tokens->len;
         ++token) {
        switch (token->type) {
        case token_word:
            print_warning_pos(&token->fpos, "%s",
                              str_cbegin(&token->data.s));
            break;
        case token_fun:
            print_warning_pos(&token->fpos, "fun");
            break;
        case token_struct:
            print_warning_pos(&token->fpos, "struct");
            break;
        case token_return:
            print_warning_pos(&token->fpos, "return");
            break;
        case token_namespace:
            print_warning_pos(&token->fpos, "::");
            break;
        case token_colon:
            print_warning_pos(&token->fpos, ":");
            break;
        case token_plus:
            print_warning_pos(&token->fpos, "+");
            break;
        case token_minus:
            print_warning_pos(&token->fpos, "-");
            break;
        case token_semicolon:
            print_warning_pos(&token->fpos, ";");
            break;
        case token_open_curly:
            print_warning_pos(&token->fpos, "{");
            break;
        case token_close_curly:
            print_warning_pos(&token->fpos, "}");
            break;
        case token_open_paren:
            print_warning_pos(&token->fpos, "(");
            break;
        case token_close_paren:
            print_warning_pos(&token->fpos, ")");
            break;
        case token_comma:
            print_warning_pos(&token->fpos, ",");
            break;
        case token_right_arrow:
            print_warning_pos(&token->fpos, "->");
            break;
        case token_assign:
            print_warning_pos(&token->fpos, "=");
            break;
        }
    }
}

int main(int argc, char** argv) {
    fiter fiter;
    arguments args;
    if (rpmalloc_initialize()) {
        return 1;
    }

    --argc;
    ++argv;

    if (parse_arguments(&args, argc, argv)) {
        rpmalloc_finalize();
        return 1;
    }

    if (!argc) {
        print_error("Argument not provided");
        return 1;
    }
    fiter.file = fopen(args.file, "r");
    if (!fiter.file) {
        print_error("Cannot open file: %s", args.file);
        return 1;
    }
    memset(fiter.buffer, 0, sizeof(fiter.buffer));
    fiter.index = 0;
    fiter.fpos.fname = args.file;
    fiter.fpos.line = 1;
    fiter.fpos.column = 1;
    {
        size_t numread = fread(fiter.buffer, sizeof(char),
                               sizeof(fiter.buffer), fiter.file);
        if (numread < sizeof(fiter.buffer)) {
            fiter.buffer[numread] = '\0';
        }
    }

    {
        vec_var_decl toplevels = VEC_INIT;
        vec_token tokens = VEC_INIT;
        int res;

        res = lex(&fiter, &tokens);
        fclose(fiter.file);
        if (args.dump_tokens) {
            dump_tokens(&tokens);
        }
        if (res) {
            destroy_tokens(&tokens);
            STACK_TRACE_PRINT();
            return 1;
        }

        res = parse(&tokens, &toplevels);
        destroy_tokens(&tokens);
        if (res) {
            destroy_var_decls(&toplevels);
            STACK_TRACE_PRINT();
            return 1;
        }

        destroy_var_decls(&toplevels);
    }

    rpmalloc_finalize();

    return 0;
}

#else

int failures = 0;
int successes = 0;
int successes_assert = 0;

#define run(test)                                                    \
    do {                                                             \
        void test();                                                 \
        test();                                                      \
    } while (0)

int main(void) {
    rpmalloc_initialize();
    run(test_lex);
    printf("%d of %d succeeded.\n", successes, failures + successes);
    printf("%d assertions succeeded.\n", successes_assert);
    rpmalloc_finalize();
    return failures;
}

#endif
