#include "diagnostics.h"
#include <stdarg.h>
#include <stdio.h>
#include "fposition.h"

#define WITH_ARG(argument, e)                                        \
    do {                                                             \
        va_list arg;                                                 \
        va_start(arg, argument);                                     \
        e;                                                           \
        va_end(arg);                                                 \
    } while (0)

static void
vprint_error(const char* message, va_list arg) {
    fputs("Error: ", stderr);
    vfprintf(stderr, message, arg);
    fputc('\n', stderr);
}
void
print_error(const char* message, ...) {
    WITH_ARG(message, vprint_error(message, arg));
}

static void
vprint_warning(const char* message, va_list arg) {
    fputs("Warning: ", stderr);
    vfprintf(stderr, message, arg);
    fputc('\n', stderr);
}
void
print_warning(const char* message, ...) {
    WITH_ARG(message, vprint_warning(message, arg));
}

static void
print_fpos(const fposition* fpos) {
    fprintf(stderr, "%s:%d:%d: ", fpos->fname, fpos->line,
            fpos->column);
}

void
print_error_pos(const fposition* fpos, const char* message, ...) {
    print_fpos(fpos);
    WITH_ARG(message, vprint_error(message, arg));
}

void
print_warning_pos(const fposition* fpos, const char* message, ...) {
    print_fpos(fpos);
    WITH_ARG(message, vprint_warning(message, arg));
}
