# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/teunvbrakel/assignments/OperatingSystemConcepts/assignment1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/teunvbrakel/assignments/OperatingSystemConcepts/build

# Include any dependencies generated for this target.
include CMakeFiles/shelllib.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/shelllib.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/shelllib.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/shelllib.dir/flags.make

CMakeFiles/shelllib.dir/shell.cpp.o: CMakeFiles/shelllib.dir/flags.make
CMakeFiles/shelllib.dir/shell.cpp.o: /home/teunvbrakel/assignments/OperatingSystemConcepts/assignment1/shell.cpp
CMakeFiles/shelllib.dir/shell.cpp.o: CMakeFiles/shelllib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/teunvbrakel/assignments/OperatingSystemConcepts/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/shelllib.dir/shell.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/shelllib.dir/shell.cpp.o -MF CMakeFiles/shelllib.dir/shell.cpp.o.d -o CMakeFiles/shelllib.dir/shell.cpp.o -c /home/teunvbrakel/assignments/OperatingSystemConcepts/assignment1/shell.cpp

CMakeFiles/shelllib.dir/shell.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/shelllib.dir/shell.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/teunvbrakel/assignments/OperatingSystemConcepts/assignment1/shell.cpp > CMakeFiles/shelllib.dir/shell.cpp.i

CMakeFiles/shelllib.dir/shell.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/shelllib.dir/shell.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/teunvbrakel/assignments/OperatingSystemConcepts/assignment1/shell.cpp -o CMakeFiles/shelllib.dir/shell.cpp.s

# Object files for target shelllib
shelllib_OBJECTS = \
"CMakeFiles/shelllib.dir/shell.cpp.o"

# External object files for target shelllib
shelllib_EXTERNAL_OBJECTS =

libshelllib.a: CMakeFiles/shelllib.dir/shell.cpp.o
libshelllib.a: CMakeFiles/shelllib.dir/build.make
libshelllib.a: CMakeFiles/shelllib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/teunvbrakel/assignments/OperatingSystemConcepts/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libshelllib.a"
	$(CMAKE_COMMAND) -P CMakeFiles/shelllib.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/shelllib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/shelllib.dir/build: libshelllib.a
.PHONY : CMakeFiles/shelllib.dir/build

CMakeFiles/shelllib.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/shelllib.dir/cmake_clean.cmake
.PHONY : CMakeFiles/shelllib.dir/clean

CMakeFiles/shelllib.dir/depend:
	cd /home/teunvbrakel/assignments/OperatingSystemConcepts/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/teunvbrakel/assignments/OperatingSystemConcepts/assignment1 /home/teunvbrakel/assignments/OperatingSystemConcepts/assignment1 /home/teunvbrakel/assignments/OperatingSystemConcepts/build /home/teunvbrakel/assignments/OperatingSystemConcepts/build /home/teunvbrakel/assignments/OperatingSystemConcepts/build/CMakeFiles/shelllib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/shelllib.dir/depend

