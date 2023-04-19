#define _DEFAULT_SOURCE
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include "fjx-fiber/internal/fiber-memory.h"
#include "fjx-fiber/internal/debug.h"

enum {
    STACK_SIZE = (size_t) 8 * 1024
};

static inline void *allocate_stack(void) {
    void *addr = mmap(
        NULL,
        STACK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK | MAP_GROWSDOWN,
        -1,
        0);

    ERROR_ABORT_IF(addr, MAP_FAILED, "allocate stack failed");

    return addr;
}

fjx_fiber_memory *fiber_memory_create(void) {
    fjx_fiber_memory *mem = calloc(1, sizeof(fjx_fiber_memory));

    ERROR_ABORT_IF(mem, NULL, "allocate fiber memory failed");

    mem->addr = allocate_stack();
    mem->size = STACK_SIZE;
    fjx_avl_node_init(&mem->link);
    return mem;
}

void fiber_memory_destroy(fjx_fiber_memory *mem) {
    munmap(mem->addr, mem->size);
    free(mem);
}
