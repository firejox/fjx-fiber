
set(LIB_SOURCE fiber-memory.c fiber.c scheduler.c semaphore.c mutex.c channel.c)

add_library(${PROJECT_NAME} SHARED)

if (USE_SYSTEM_THREAD)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_SYSTEM_THREAD)

    find_package(Threads REQUIRED)
    if (CMAKE_USE_PTHREADS_INIT)
        target_sources(${PROJECT_NAME}
            PRIVATE pthread.c)
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
else()
    target_sources(${PROJECT_NAME}
        PRIVATE std-thread.c)
endif()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
    target_sources(${PROJECT_NAME}
        PRIVATE gas-amd64-sysv.s)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${LIB_SOURCE})

include(FetchContent)
FetchContent_Declare(
    fjx-utils
    GIT_REPOSITORY https://github.com/firejox/fjx-utils.git
    GIT_TAG main
)
FetchContent_MakeAvailable(fjx-utils)

set_property(TARGET fjx-utils.utils-objects PROPERTY C_VISIBILITY_PRESET hidden)
target_link_libraries(${PROJECT_NAME} PRIVATE fjx-utils.utils-objects)
