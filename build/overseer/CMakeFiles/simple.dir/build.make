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
CMAKE_SOURCE_DIR = /home/hoangqwe159/process_overseer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hoangqwe159/process_overseer/build

# Include any dependencies generated for this target.
include overseer/CMakeFiles/simple.dir/depend.make

# Include the progress variables for this target.
include overseer/CMakeFiles/simple.dir/progress.make

# Include the compile flags for this target's objects.
include overseer/CMakeFiles/simple.dir/flags.make

overseer/CMakeFiles/simple.dir/simple.c.o: overseer/CMakeFiles/simple.dir/flags.make
overseer/CMakeFiles/simple.dir/simple.c.o: ../overseer/simple.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hoangqwe159/process_overseer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object overseer/CMakeFiles/simple.dir/simple.c.o"
	cd /home/hoangqwe159/process_overseer/build/overseer && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/simple.dir/simple.c.o   -c /home/hoangqwe159/process_overseer/overseer/simple.c

overseer/CMakeFiles/simple.dir/simple.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/simple.dir/simple.c.i"
	cd /home/hoangqwe159/process_overseer/build/overseer && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/hoangqwe159/process_overseer/overseer/simple.c > CMakeFiles/simple.dir/simple.c.i

overseer/CMakeFiles/simple.dir/simple.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/simple.dir/simple.c.s"
	cd /home/hoangqwe159/process_overseer/build/overseer && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/hoangqwe159/process_overseer/overseer/simple.c -o CMakeFiles/simple.dir/simple.c.s

# Object files for target simple
simple_OBJECTS = \
"CMakeFiles/simple.dir/simple.c.o"

# External object files for target simple
simple_EXTERNAL_OBJECTS =

overseer/simple: overseer/CMakeFiles/simple.dir/simple.c.o
overseer/simple: overseer/CMakeFiles/simple.dir/build.make
overseer/simple: overseer/CMakeFiles/simple.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hoangqwe159/process_overseer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable simple"
	cd /home/hoangqwe159/process_overseer/build/overseer && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simple.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
overseer/CMakeFiles/simple.dir/build: overseer/simple

.PHONY : overseer/CMakeFiles/simple.dir/build

overseer/CMakeFiles/simple.dir/clean:
	cd /home/hoangqwe159/process_overseer/build/overseer && $(CMAKE_COMMAND) -P CMakeFiles/simple.dir/cmake_clean.cmake
.PHONY : overseer/CMakeFiles/simple.dir/clean

overseer/CMakeFiles/simple.dir/depend:
	cd /home/hoangqwe159/process_overseer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hoangqwe159/process_overseer /home/hoangqwe159/process_overseer/overseer /home/hoangqwe159/process_overseer/build /home/hoangqwe159/process_overseer/build/overseer /home/hoangqwe159/process_overseer/build/overseer/CMakeFiles/simple.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : overseer/CMakeFiles/simple.dir/depend

