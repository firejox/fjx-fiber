#include "fjx-fiber/internal/fiber.h"
#include "fjx-fiber/internal/work-thread.h"
#include "fjx-fiber/internal/scheduler.h"
#include "fjx-fiber/internal/fiber-memory.h"

extern DLL_LOCAL void fiber_switch_impl(void **);
extern DLL_LOCAL void fiber_load_unaryop(void);
extern DLL_LOCAL void fiber_load_entrance(void);

void fiber_switch(fjx_fiber *f) {
    fiber_switch_impl(&f->stack_top);
}

void fiber_add_deferred(
        fjx_fiber *f,
        deferred_func_t cleanup,
        void *data) {
    void** stack_ptr = (void **)f->stack_top;
    *(stack_ptr - 1) = NULL;
    *(stack_ptr - 2) = data;
    *(stack_ptr - 3) = (void *)cleanup;
    *(stack_ptr - 4) = (void *)fiber_load_unaryop;
    f->stack_top = (void *)(stack_ptr - 4);
}

static void fiber_yield_impl(fjx_fiber_scheduler *sched) {
    fjx_fiber f;

    if (get_available_fiber(sched, &f)) {
        fjx_thread_yield();
    } else {
        fjx_fiber_pair p = { .sched = sched, .f = &f };
        fiber_add_deferred(&f,
                (deferred_func_t)enqueue_fiber_pair,
                &p);
        fiber_switch(&f);
    }
}

void fiber_yield(void) {
    fiber_yield_impl(current_fiber_scheduler());
}

static void fiber_exit(fjx_fiber_scheduler *sched) {
    fjx_fiber f;
    fjx_fiber_pair p = {.sched = sched, .f = &f};

    get_available_fiber(sched, &f);

    fiber_add_deferred(&f, (deferred_func_t)recycle_fiber_pair, &p);
    fiber_switch(&f);
}

enum {
    ALIGN16_MASK = ~((uintptr_t)0xf)
};

static inline void *align_address(void *addr) {
    return (void *)(((uintptr_t)addr) & ALIGN16_MASK);
}

void *fiber_spawn_stack_top(
        fjx_fiber_scheduler *sched,
        entrance_func_t entrance,
        void *data) {
    fjx_fiber_memory *mem = get_available_memory(sched);
    void **stack_ptr = (void **)mem->stack_top;
    *(stack_ptr - 1) = sched;
    *(stack_ptr - 2) = (void *)fiber_exit;
    *(stack_ptr - 3) = data;
    *(stack_ptr - 4) = (void *)entrance;
    *(stack_ptr - 5) = (void *)fiber_load_entrance;

    return (void *)(stack_ptr - 5);
}

static void fiber_spawn_impl(
        fjx_fiber_scheduler *sched,
        entrance_func_t entrance,
        void *data) {
    struct {
        _Alignas(16) fjx_fiber f;
    } m;

    void *stack_top = fiber_spawn_stack_top(sched, entrance, data);
    fjx_fiber *f = (fjx_fiber *)(void *)((char *)align_address(stack_top) - sizeof(m));
    f->stack_top = stack_top;
    enqueue_fiber(sched, f);
}

void fiber_spawn(
        entrance_func_t entrance,
        void *data) {
    fiber_spawn_impl(current_fiber_scheduler(),
            entrance,
            data);
}
