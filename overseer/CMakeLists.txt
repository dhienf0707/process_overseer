# add executable target
add_executable(overseer overseer.c)
target_link_libraries(overseer helpers Threads::Threads)

# add exec target
add_executable(exec exec_cmd.c)
target_link_libraries(exec helpers -lutil Threads::Threads)


# add simple target
add_executable(simple simple.c)

# include directory
target_include_directories(overseer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
