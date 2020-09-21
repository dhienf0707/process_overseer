cmake_minimum_required(VERSION 3.16)
project(process_overseer C)

set(CMAKE_C_STANDARD 99)

# add subdirectories of targets (note: if libraries is used, subdirectories of libraries need to be added first)
add_subdirectory(helpers)
add_subdirectory(overseer)
add_subdirectory(controller)
add_executable(test test.c)