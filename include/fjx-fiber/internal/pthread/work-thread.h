#ifndef fjx_fiber_internal_work_thread_impl_h
#define fjx_fiber_internal_work_thread_impl_h

#include <pthread.h>
#include "./thread-semaphore.h"
#include "../fiber.h"

struct __fjx_work_thread {
    pthread_t               id;
    fjx_fiber               thread_fiber;
    fjx_list                idle_link;
    fjx_list                link;
    fjx_thread_semaphore    s;
};
typedef struct __fjx_work_thread fjx_work_thread;

DLL_LOCAL void work_thread_spawn(fjx_fiber_scheduler *);

DLL_LOCAL fjx_work_thread *current_work_thread(fjx_fiber_scheduler *);

DLL_LOCAL void main_work_thread_init(fjx_fiber_scheduler *);

#endif
