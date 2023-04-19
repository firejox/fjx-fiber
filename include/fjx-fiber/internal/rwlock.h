#ifndef fjx_fiber_rwlock_h
#define fjx_fiber_rwlock_h

#include "fjx-fiber/thread-config.h"

#ifdef USE_PTHREAD
#include "./pthread/rwlock.h"
#else
#include "./std-thread/rwlock.h"
#endif

#endif
