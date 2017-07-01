#pragma once

#ifndef HEADER_GUARD_DIAGNOSTICS_H
#define HEADER_GUARD_DIAGNOSTICS_H

#ifdef __cplusplus
extern "C" {
#endif

void print_error(const char* message, ...);
void print_warning(const char* message, ...);

struct fposition;
void print_error_pos(const struct fposition* fpos,
                     const char* message, ...);
void print_warning_pos(const struct fposition* fpos,
                       const char* message, ...);

#ifdef __cplusplus
}
#endif

#endif
