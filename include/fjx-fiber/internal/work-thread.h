#ifndef fjx_fiber_internal_work_thread_h
#define fjx_fiber_internal_work_thread_h

#include "./fiber.h"
#include "./thread.h"
#include "./thread-semaphore.h"

struct fjx_work_thread__ {
    fjx_fiber               thread_fiber;
    fjx_list                idle_link;
    fjx_list                link;
    fjx_thread_semaphore    s;
};

DLL_LOCAL void work_thread_spawn(fjx_fiber_scheduler *);

DLL_LOCAL fjx_work_thread *current_work_thread(fjx_fiber_scheduler *);

#define current_work_thread_fiber(sched) (&current_work_thread(sched)->thread_fiber)

DLL_LOCAL void main_work_thread_init(fjx_fiber_scheduler *);

#endif
