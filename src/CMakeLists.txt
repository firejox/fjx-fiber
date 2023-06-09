
set(LIB_SOURCE
        fiber-memory.c
        fiber.c
        scheduler.c
        semaphore.c
        mutex.c
        channel.c
        work-thread.c
        timer.c)

add_library(fiber-objects OBJECT)
set_property(TARGET fiber-objects PROPERTY POSITION_INDEPENDENT_CODE ON)

if (USE_SYSTEM_THREAD)
    target_compile_definitions(fiber-objects PRIVATE USE_SYSTEM_THREAD)

    find_package(Threads REQUIRED)
    if (CMAKE_USE_PTHREADS_INIT)
        target_sources(fiber-objects PRIVATE pthread.c)
    endif()

    target_link_libraries(fiber-objects PRIVATE Threads::Threads)
else()
    target_sources(fiber-objects PRIVATE std-thread.c)
endif()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
    target_sources(fiber-objects PRIVATE
        $<$<PLATFORM_ID:Linux,Darwin>:gas-amd64-sysv.s>
        $<$<PLATFORM_ID:Windows>:gas-ms-x64.s>)
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86")
    target_sources(fiber-objects PRIVATE gas-i386-sysv.s)
endif()

target_sources(fiber-objects PRIVATE ${LIB_SOURCE})

include(FetchContent)
FetchContent_Declare(
    fjx-utils
    GIT_REPOSITORY https://github.com/firejox/fjx-utils.git
    GIT_TAG main
)
FetchContent_MakeAvailable(fjx-utils)
set_property(DIRECTORY ${fjx-utils_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)

set_property(TARGET fjx-utils.utils-objects PROPERTY C_VISIBILITY_PRESET hidden)

add_library(fiber-utils INTERFACE)
target_sources(fiber-utils
    INTERFACE $<TARGET_OBJECTS:fjx-libs::utils-objects>)

target_link_libraries(fiber-utils
    INTERFACE fjx-utils.utils-headers)

target_link_libraries(fiber-objects
    PRIVATE     fjx-utils.utils-headers
    PRIVATE     fiber-compl
    PRIVATE     fiber-headers)

add_library(fiber STATIC)
target_link_libraries(fiber
    PRIVATE     fiber-objects
    PRIVATE     fiber-utils
    PRIVATE     fiber-compl
    INTERFACE   fiber-headers)
set_target_properties(fiber
    PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME})

add_library(fiber-shared SHARED)
target_link_libraries(fiber-shared
    PRIVATE     fiber-objects
    PRIVATE     fiber-utils
    PRIVATE     fiber-compl
    INTERFACE   fiber-headers)
set_target_properties(fiber-shared
    PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME})

add_custom_command(
    TARGET fiber POST_BUILD
    DEPENDS fiber
    COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}>
    ARGS --strip-debug --strip-unneeded $<TARGET_FILE:fiber>)

add_custom_command(
    TARGET fiber-shared POST_BUILD
    DEPENDS fiber-shared
    COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}>
    ARGS --strip-all $<TARGET_FILE:fiber-shared>)
