cmake_minimum_required(VERSION 3.16)
project(process_overseer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_MAKE_PROGRAM ninja)
set(CMAKE_C_COMPILER clang)

# Find pthread library
find_package(Threads)

# add subdirectories of targets (note: if libraries is used, subdirectories of libraries need to be added first)
add_subdirectory(helpers)
add_subdirectory(overseer)
add_subdirectory(controller)