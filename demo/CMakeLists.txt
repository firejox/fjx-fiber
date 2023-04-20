
macro(add_demo exec_name)
    add_executable("demo.${exec_name}" EXCLUDE_FROM_ALL ${ARGN})
    set_target_properties("demo.${exec_name}"
        PROPERTIES
            OUTPUT_NAME ${exec_name}
        )
    target_include_directories("demo.${exec_name}"
        PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries("demo.${exec_name}" ${PROJECT_NAME})
endmacro()

add_demo(hello-world hello-world.c)
add_demo(hello2 hello2.c)
add_demo(dining-philosopher dining-philosopher.c)
add_demo(channel-test channel-test.c)
add_demo(prime-seive prime-seive.c)
