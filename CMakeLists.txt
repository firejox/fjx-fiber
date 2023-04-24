cmake_minimum_required(VERSION 3.21)

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
    INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

add_subdirectory(src)

add_subdirectory(demo)
install(TARGETS fiber fiber-shared DESTINATION lib)
install(DIRECTORY include/fjx-fiber DESTINATION include
    PATTERN "internal" EXCLUDE)
install(FILES include/fjx-fiber.h DESTINATION include)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "Firejox firejox@gmail.com")
set(CPACK_GENERATOR DEB)
include(CPack)
