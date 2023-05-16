#ifndef fjx_fiber_thread_mutex_h
#define fjx_fiber_thread_mutex_h

#ifndef USE_SYSTEM_THREAD
#include "./std-thread/mutex.h"
#else
#include "./pthread/mutex.h"
#endif

#endif
