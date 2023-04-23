cmake_minimum_required(VERSION 3.21)

project(fjx-fiber C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

option(USE_SYSTEM_THREAD "Use system thread as backend" OFF)

add_subdirectory(src)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(demo)
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY include/fjx-fiber DESTINATION include
    PATTERN "internal" EXCLUDE)
install(FILES include/fjx-fiber.h DESTINATION include)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "Firejox firejox@gmail.com")
set(CPACK_GENERATOR DEB)
include(CPack)
