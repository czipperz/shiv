#include "parse.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "../cutil/rpmalloc.h"
#include "../cutil/vec.h"
#include "diagnostics.h"
#include "lex.h"

static void
destroy_expression(expression* expression) {
    assert(expression);
    switch (expression->type) {
    case expression_plus:
        assert(expression->data.binary.first);
        destroy_expression(expression->data.binary.first);
        rpfree(expression->data.binary.first);

        assert(expression->data.binary.second);
        destroy_expression(expression->data.binary.second);
        rpfree(expression->data.binary.second);
        break;
    }
}

static void
destroy_type_expression(type_expression* type) {
    switch (type->type) {
    case type_pointer:

    case type_const_pointer:
        destroy_type_expression(type->data.next_type);
        rpfree(type->data.next_type);
        break;
    case type_name:
    case type_const_name:
        str_destroy(&type->data.name);
        break;
    }
}

static void destroy_statements(statements*);

static void
destroy_defining_type_expression(defining_type_expression* dtype) {
    switch (dtype->type) {
    case dtype_pointer:
    case dtype_const_pointer:
        destroy_type_expression(dtype->data.next_type);
        break;
    case dtype_name:
    case dtype_const_name:
        str_destroy(&dtype->data.name);
        break;
    case dtype_fun_def:
        str_destroy(&dtype->data.fun_def.name);
        destroy_var_decls(&dtype->data.fun_def.params);
        destroy_type_expression(&dtype->data.fun_def.return_type);
        destroy_statements(&dtype->data.fun_def.stmts);
        break;
    }
}

static void
destroy_var_decl(var_decl* vd) {
    str_destroy(&vd->name);
    destroy_defining_type_expression(&vd->type);
}

static void
destroy_statements(statements* statements) {
    statement* stmt;
    statement* last;
    assert(statements);
    if (!statements->stmts) { return; }
    last = statements->stmts + statements->len;
    for (stmt = statements->stmts; stmt != last; ++stmt) {
        switch (stmt->type) {
        case statement_if:
            destroy_expression(&stmt->data.s_if.cond);
            destroy_statements(&stmt->data.s_if.truebranch);
            destroy_statements(&stmt->data.s_if.falsebranch);
            break;
        case statement_expression:
            destroy_expression(&stmt->data.s_expression);
            break;
        case statement_var_decl:
            destroy_var_decl(&stmt->data.s_var_decl);
            break;
        }
    }
    rpfree(statements->stmts);
}

void
destroy_var_decls(vec_var_decl* vds) {
    size_t i;
    for (i = 0; i != vds->len; ++i) {
        destroy_var_decl(&vds->vars[i]);
    }
    rpfree(vds->vars);
}

static const char*
_a_or_an(const char* str) {
    assert(str);
    return (str[0] == 'a' || str[0] == 'e' || str[0] == 'i' ||
            str[0] == 'o' || str[0] == 'u') ? "an" : "a";
}
static void
erroreof(fposition* fpos, const char* expected) {
    print_error_pos(fpos, "Unexpected end of file, expected %s %s",
                    _a_or_an(expected),
                    expected);
}
static void
errortoken(fposition* fpos, const char* expected) {
    print_error_pos(fpos, "Expected %s %s", _a_or_an(expected),
                    expected);
}

static int
assertattoken(token* tk, token* last, int token_type,
              const char* expected) {
    if (tk == last) {
        erroreof(&tk[-1].fpos, expected);
        return -1;
    }
    if (tk->type != token_type) {
        errortoken(&tk->fpos, expected);
        return -1;
    }
    return 0;
}

static int
parse_word(str* str, token** tk, token* last) {
    if (assertattoken(*tk, last, token_word, "word")) {
        return -1;
    }
    if (str_push_str(str, &(*tk)->data.s)) {
        return -1;
    }
    ++*tk;
    return 0;
}

static int
parse_namespaced_word(str* str, token** tk, token* last) {
top:
    if (parse_word(str, tk, last)) {
        return -1;
    }
    if (*tk == last || (*tk)->type != token_namespace) {
        return 0;
    }
    str_push_s(str, "::");
    ++*tk;
    goto top;
}

static int
parse_colon(token** tk, token* last) {
    if (assertattoken(*tk, last, token_colon, "colon")) {
        return -1;
    }
    ++*tk;
    return 0;
}

