#ifndef fjx_fiber_internal_work_thread_h
#define fjx_fiber_internal_work_thread_h

#ifndef USE_SYSTEM_THREAD
#   include "std-thread/work-thread.h"
#else
#   include "pthread/work-thread.h"
#endif

#endif
