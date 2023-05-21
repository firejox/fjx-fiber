#ifndef fjx_fiber_rwlock_h
#define fjx_fiber_rwlock_h

#ifndef USE_SYSTEM_THREAD
#   include "./std-thread/rwlock.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#   include "./pthread/rwlock.h"
#else
#   error "No thread library support"
#endif

#endif
