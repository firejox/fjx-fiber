#ifndef fjx_fiber_internal_thread_h
#define fjx_fiber_internal_thread_h

#ifndef USE_SYSTEM_THREAD
#include "./std-thread/thread.h"
#else
#include "./pthread/thread.h"
#endif

#endif