static int
parse_params(token** tk, token* last) {
top:
    if (*tk == last) {
        erroreof(&tk[-1]->fpos, "list of parameters, a closing "
                                "parenthesis, and then a function "
                                "body.");
        return -1;
    }
    if ((*tk)->type == token_close_paren) {
        return 0;
    }
nextparam:
    {
        str paramname = STR_INIT;
        str type = STR_INIT;
        if (parse_namespaced_word(&paramname, tk, last)) {
            str_destroy(&paramname);
            return -1;
        }
        if (assertattoken(*tk, last, token_colon, "colon")) {
            str_destroy(&paramname);
            return -1;
        }
        ++*tk;
        if (parse_namespaced_word(&type, tk, last)) {
            str_destroy(&type);
            str_destroy(&paramname);
            return -1;
        }
        printf("Parameter %s of %s.\n", str_cbegin(&paramname),
               str_cbegin(&type));
        str_destroy(&type);
        str_destroy(&paramname);
        if (*tk == last || (*tk)->type == token_close_paren) {
            goto top;
        }
        if ((*tk)->type != token_comma) {
            errortoken(&(*tk)->fpos, "comma");
            return -1;
        }
        ++*tk;
        goto nextparam;
    }
}

static int
is_x_wider_than(token_type x, expression_type y) {
    assert(x >= token_comma);
    if (y < expression_comma) {
        return 1;
    }
    if (x < y) {
        return 1;
    }
    if (y > x) {
        return 0;
    }
    switch (x) {
    case token_comma:
    case token_minus:
    case token_plus:
        /* left associating:
         * a - b - c == (a - b) - c */
        return 1;
    case token_assign:
        /* right associating:
         * a = b = c == a = (b = c) */
        return 0;
    default:
        /* shouldn't happen */
        abort();
    }
}

static int
parse_expression(token** tk, token* last, token_type escape_type,
                 expression** expr);

static int
parse_sub_expression(token** tk, token* last, token_type escape_type,
                     expression** expr) {
    if (*tk == last) {
        goto last;
    }
    switch ((*tk)->type) {
    case token_word:
        (*expr)->type = expression_name;
        if (str_copy_str(&(*expr)->data.name, &(*tk)->data.s)) {
            return -1;
        }
        break;
    case token_open_paren:
        {
            expression* embedded = rpmalloc(sizeof(expression));
            if (!embedded) {
                return -1;
            }
            ++*tk;
            if (parse_expression(tk, last, token_close_paren,
                                 &embedded)) {
                destroy_expression(embedded);
                rpfree(embedded);
                return -1;
            }
            destroy_expression(embedded);
            rpfree(embedded);
            assert((*tk)->type == token_close_paren);
            ++*tk;
        }
        break;
    case token_close_paren:
    case token_close_curly:
    case token_semicolon:
        if (escape_type == (*tk)->type) {
            return 0;
        }
        // fall through
    case token_return:
        print_error_pos(&(*tk)->fpos, "Unexpected token");
        return -1;
    }
    ++*tk;
    if (*tk == last) {
        const char* mes;
last:
        switch (escape_type) {
        case token_close_paren:
            mes = "closing parenthesis";
            break;
        case token_close_curly:
            mes = "closing curly";
            break;
        case token_semicolon:
            mes = "semicolon";
            break;
        default:
            abort();
        }
        erroreof(&(*tk)[-1].fpos, mes);
        return -1;
    }
    return 0;
}

static int
parse_expression(token** tk, token* last, token_type escape_type,
                 expression** expr) {
    if (parse_sub_expression(tk, last, escape_type, expr)) {
        return -1;
    }
    /* binary operator */
    while ((*tk)->type >= token_comma) {
        /* left associating (<=):
         * a - b - c == (a - b) - c */
        /* right associating (<):
         * a = b = c == a = (b = c) */
        expression** iter = expr;
        token_type type = (*tk)->type;
        ++*tk;
        if (*tk == last) {
            erroreof(&(*tk)[-1].fpos, "expression");
            return -1;
        }
        while (!is_x_wider_than(type, (*iter)->type)) {
            iter = &(*iter)->data.binary.second;
        }
        {
            expression* bin = rpmalloc(sizeof(expression));
            if (!bin) {
                return -1;
            }
            bin->data.binary.second = rpmalloc(sizeof(expression));
            if (!bin->data.binary.second) {
                rpfree(bin);
                return -1;
            }
            bin->type = (expression_type) type;
            bin->data.binary.first = *iter;
            if (parse_sub_expression(tk, last, escape_type,
                                     &bin->data.binary.second)) {
                rpfree(bin->data.binary.second);
                rpfree(bin);
                destroy_expression(*expr);
                return -1;
            }
            /* wait to publish so that cleanup is easier */
            *iter = bin;
        }
    }
    return 0;
}

