cmake_minimum_required(VERSION 3.23)

project(fjx-fiber C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)

option(USE_SYSTEM_THREAD "Use system thread as backend" OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(fiber-compl-debug INTERFACE)
add_library(fjx-libs::fiber-compl-debug ALIAS fiber-compl-debug)
target_compile_options(fiber-compl-debug
    INTERFACE "$<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>>:-O0;-ggdb>")

add_library(fiber-compl-release INTERFACE)
add_library(fjx-libs::fiber-compl-release ALIAS fiber-compl-release)
target_compile_options(fiber-compl-release
    INTERFACE "$<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>>:-O2>")

add_library(fiber-compl INTERFACE)
add_library(fjx-libs::fiber-compl ALIAS fiber-compl)
target_compile_options(fiber-compl
    INTERFACE "$<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>>:-Wall;-Wextra>")
target_link_libraries(fiber-compl
    INTERFACE debug     fjx-libs::fiber-compl-debug
    INTERFACE optimized fjx-libs::fiber-compl-release)

add_library(fiber-headers INTERFACE)
target_include_directories(fiber-headers
    INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    INTERFACE $<INSTALL_INTERFACE:include>)

add_library(fiber-headers-export INTERFACE)
target_sources(fiber-headers-export
    INTERFACE
    FILE_SET pub0 TYPE HEADERS BASE_DIRS include FILES
    include/fjx-fiber.h
    FILE_SET pub1 TYPE HEADERS BASE_DIRS include/fjx-fiber FILES
    include/fjx-fiber/fiber.h
    include/fjx-fiber/timer.h
    include/fjx-fiber/channel.h
    include/fjx-fiber/mutex.h
    include/fjx-fiber/scheduler.h
    include/fjx-fiber/semaphore.h)

add_subdirectory(src)

add_subdirectory(demo EXCLUDE_FROM_ALL)

add_subdirectory(tests EXCLUDE_FROM_ALL)
get_directory_property(TEST_EXEC_LIST DIRECTORY tests DEFINITION TEST_EXEC_LIST)

set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")
enable_testing()

foreach (test_name IN LISTS TEST_EXEC_LIST)
	add_test(NAME "${test_name}" COMMAND ${CMAKE_COMMAND}
		--build ${CMAKE_BINARY_DIR}
		--target ${test_name})
	add_custom_target("${test_name}" $<TARGET_FILE:build.${test_name}>)
	add_dependencies(${test_name} "build.${test_name}")
endforeach()

install(TARGETS
    fiber
    fiber-shared
    fiber-objects
    fiber-compl
    fiber-compl-debug
    fiber-compl-release
    fiber-headers
    fiber-headers-export
    fiber-utils
    fjx-utils.utils-headers
    EXPORT fiber-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    FILE_SET pub0 DESTINATION include
    FILE_SET pub1 DESTINATION include/fjx-fiber)

install(EXPORT fiber-targets
    NAMESPACE "fjx-libs::"
    FILE fiber.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fjx-libs)

export(EXPORT fiber-targets
    NAMESPACE "fjx-libs::"
    FILE fiber.cmake)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "Firejox firejox@gmail.com")
set(CPACK_GENERATOR DEB)
include(CPack)
