#include "fiter.h"
#include <assert.h>

#ifndef TEST_MODE
char fiter_next(fiter* iter) {
    char c;
    assert(iter);
    assert(iter->file);
    assert(iter->index >= 0 && iter->index < sizeof(iter->buffer));
    c = iter->buffer[iter->index];
    if (c != '\0') {
        if (c == '\n') {
            ++iter->fpos.line;
            /* This is a workaround for the columns being always off
             * by one.  We are at EOL right now.  The *next*
             * character is BOL and has column 1 as expected. */
            iter->fpos.column = 0;
        } else {
            ++iter->fpos.column;
        }
        ++iter->index;
        if (iter->index >= sizeof(iter->buffer)) {
            size_t numread;
            iter->index = 0;
            numread = fread(iter->buffer, sizeof(char),
                            sizeof(iter->buffer), iter->file);
            if (numread < sizeof(iter->buffer)) {
                /* eof */
                iter->buffer[numread] = '\0';
            }
        }
    }
    return c;
}
#else
/* In testing mode, we define fiter_next very differently.  It will
 * reinterpret the `FILE*` as a `const char* const*` where both are
 * null terminated. */
char fiter_next(fiter* iter) {
    char c;
    assert(iter);
    assert(iter->file);
x:
    if (!*(const char* const*)iter->file) {
        return '\0';
    }
    if (!(*(const char* const*)iter->file)[iter->index]) {
        ++*(const char* const**)&iter->file;
        iter->index = 0;
        goto x;
    }
    c = (*(const char* const*)iter->file)[iter->index];
    if (c != '\0') {
        if (c == '\n') {
            ++iter->fpos.line;
            /* This is a workaround for the columns being always off
             * by one.  We are at EOL right now.  The *next*
             * character is BOL and has column 1 as expected. */
            iter->fpos.column = 0;
        } else {
            ++iter->fpos.column;
        }
        ++iter->index;
    }
    return c;
}


#endif
