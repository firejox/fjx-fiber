#ifndef fjx_fiber_internal_work_thread_h
#define fjx_fiber_internal_work_thread_h

#include "fjx-fiber/thread-config.h"

#ifdef USE_PTHREAD
#include "pthread/work-thread.h"
#else
#include "std-thread/work-thread.h"
#endif

#endif
