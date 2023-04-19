#ifndef fjx_spinlock_h
#define fjx_spinlock_h

#include "fjx-fiber/thread-config.h"

#ifdef USE_PTHREAD
#include "./pthread/spinlock.h"
#else
#include "./std-thread/spinlock.h"
#endif

#endif
