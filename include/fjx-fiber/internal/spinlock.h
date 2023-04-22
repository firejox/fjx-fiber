#ifndef fjx_spinlock_h
#define fjx_spinlock_h

#ifndef USE_SYSTEM_THREAD
#   include "./std-thread/spinlock.h"
#else
#   include "./pthread/spinlock.h"
#endif

#endif
