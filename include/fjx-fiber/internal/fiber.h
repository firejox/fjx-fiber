#ifndef fjx_fiber_internal_fiber_h
#define fjx_fiber_internal_fiber_h

#include "./utils.h"
#include "../fiber.h"

struct __fjx_fiber {
    void *stack_top;
    fjx_list link;
};

typedef void (*entrance_func_t)(void *);
typedef void (*cleanup_func_t)(void *);

DLL_LOCAL void fiber_switch(fjx_fiber *);
DLL_LOCAL void fiber_insert_cleanup(fjx_fiber *, cleanup_func_t, void *);
DLL_LOCAL void *fiber_spawn_stack_top(fjx_fiber_scheduler *, entrance_func_t, void *);

#endif
