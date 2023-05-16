#ifndef fjx_fiber_thread_cond_h
#define fjx_fiber_thread_cond_h

#ifndef USE_SYSTEM_THREAD
#include "./std-thread/cond.h"
#else
#include "./pthread/cond.h"
#endif

#endif
