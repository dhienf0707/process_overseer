add_executable(overseer overseer.c)
target_link_libraries(overseer helpers)
target_include_directories(overseer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")