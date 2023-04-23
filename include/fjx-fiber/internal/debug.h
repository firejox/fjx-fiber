#ifndef fjx_fiber_internal_debug_h
#define fjx_fiber_internal_debug_h

#ifndef NDEBUG
#   include <stdio.h>
#   include <stdlib.h>
#   include <string.h>
#   include <errno.h>
#   include <assert.h>

#   define S(str) #str
#   define SS(str) S(str)

#   define ERROR_ABORT(cond, msg) do { \
        if (cond) { \
            fprintf( \
                stderr, \
                __FILE__ ":" SS(__LINE__) ": %s: " msg ": %s\n", \
                __func__, \
                strerror(errno)); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)
#else
#   define ERROR_ABORT(cond, msg) ((void)(cond))
#   define assert(c) ((void)(c))
#endif

#define ERROR_ABORT_IF(s, expect, msg) ERROR_ABORT((s) == (expect), msg)
#define ERROR_ABORT_UNLESS(s, expect, msg) ERROR_ABORT((s) != (expect), msg)

#endif
