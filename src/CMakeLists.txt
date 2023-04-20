
set(LIB_SOURCE fiber-memory.c fiber.c scheduler.c semaphore.c mutex.c channel.c)
set(THREAD_LIBS)

find_package(Threads REQUIRED)
if (USE_PTHREAD)
    if (CMAKE_USE_PTHREADS_INIT)
        set(THREAD_LIBS ${CMAKE_THREAD_LIBS_INIT})
        set(LIB_SOURCE ${LIB_SOURCE} pthread.c)
    endif()
else()
    set(THREAD_LIBS ${CMAKE_THREAD_LIBS_INIT})
    set(LIB_SOURCE ${LIB_SOURCE} std-thread.c)
endif()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
    set(LIB_SOURCE ${LIB_SOURCE} gas-amd64-sysv.s)
endif()

add_library(${PROJECT_NAME} SHARED ${LIB_SOURCE})

if (THREAD_LIBS)
    target_link_libraries(${PROJECT_NAME} ${THREAD_LIBS} "-lfjx-utils")
else()
    target_link_libraries(${PROJECT_NAME} "-lfjx-utils")
endif()
