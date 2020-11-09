add_executable(controller controller.c)
target_link_libraries(controller helpers)
target_include_directories(controller PUBLIC "{CMAKE_CURRENT_SOURCE_DIR}")