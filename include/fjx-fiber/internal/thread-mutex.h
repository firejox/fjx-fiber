#ifndef fjx_fiber_thread_mutex_h
#define fjx_fiber_thread_mutex_h

#ifndef USE_SYSTEM_THREAD
#   include "./std-thread/mutex.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#   include "./pthread/mutex.h"
#else
#   error "No thread library support"
#endif

#endif
