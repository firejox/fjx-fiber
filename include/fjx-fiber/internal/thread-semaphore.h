#ifndef fjx_fiber_internal_thread_semaphore_h
#define fjx_fiber_internal_thread_semaphore_h

#ifndef USE_SYSTEM_THREAD
#   include "./std-thread/thread-semaphore.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#   include "./pthread/thread-semaphore.h"
#else
#   error "No thread library support"
#endif

#endif
