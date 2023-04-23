#ifndef fjx_fiber_internal_cpu_builtin_h
#define fjx_fiber_internal_cpu_builtin_h

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#   if defined(__GNUC__)
#       define fjx_cpu_pause() __builtin_ia32_pause()
#   elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#       define fjx_cpu_pause() _mm_pause()
#   else
#       define fjx_cpu_pause() ((void)(0))
#   endif
#elif defined(__arm__) || defined(__aarch64__)
#   define fjx_cpu_pause() __yield()
#else
#   define fjx_cpu_pause() ((void)0)
#endif

#endif
