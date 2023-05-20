
macro(add_demo exec_name)
    add_executable("demo.${exec_name}" ${ARGN})
    set_target_properties("demo.${exec_name}"
        PROPERTIES
            OUTPUT_NAME ${exec_name}
        )
    target_link_libraries("demo.${exec_name}" fiber-shared)
endmacro()

add_demo(hello-world hello-world.c)
add_demo(hello2 hello2.c)
add_demo(dining-philosopher dining-philosopher.c)
add_demo(channel-test channel-test.c)
add_demo(prime-seive prime-seive.c)
