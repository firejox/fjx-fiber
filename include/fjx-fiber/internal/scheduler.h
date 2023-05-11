#ifndef fjx_fiber_internal_scheduler_h
#define fjx_fiber_internal_scheduler_h

#include "./utils.h"
#include "./work-thread.h"
#include "./spinlock.h"
#include "./rwlock.h"
#include "../scheduler.h"

struct fjx_fiber_scheduler__ {
    fjx_spinlock queue_lock;
    fjx_list fiber_list;
    fjx_list idle_thread_list;

    fjx_spinlock tlist_init_lock;
    fjx_list thread_list;

    fjx_spinlock disposed_fiber_lock;
    fjx_list disposed_fiber_list;

    fjx_rwlock pool_lock;
    fjx_avl fiber_pool;

    fjx_work_thread main_th;
};

DLL_LOCAL bool get_available_fiber(fjx_fiber_scheduler *, fjx_fiber *);

DLL_LOCAL fjx_fiber *get_available_fiber_unsafe(fjx_fiber_scheduler *);
DLL_LOCAL fjx_fiber_memory *get_available_memory(fjx_fiber_scheduler *);
DLL_LOCAL fjx_work_thread *try_get_idle_thread_unsafe(fjx_fiber_scheduler *);
DLL_LOCAL void enqueue_fiber(fjx_fiber_scheduler *, fjx_fiber *);
DLL_LOCAL void enqueue_fiber_list(fjx_fiber_scheduler *, fjx_list *);

#endif
