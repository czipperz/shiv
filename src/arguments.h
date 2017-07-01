#pragma once

#ifndef HEADER_GUARD_ARGUMENTS_H
#define HEADER_GUARD_ARGUMENTS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct arguments {
    const char* file;
    int dump_tokens : 1;
    int dump_syntax_tree : 1;
};
typedef struct arguments arguments;

int parse_arguments(arguments*, size_t argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif
