#define _DEFAULT_SOURCE
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include "fjx-fiber/internal/fiber-memory.h"
#include "fjx-fiber/internal/debug.h"

enum {
    STACK_SIZE = (size_t) 8 * 1024
};

enum {
    ALIGN16_MASK = ~((uintptr_t)0xf)
};

static inline void *align_address(void *addr) {
    return (void *)(((uintptr_t)addr) & ALIGN16_MASK);
}

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
    struct {
        _Alignas(16) fjx_fiber_memory _;
    } m;

    void *addr = allocate_stack();
    fjx_fiber_memory *mem = align_address(((char*)addr) + STACK_SIZE - sizeof(m));

    mem->addr = addr;
    mem->size = STACK_SIZE;
    fjx_avl_node_init(&mem->link);
    mem->stack_top = mem;
    return mem;
}

void fiber_memory_destroy(fjx_fiber_memory *mem) {
    munmap(mem->addr, mem->size);
}