static int
parse_statements(token** tk, token* last) {
    for (; *tk != last; ++*tk) {
        switch ((*tk)->type) {
        case token_close_curly:
            /* go past close curly */
            ++*tk;
            return 0;
        case token_open_curly:
            ++*tk;
            parse_statements(tk, last);
            if (assertattoken(*tk, last, token_close_curly,
                              "closing curly")) {
                return -1;
            }
            break;
        case token_return:
            ++*tk;
            {
                expression* expr = rpmalloc(sizeof(expression));
                if (!expr) {
                    return -1;
                }
                if (parse_expression(tk, last, token_semicolon,
                                     &expr)) {
                    destroy_expression(expr);
                    rpfree(expr);
                    return -1;
                }
                assert(*tk - 1 < last);
                assert((*tk)[-1].type == token_semicolon);
                destroy_expression(expr);
                rpfree(expr);
            }
            if (assertattoken(*tk, last, token_semicolon,
                              "semicolon")) {
                return -1;
            }
        }
    }
    erroreof(&(*tk)[-1].fpos, "closing curly");
    return -1;
}

static int
parse_fun(const str* name, token** tk, token* last) {
    /* after fun word is '\($param*\) (\-\> $type)? \{ $statement* \}' */
    if (assertattoken(*tk, last, token_open_paren,
                      "opening parenthesis")) {
        return -1;
    }
    ++*tk;
    if (parse_params(tk, last)) {
        return -1;
    }
    if (assertattoken(*tk, last, token_close_paren,
                      "closing parenthesis")) {
        return -1;
    }
    ++*tk;
    if (*tk == last) {
        erroreof(&(*tk)[-1].fpos, "function body");
        return -1;
    }
    if ((*tk)->type == token_right_arrow) {
        str return_type = STR_INIT;
        ++*tk;
        if (*tk == last) {
            erroreof(&(*tk)[-1].fpos,
                     "type to point to then the function body");
            return -1;
        }
        if (parse_namespaced_word(&return_type, tk, last)) {
            return -1;
        }
        printf("Return type: %s\n", str_cbegin(&return_type));
        str_destroy(&return_type);
    }
    if (assertattoken(*tk, last, token_open_curly, "opening curly")) {
        return -1;
    }
    ++*tk;
    if (parse_statements(tk, last)) {
        return -1;
    }
    return 0;
}

static int
parse_struct(const str* name, token** tk, token* last) {
    return 0;
}

int
parse(const vec_token* tokens, vec_var_decl* statements) {
    token* tk;
    token* last;
    assert(tokens);
    assert(statements);
    last = tokens->tokens + tokens->len;
    for (tk = tokens->tokens; tk != last; ++tk) {
        str name = STR_INIT;
        if (parse_namespaced_word(&name, &tk, last)) {
            str_destroy(&name);
            return -1;
        }
        if (parse_colon(&tk, last)) {
            str_destroy(&name);
            return -1;
        }
        if (tk == last) {
            erroreof(&tk[-1].fpos, "type");
            str_destroy(&name);
            return -1;
        }
        if (tk->type != token_assign) {
            fputs("UNSUPPORTED RIGHT NOW\n", stderr);
            return -1;
        }
        ++tk;
        if (tk == last) {
            erroreof(&tk[-1].fpos, "value, a function definition, or "
                                   "a type definition.");
        }
        /* we are defining an untyped variable or a named type. */
        if (tk->type == token_fun) {
            printf("Defining fun %s\n", str_cbegin(&name));
            ++tk;
            if (parse_fun(&name, &tk, last)) {
                str_destroy(&name);
                return -1;
            }
            /* counter iteration */
            --tk;
            continue;
        } else if (tk->type == token_struct) {
            ++tk;
            if (parse_struct(&name, &tk, last)) {
                str_destroy(&name);
                return -1;
            }
        } else if (tk->type == token_word) {
            str type = STR_INIT;
            ++tk;
            if (parse_namespaced_word(&type, &tk, last)) {
                str_destroy(&type);
                str_destroy(&name);
                return -1;
            }
            if (tk == last) {
                erroreof(&tk[-1].fpos, "semicolon");
                str_destroy(&type);
                str_destroy(&name);
                return -1;
            }
            if (tk->type != token_semicolon) {
                print_error_pos(&tk->fpos, "Expected a semicolon");
                str_destroy(&type);
                str_destroy(&name);
                return -1;
            }
            /* NOT DONE */
            assert(0);
            str_destroy(&type);
        }
        str_destroy(&name);
    }
    return 0;
}
