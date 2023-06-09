#ifndef fjx_fiber_thread_cond_h
#define fjx_fiber_thread_cond_h

#ifndef USE_SYSTEM_THREAD
#   include "./std-thread/cond.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#   include "./pthread/cond.h"
#else
#   error "No thread library support"
#endif

#endif
