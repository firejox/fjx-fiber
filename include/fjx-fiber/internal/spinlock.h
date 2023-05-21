#ifndef fjx_spinlock_h
#define fjx_spinlock_h

#ifndef USE_SYSTEM_THREAD
#   include "./std-thread/spinlock.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#   include "./pthread/spinlock.h"
#else
#   error "No thread library support"
#endif

#endif
