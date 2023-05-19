#ifndef fjx_fiber_internal_fiber_h
#define fjx_fiber_internal_fiber_h

#include "./utils.h"
#include "../fiber.h"

struct fjx_fiber__ {
    void *stack_top;
    fjx_list link;
};

typedef void (*entrance_func_t)(void *);
typedef void (*deferred_func_t)(void *);

DLL_LOCAL void fiber_switch(fjx_fiber *);
DLL_LOCAL void fiber_add_deferred(fjx_fiber *, deferred_func_t, void *);
DLL_LOCAL void *fiber_spawn_stack_top(fjx_fiber_scheduler *, entrance_func_t, void *);

#endif
