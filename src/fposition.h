#pragma once

#ifndef HEADER_GUARD_FPOSITION_H
#define HEADER_GUARD_FPOSITION_H

#ifdef __cplusplus
extern "C" {
#endif

struct fposition {
    const char* fname;
    int line, column;
};
typedef struct fposition fposition;

#ifdef __cplusplus
}
#endif

#endif
