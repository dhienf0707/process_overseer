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
include overseer/CMakeFiles/exec.dir/depend.make

# Include the progress variables for this target.
include overseer/CMakeFiles/exec.dir/progress.make

# Include the compile flags for this target's objects.
include overseer/CMakeFiles/exec.dir/flags.make

overseer/CMakeFiles/exec.dir/exec_cmd.c.o: overseer/CMakeFiles/exec.dir/flags.make
overseer/CMakeFiles/exec.dir/exec_cmd.c.o: ../overseer/exec_cmd.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hoangqwe159/process_overseer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object overseer/CMakeFiles/exec.dir/exec_cmd.c.o"
	cd /home/hoangqwe159/process_overseer/build/overseer && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/exec.dir/exec_cmd.c.o   -c /home/hoangqwe159/process_overseer/overseer/exec_cmd.c

overseer/CMakeFiles/exec.dir/exec_cmd.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/exec.dir/exec_cmd.c.i"
	cd /home/hoangqwe159/process_overseer/build/overseer && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/hoangqwe159/process_overseer/overseer/exec_cmd.c > CMakeFiles/exec.dir/exec_cmd.c.i

overseer/CMakeFiles/exec.dir/exec_cmd.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/exec.dir/exec_cmd.c.s"
	cd /home/hoangqwe159/process_overseer/build/overseer && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/hoangqwe159/process_overseer/overseer/exec_cmd.c -o CMakeFiles/exec.dir/exec_cmd.c.s

# Object files for target exec
exec_OBJECTS = \
"CMakeFiles/exec.dir/exec_cmd.c.o"

# External object files for target exec
exec_EXTERNAL_OBJECTS =

overseer/exec: overseer/CMakeFiles/exec.dir/exec_cmd.c.o
overseer/exec: overseer/CMakeFiles/exec.dir/build.make
overseer/exec: helpers/libhelpers.a
overseer/exec: overseer/CMakeFiles/exec.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hoangqwe159/process_overseer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable exec"
	cd /home/hoangqwe159/process_overseer/build/overseer && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/exec.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
overseer/CMakeFiles/exec.dir/build: overseer/exec

.PHONY : overseer/CMakeFiles/exec.dir/build

overseer/CMakeFiles/exec.dir/clean:
	cd /home/hoangqwe159/process_overseer/build/overseer && $(CMAKE_COMMAND) -P CMakeFiles/exec.dir/cmake_clean.cmake
.PHONY : overseer/CMakeFiles/exec.dir/clean

overseer/CMakeFiles/exec.dir/depend:
	cd /home/hoangqwe159/process_overseer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hoangqwe159/process_overseer /home/hoangqwe159/process_overseer/overseer /home/hoangqwe159/process_overseer/build /home/hoangqwe159/process_overseer/build/overseer /home/hoangqwe159/process_overseer/build/overseer/CMakeFiles/exec.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : overseer/CMakeFiles/exec.dir/depend

