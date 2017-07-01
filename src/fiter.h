#pragma once

#ifndef HEADER_GUARD_FITER_H
#define HEADER_GUARD_FITER_H

#include <stdio.h>
#include "fposition.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fiter {
    FILE* file;
    char buffer[1024];
    size_t index;
    fposition fpos;
};
typedef struct fiter fiter;

char fiter_next(fiter*);

#ifdef __cplusplus
}
#endif

#endif
