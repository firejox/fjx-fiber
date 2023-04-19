#ifndef fjx_fiber_internal_thread_semaphore_h
#define fjx_fiber_internal_thread_semaphore_h

#include "fjx-fiber/thread-config.h"

#ifdef USE_PTHREAD
#include "./pthread/thread-semaphore.h"
#else
#include "./std-thread/thread-semaphore.h"
#endif

#endif
