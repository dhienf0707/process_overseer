# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/d/CLionProjects/process_overseer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl

# Include any dependencies generated for this target.
include controller/CMakeFiles/controller.dir/depend.make

# Include the progress variables for this target.
include controller/CMakeFiles/controller.dir/progress.make

# Include the compile flags for this target's objects.
include controller/CMakeFiles/controller.dir/flags.make

controller/CMakeFiles/controller.dir/controller.c.o: controller/CMakeFiles/controller.dir/flags.make
controller/CMakeFiles/controller.dir/controller.c.o: ../controller/controller.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object controller/CMakeFiles/controller.dir/controller.c.o"
	cd /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl/controller && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/controller.dir/controller.c.o   -c /mnt/d/CLionProjects/process_overseer/controller/controller.c

controller/CMakeFiles/controller.dir/controller.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/controller.dir/controller.c.i"
	cd /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl/controller && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/d/CLionProjects/process_overseer/controller/controller.c > CMakeFiles/controller.dir/controller.c.i

controller/CMakeFiles/controller.dir/controller.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/controller.dir/controller.c.s"
	cd /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl/controller && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/d/CLionProjects/process_overseer/controller/controller.c -o CMakeFiles/controller.dir/controller.c.s

# Object files for target controller
controller_OBJECTS = \
"CMakeFiles/controller.dir/controller.c.o"

# External object files for target controller
controller_EXTERNAL_OBJECTS =

controller/controller: controller/CMakeFiles/controller.dir/controller.c.o
controller/controller: controller/CMakeFiles/controller.dir/build.make
controller/controller: helpers/libhelpers.a
controller/controller: controller/CMakeFiles/controller.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable controller"
	cd /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl/controller && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/controller.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
controller/CMakeFiles/controller.dir/build: controller/controller

.PHONY : controller/CMakeFiles/controller.dir/build

controller/CMakeFiles/controller.dir/clean:
	cd /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl/controller && $(CMAKE_COMMAND) -P CMakeFiles/controller.dir/cmake_clean.cmake
.PHONY : controller/CMakeFiles/controller.dir/clean

controller/CMakeFiles/controller.dir/depend:
	cd /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/d/CLionProjects/process_overseer /mnt/d/CLionProjects/process_overseer/controller /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl/controller /mnt/d/CLionProjects/process_overseer/cmake-build-release-wsl/controller/CMakeFiles/controller.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : controller/CMakeFiles/controller.dir/depend

