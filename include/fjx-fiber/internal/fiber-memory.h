#ifndef fjx_fiber_internal_fiber_memory_h
#define fjx_fiber_internal_fiber_memory_h

#include "./utils.h"

struct fjx_fiber_memory__ {
    void *addr;
    size_t size;
    fjx_avl_node link;
};

DLL_LOCAL fjx_fiber_memory *fiber_memory_create(void);

DLL_LOCAL void fiber_memory_destroy(fjx_fiber_memory *);

#endif
