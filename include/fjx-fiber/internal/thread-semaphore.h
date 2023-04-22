#ifndef fjx_fiber_internal_thread_semaphore_h
#define fjx_fiber_internal_thread_semaphore_h

#ifndef USE_SYSTEM_THREAD
#   include "./std-thread/thread-semaphore.h"
#else
#   include "./pthread/thread-semaphore.h"
#endif

#endif
