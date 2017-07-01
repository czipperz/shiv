#include "arguments.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "diagnostics.h"

int parse_arguments(arguments* args, size_t argc, char** argv) {
    size_t argi;
    assert(args);

    /* setup default values */
    args->file = 0;
    args->dump_tokens = 0;
    args->dump_syntax_tree = 0;

    for (argi = 0; argi != argc; ++argi) {
        char* arg = argv[argi];
        /* parse options */
        if (strcmp(arg, "-compiler-dump=tokens") == 0) {
            args->dump_tokens = 1;
            continue;
        }
        if (strcmp(arg, "-compiler-dump=syntax") == 0) {
            args->dump_syntax_tree = 1;
            continue;
        }
        if (arg[0] == '-') {
            print_error("Unknown option %s", arg);
            return -1;
        }
        /* end parse options */

        if (args->file) {
            print_error("Already specified file to compile");
            return -1;
        }
        args->file = arg;
    }

    if (!args->file) {
        print_error("File not specified to compile.");
        return -1;
    }
    return 0;
}
