cmake_minimum_required(VERSION 3.11)

project(fjx-fiber C ASM)

set(CMAKE_C_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

option(USE_PTHREAD "Use pthread as backend" OFF)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/fjx-fiber/internal/thread-config.h.in"
    "${PROJECT_BINARY_DIR}/include/fjx-fiber/thread-config.h")

add_subdirectory(src)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/include/fjx-fiber/internal
    PRIVATE ${PROJECT_BINARY_DIR}/include)

add_subdirectory(demo)
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY include/fjx-fiber DESTINATION include
    PATTERN "internal" EXCLUDE)
install(FILES include/fjx-fiber.h DESTINATION include)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "Firejox firejox@gmail.com")
set(CPACK_GENERATOR DEB)
include(CPack)
