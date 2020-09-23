# Find pthread library
find_package(Threads)

# add executable target
add_executable(overseer overseer.c)

# link libraries
target_link_libraries(overseer helpers Threads::Threads)

# include directory
target_include_directories(overseer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")