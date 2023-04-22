#ifndef fjx_fiber_rwlock_h
#define fjx_fiber_rwlock_h

#ifndef USE_SYSTEM_THREAD
#   include "./std-thread/rwlock.h"
#else
#   include "./pthread/rwlock.h"
#endif

#endif
